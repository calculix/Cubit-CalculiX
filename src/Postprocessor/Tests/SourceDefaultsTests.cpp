#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include <QApplication>
#include <QTreeWidgetItemIterator>
#include <vtkSmartPointer.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkCellType.h>
#include <iostream>
namespace
{
  int failures = 0;
  void check(bool value,const char* message)
  {
    if (!value)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data = std::make_shared<SimResults::VtkResultsData>();
    explicit Provider(bool blocks)
    {
      auto collection = vtkSmartPointer<vtkPartitionedDataSetCollection>::New();
      SimResults::Increment frame;
      const std::vector<std::string> kinds = blocks ? std::vector<std::string>{"Nodesets","Blocks","Surfaces","DAT_IntegrationPoints"} :
        std::vector<std::string>{"Nodesets","DAT_IntegrationPoints"};
      for (std::size_t index = 0; index < kinds.size(); ++index)
      {
        SimResults::Dataset dataset;
        dataset.id = index; dataset.name = "Part_"+std::to_string(index); dataset.kind = kinds[index];
        data->datasets.push_back(dataset);
        auto mesh = vtkSmartPointer<vtkUnstructuredGrid>::New();
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->InsertNextPoint(0,0,0); points->InsertNextPoint(1,0,0);
        mesh->SetPoints(points); mesh->Allocate(1);
        vtkIdType ids[] = {0,1}; mesh->InsertNextCell(VTK_LINE,2,ids);
        auto values = vtkSmartPointer<vtkDoubleArray>::New();
        values->SetName("values");
        const bool vector = kinds[index] == "Blocks";
        values->SetNumberOfComponents(vector ? 3 : 1);
        values->SetNumberOfTuples(2); values->Fill(1.0);
        mesh->GetPointData()->AddArray(values);
        SimResults::Field field;
        field.name = vector ? "DISP" : "A_hidden"; field.source = "test";
        field.array_name = "values"; field.dataset_id = index;
        field.components = vector ? std::vector<std::string>{"X","Y","Z"} : std::vector<std::string>{"Value"};
        if (vector)
        {
          field.vector_components = std::array<std::size_t,3>{0,1,2};
        }
        frame.fields.push_back(field);
        collection->SetPartition(static_cast<unsigned int>(index),0,mesh);
      }
      collection->Register(nullptr);
      frame.dataset = std::shared_ptr<vtkDataObject>(collection.GetPointer(),[](vtkDataObject* value)
      {
        value->Delete();
      });
      data->increments.push_back(frame);
    }
    std::string command_prefix() const override
    {
      return "fake";
    }
    bool open(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      auto reference = std::get<SimResults::JobSourceReference>(request);
      reference.run_identity = "retained";
      data->run_identity = reference.run_identity; data->job_id = reference.job_id;
      output = {reference,"Example",data}; error.clear(); return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      return open(request,output,error);
    }
  };
}
int main(int argc,char** argv)
{
  QApplication application(argc,argv);
  using namespace Postprocessor;
  Provider provider(true);
  PostprocessorCore core;
  std::string error;
  const SimResults::JobSourceReference request{"test","",1,""};
  check(core.open_result(provider,request,std::nullopt,error),"Cannot open default-display fixture");
  const auto source = core.session().active_source_id;
  const auto& display = core.session().representations.at(core.session().active_representation_id);
  check(display.hidden_datasets == std::set<std::size_t>{0,2,3},"Opening did not show blocks only");
  check(display.coloring.field && display.coloring.field->magnitude && display.coloring.field->array_name == "values" &&
    display.coloring.field->group_key == field_group_key(provider.data->increments[0].fields[1]),"Opening did not choose the first visible vector result");
  PostprocessorResultsTree tree([](const QString&)
  {
  },[](const QString&,bool)
  {
  });
  const auto refresh = [&]()
  {
    tree.refresh(core.session(),[&](const std::string& id)
    {
      return core.result_data(id);
    },[&](const std::string& id,std::size_t frame)
    {
      return core.frame_has_mesh(id,frame);
    });
  };
  refresh();
  QTreeWidgetItem* results = nullptr;
  for (QTreeWidgetItemIterator item(&tree); *item; ++item)
  {
    if ((*item)->text(0) == "Results")
    {
      results = *item;
    }
  }
  check(results && results->isExpanded(),"Results did not expand on first opening");
  check(tree.currentItem() && tree.currentItem()->text(0) == "DISP","Active result row was not selected");
  if (results)
  {
    results->setExpanded(false);
  }
  refresh();
  for (QTreeWidgetItemIterator item(&tree); *item; ++item)
  {
    if ((*item)->text(0) == "Results")
    {
      check(!(*item)->isExpanded(),"Refresh overwrote manual Results collapse");
    }
  }
  check(core.set_category_visible(source,"view1",DatasetCategory::NodeSets,true,error),"Cannot customize initial visibility");
  const auto customized = core.session().representations.at(core.session().active_representation_id).hidden_datasets;
  check(core.open_result(provider,request,std::nullopt,error) &&
    core.session().representations.at(core.session().active_representation_id).hidden_datasets == customized,
    "Reopening a retained source reset visibility choices");
  Provider fallback(false);
  PostprocessorCore other;
  check(other.open_result(fallback,request,std::nullopt,error),"Cannot open category-fallback fixture");
  const auto& fallbackDisplay = other.session().representations.at(other.session().active_representation_id);
  check(fallbackDisplay.hidden_datasets == std::set<std::size_t>{1},"No-block opening did not show the first available category only");
  check(fallbackDisplay.coloring.field && !fallbackDisplay.coloring.field->magnitude &&
    fallbackDisplay.coloring.field->component == "Value","Scalar default did not choose the first component");
  return failures ? 1 : 0;
}
