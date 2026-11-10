#include "VtkFileSourceProvider.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Framework/Results/ResultFileIdentity.hpp"
#include <vtkXMLTableReader.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLPartitionedDataSetCollectionReader.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkCompositeDataSet.h>
#include <vtkInformation.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkTable.h>
#include <vtkStringArray.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkSmartPointer.h>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <exception>
#include <limits>
#include <algorithm>
#include <cmath>
namespace
{
  template<class Reader> std::shared_ptr<vtkDataObject> read(const std::string& path, std::string& error)
  {
    auto reader = vtkSmartPointer<Reader>::New();
    bool failed = false;
    auto observer = vtkSmartPointer<vtkCallbackCommand>::New();
    observer->SetClientData(&failed);
    observer->SetCallback([](vtkObject*,unsigned long,void* client,void*)
    {
      *static_cast<bool*>(client) = true;
    });
    reader->AddObserver(vtkCommand::ErrorEvent,observer);
    reader->SetFileName(path.c_str());
    reader->Update();
    auto* output = reader->GetOutputDataObject(0);
    if (failed || reader->GetErrorCode() != 0 || !output)
    {
      error = "VTK could not read the result file.";
      return {};
    }
    output->Register(nullptr);
    return std::shared_ptr<vtkDataObject>(output,[](vtkDataObject* value)
    {
      value->Delete();
    });
  }
  std::string metadata(vtkFieldData* data, const char* name)
  {
    auto* array = data ? vtkStringArray::SafeDownCast(data->GetAbstractArray(name)) : nullptr;
    return array && array->GetNumberOfValues() ? std::string(array->GetValue(0)) : std::string{};
  }
}
std::string Postprocessor::VtkFileSourceProvider::command_prefix() const
{
  return "vtk";
}
bool Postprocessor::VtkFileSourceProvider::refresh(const SimResults::ResultSourceReference& request,
  SimResults::PublishedResultSource& output, std::string& error)
{
  return open(request,output,error);
}
bool Postprocessor::VtkFileSourceProvider::open(const SimResults::ResultSourceReference& request,
  SimResults::PublishedResultSource& output, std::string& error)
{
  error.clear();
  const auto* files = std::get_if<SimResults::FileSourceReference>(&request);
  if (!files || files->provider_id != "vtk" || files->paths.size() != 1)
  {
    error = "A VTK source requires exactly one VTU, VTPC or VTT file.";
    return false;
  }
  try
  {
    const QFileInfo info(QString::fromUtf8(files->paths.front().c_str()));
    const auto extension = info.suffix().toLower();
    if (!info.isFile() || !info.isReadable() || (extension != "vtu" && extension != "vtpc" && extension != "vtt"))
    {
      error = "The VTK file is missing, unreadable or has an unsupported extension.";
      return false;
    }
    const auto path = info.canonicalFilePath().toUtf8().toStdString();
    std::string identity;
    if (!SimResults::result_file_identity({path},identity,error))
    {
      return false;
    }
    std::shared_ptr<vtkDataObject> object;
    if (extension == "vtt")
    {
      object = read<vtkXMLTableReader>(path,error);
    }
    else if (extension == "vtu")
    {
      object = read<vtkXMLUnstructuredGridReader>(path,error);
    }
    else
    {
      object = read<vtkXMLPartitionedDataSetCollectionReader>(path,error);
    }
    if (!object)
    {
      return false;
    }
    auto data = std::make_shared<SimResults::VtkResultsData>();
    data->run_identity = identity;
    data->result_identity = identity;
    if (auto* table = vtkTable::SafeDownCast(object.get()))
    {
      if (!table->GetFieldData()->HasArray("TableName"))
      {
        auto name = vtkSmartPointer<vtkStringArray>::New();
        name->SetName("TableName");
        name->InsertNextValue(info.completeBaseName().toUtf8().toStdString());
        table->GetFieldData()->AddArray(name);
      }
      data->tables.emplace_back(object,table);
    }
    else
    {
      SimResults::Increment frame;
      frame.dataset = object;
      auto* collection = vtkPartitionedDataSetCollection::SafeDownCast(object.get());
      data->layout = collection ? SimResults::DatasetLayout::Collection : SimResults::DatasetLayout::SingleGrid;
      const auto parts = Postprocessor::result_mesh_parts(object.get());
      for (const auto& part : parts)
      {
        if (data->datasets.empty())
        {
          auto* time = part.second->GetFieldData()->GetArray("TimeValue");
          if (time && time->GetNumberOfTuples() == 1 && time->GetNumberOfComponents() == 1 &&
              std::isfinite(time->GetComponent(0,0)))
          {
            frame.time = time->GetComponent(0,0);
          }
          const auto integer = [&](const char* key,int& value)
          {
            const auto text = metadata(part.second->GetFieldData(),key);
            bool valid = false;
            const int parsed = QString::fromUtf8(text.c_str()).toInt(&valid);
            if (valid)
            {
              value = parsed;
            }
          };
          integer("Step",frame.step); integer("Increment",frame.increment);
          integer("AnalysisType",frame.analysis_type); integer("Mode",frame.mode);
          integer("NodalDiameter",frame.nodal_diameter); integer("Subcase",frame.subcase);
        }
        SimResults::Dataset dataset;
        dataset.id = part.first;
        dataset.kind = metadata(part.second->GetFieldData(),"GroupKind");
        dataset.name = metadata(part.second->GetFieldData(),"GroupName");
        dataset.element_type = metadata(part.second->GetFieldData(),"ElementType");
        auto* modelId = part.second->GetFieldData()->GetArray("ModelId");
        if (modelId && modelId->GetNumberOfTuples() == 1 && modelId->GetComponent(0,0) >= 0 &&
            modelId->GetComponent(0,0) <= std::numeric_limits<int>::max() &&
            std::floor(modelId->GetComponent(0,0)) == modelId->GetComponent(0,0))
        {
          dataset.model_id = static_cast<int>(modelId->GetComponent(0,0));
        }
        if (dataset.name.empty() && collection && collection->HasMetaData(static_cast<unsigned int>(part.first)))
        {
          const auto* name = collection->GetMetaData(static_cast<unsigned int>(part.first))->Get(vtkCompositeDataSet::NAME());
          if (name)
          {
            dataset.name = name;
          }
        }
        if (dataset.name.empty())
        {
          dataset.name = collection ? "Dataset "+std::to_string(part.first) : info.completeBaseName().toUtf8().toStdString();
        }
        dataset.point_count = static_cast<std::size_t>(part.second->GetNumberOfPoints());
        dataset.cell_count = static_cast<std::size_t>(part.second->GetNumberOfCells());
        data->datasets.push_back(dataset);
        for (int association = 0; association < 2; ++association)
        {
          vtkFieldData* arrays = association == 0 ? static_cast<vtkFieldData*>(part.second->GetPointData()) :
            static_cast<vtkFieldData*>(part.second->GetCellData());
          for (int index = 0; index < arrays->GetNumberOfArrays(); ++index)
          {
            auto* array = vtkDataArray::SafeDownCast(arrays->GetAbstractArray(index));
            if (!array || !array->GetName())
            {
              continue;
            }
            SimResults::Field field;
            field.name = array->GetName(); field.array_name = field.name; field.source = "VTK";
            field.dataset_id = part.first;
            field.association = association == 0 ? SimResults::Association::Node : SimResults::Association::Element;
            if (association == 0 && dataset.kind == "DAT_IntegrationPoints")
            {
              field.association = SimResults::Association::IntegrationPoint;
            }
            for (int component = 0; component < array->GetNumberOfComponents(); ++component)
            {
              const auto* name = array->GetComponentName(component);
              field.components.push_back(name ? std::string(name) : std::to_string(component+1));
            }
            if (array->GetNumberOfComponents() == 3)
            {
              field.vector_components = std::array<std::size_t,3>{0,1,2};
            }
            const auto expected = association == 0 ? part.second->GetNumberOfPoints() : part.second->GetNumberOfCells();
            if (array->GetNumberOfTuples() != expected)
            {
              error = "A VTK result array has a tuple count inconsistent with its mesh.";
              return false;
            }
            if (field.array_name.rfind("Valid/",0) == 0)
            {
              continue;
            }
            if (arrays->HasArray(("Valid/"+field.array_name).c_str()))
            {
              field.validity_array = "Valid/"+field.array_name;
            }
            // Shape alone does not establish displacement semantics. Multiple
            // partitions can share one logical field/dataset descriptor.
            const auto duplicate = std::find_if(frame.fields.begin(),frame.fields.end(),[&](const auto& candidate)
            {
              return candidate.dataset_id == field.dataset_id && candidate.array_name == field.array_name &&
                candidate.association == field.association && candidate.components == field.components &&
                candidate.validity_array == field.validity_array;
            });
            if (duplicate == frame.fields.end())
            {
              frame.fields.push_back(std::move(field));
            }
          }
        }
      }
      data->increments.push_back(std::move(frame));
    }
    std::string after;
    if (!SimResults::result_file_identity({path},after,error) || after != identity)
    {
      error = "VTK result dependencies changed during import.";
      return false;
    }
    SimResults::PublishedResultSource candidate;
    candidate.reference = SimResults::FileSourceReference{"vtk",{path},identity};
    candidate.suggested_name = info.completeBaseName().toUtf8().toStdString();
    candidate.data = std::move(data);
    output = std::move(candidate);
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot import VTK results: ")+exception.what();
    return false;
  }
}
bool Postprocessor::VtkFileSourceProvider::restore(const SimResults::ResultSourceReference& request,
  SimResults::PublishedResultSource& output, std::string& error)
{
  const auto* files = std::get_if<SimResults::FileSourceReference>(&request);
  if (!files || files->paths.size() != 1 || !QDir::isAbsolutePath(QString::fromUtf8(files->paths.front().c_str())))
  {
    error = "The saved VTK reference needs a valid local path.";
    return false;
  }
  std::string identity;
  if (!SimResults::result_file_identity(files->paths,identity,error))
  {
    return false;
  }
  // Older schema-1 VTT references used the single file's SHA-256 directly.
  bool matches = identity == files->content_identity;
  if (!matches && files->content_identity.rfind("sha256:",0) == 0)
  {
    QFile file(QString::fromUtf8(files->paths.front().c_str()));
    QCryptographicHash hash(QCryptographicHash::Sha256);
    matches = file.open(QIODevice::ReadOnly) && hash.addData(&file) &&
      "sha256:"+hash.result().toHex().toStdString() == files->content_identity;
  }
  if (!matches)
  {
    error = "VTK result dependencies differ from the saved source.";
    return false;
  }
  SimResults::PublishedResultSource candidate;
  if (!open(request,candidate,error))
  {
    return false;
  }
  const auto* actual = std::get_if<SimResults::FileSourceReference>(&candidate.reference);
  if (!actual || actual->content_identity != identity)
  {
    error = "VTK result dependencies changed during restoration.";
    return false;
  }
  // Preserve an older verified reference until its next explicit refresh.
  auto resolved = *actual;
  resolved.content_identity = files->content_identity;
  candidate.reference = std::move(resolved);
  output = std::move(candidate);
  return true;
}
