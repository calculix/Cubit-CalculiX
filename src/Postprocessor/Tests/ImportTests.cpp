#include "Postprocessor/Core/sources/VtkFileSourceProvider.hpp"
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/GUI/Sources/PostprocessorSourceControls.hpp"
#include <QApplication>
#include <QTemporaryDir>
#include <QDirIterator>
#include <QFile>
#include <QComboBox>
#include <QToolButton>
#include <vtkUnstructuredGrid.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLPartitionedDataSetCollectionWriter.h>
#include <vtkSmartPointer.h>
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
}
int main(int argc,char** argv)
{
  QApplication application(argc,argv);
  QTemporaryDir directory;
  auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint(0,0,0); points->InsertNextPoint(1,0,0);
  grid->SetPoints(points); grid->Allocate(1);
  vtkIdType ids[] = {0,1};
  grid->InsertNextCell(VTK_LINE,2,ids);
  auto values = vtkSmartPointer<vtkDoubleArray>::New();
  values->SetName("Temperature"); values->InsertNextValue(20); values->InsertNextValue(80);
  grid->GetPointData()->AddArray(values);
  auto original = vtkSmartPointer<vtkTypeInt64Array>::New();
  original->SetName("OriginalNodeId"); original->InsertNextValue(9007199254740993LL); original->InsertNextValue(42);
  grid->GetPointData()->AddArray(original);
  const auto path = directory.filePath("mesh.vtu").toUtf8().toStdString();
  auto writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
  writer->SetFileName(path.c_str()); writer->SetInputData(grid);
  check(writer->Write() != 0,"Cannot write VTU fixture");
  Postprocessor::VtkFileSourceProvider provider;
  SimResults::PublishedResultSource publication;
  std::string error;
  check(provider.open(SimResults::FileSourceReference{"vtk",{path}},publication,error),"VTU import failed");
  if (publication.data && !publication.data->increments.empty())
  {
    auto* mesh = vtkUnstructuredGrid::SafeDownCast(publication.data->increments[0].dataset.get());
    auto* retained = mesh ? vtkTypeInt64Array::SafeDownCast(mesh->GetPointData()->GetArray("OriginalNodeId")) : nullptr;
    check(retained && retained->GetValue(0) == 9007199254740993LL,"VTU import changed original 64-bit node IDs");
    check(publication.data->increments[0].fields.size() == 2,"VTU field catalogue is missing numeric arrays");
  }
  Postprocessor::PostprocessorCore core;
  check(core.open_result(provider,SimResults::FileSourceReference{"vtk",{path}},std::string("first"),error),"Cannot add first VTU source");
  const auto first = core.session().active_source_id;
  check(core.open_result(provider,SimResults::FileSourceReference{"vtk",{path}},std::string("second"),error),"Cannot add comparison source");
  check(core.show_only_source(first,"view1",error),"Source isolation failed");
  for (const auto& item : core.session().representations)
  {
    check(item.second.visible == (item.second.input.object_id == first),"Source isolation did not isolate within the view");
  }
  const auto revision = core.display_revision();
  check(!core.open_result(provider,SimResults::FileSourceReference{"vtk",{path+"missing"}},std::string("bad"),error) &&
    core.display_revision() == revision && core.session().sources.size() == 2,"Failed import changed the committed session");
  auto collection = vtkSmartPointer<vtkPartitionedDataSetCollection>::New();
  collection->SetPartition(0,0,grid);
  const auto collectionPath = directory.filePath("collection.vtpc").toUtf8().toStdString();
  auto collectionWriter = vtkSmartPointer<vtkXMLPartitionedDataSetCollectionWriter>::New();
  collectionWriter->SetFileName(collectionPath.c_str()); collectionWriter->SetInputData(collection);
  check(collectionWriter->Write() != 0,"Cannot write VTPC fixture");
  SimResults::PublishedResultSource collectionSource, restored;
  check(provider.open(SimResults::FileSourceReference{"vtk",{collectionPath}},collectionSource,error),"VTPC import failed");
  if (collectionSource.data)
  {
    check(provider.restore(collectionSource.reference,restored,error),"Unchanged VTPC did not restore");
    QDirIterator dependencies(directory.path(),QStringList{"*.vtu"},QDir::Files,QDirIterator::Subdirectories);
    bool changed = false;
    while (dependencies.hasNext())
    {
      const auto dependency = dependencies.next();
      if (dependency != QString::fromUtf8(path.c_str()))
      {
        QFile file(dependency);
        if (file.open(QIODevice::Append))
        {
          file.write("\n"); changed = true;
        }
        break;
      }
    }
    check(changed,"No VTPC dependency was found for verification");
    check(!provider.restore(collectionSource.reference,restored,error),"Changed VTPC dependency was silently substituted");
  }
  int additions = 0, catalogues = 0;
  PostprocessorSourceControls controls([&](std::vector<SimResults::ResultJobEntry>& jobs,std::string& issue)
  {
    ++catalogues;
    SimResults::ResultJobEntry job;
    job.reference.application_id = "calculix"; job.reference.job_id = 12;
    job.application_name = "CalculiX"; job.command_prefix = "ccx"; job.job_name = "first_run"; job.available = true;
    jobs = {job}; issue.clear(); return true;
  },[&](const SimResults::ResultJobEntry& job)
  {
    ++additions;
    check(job.reference.job_id == 12,"Add selected the wrong job");
  },[]()
  {
  },nullptr);
  controls.refresh();
  auto* combo = controls.findChild<QComboBox*>();
  auto* add = controls.findChild<QToolButton*>("Postprocessor.AddJob");
  check(combo && add && catalogues == 1 && additions == 0,"Catalogue refresh added or loaded a source");
  if (combo && add)
  {
    combo->setCurrentIndex(1);
    check(additions == 0 && add->isEnabled(),"Choosing a job unexpectedly added it");
    add->click();
    check(additions == 1,"Add did not dispatch exactly once");
  }
  return failures ? 1 : 0;
}
