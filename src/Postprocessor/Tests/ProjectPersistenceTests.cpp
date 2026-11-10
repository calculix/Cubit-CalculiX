#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/persistence/PostprocessorSessionCodec.hpp"
#include "Postprocessor/Core/views/CameraOperations.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include <QApplication>
#include <QJsonArray>
#include <QTemporaryDir>
#include <QTreeWidgetItemIterator>
#include <vtkCellType.h>
#include <vtkDoubleArray.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <iostream>
namespace
{
  int failures = 0;
  void check(bool value, const char* message)
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
    Provider()
    {
      data->job_id = 1;
      data->run_identity = "runtime-token";
      data->result_identity = "sha256-v1:"+std::string(64,'a');
      SimResults::Dataset block;
      block.name = "Block_1"; block.kind = "Blocks"; block.model_id = 1;
      SimResults::Dataset nodes;
      nodes.id = 1; nodes.name = "load_target"; nodes.kind = "Nodesets"; nodes.model_id = 3;
      auto support = nodes;
      support.id = 2; support.name = "support_nodes"; support.model_id = 4;
      data->datasets = {block,nodes,support};
      for (int frame = 0; frame < 2; ++frame)
      {
        auto collection = vtkSmartPointer<vtkPartitionedDataSetCollection>::New();
        for (unsigned int index = 0; index < 3; ++index)
        {
          auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
          auto points = vtkSmartPointer<vtkPoints>::New();
          points->InsertNextPoint(0,0,0); points->InsertNextPoint(1,0,0);
          grid->SetPoints(points); grid->Allocate(1);
          vtkIdType ids[] = {0,1};
          grid->InsertNextCell(VTK_LINE,2,ids);
          collection->SetPartition(index,0,grid);
        }
        SimResults::Increment increment;
        increment.step = 1; increment.increment = frame+1; increment.time = 0.1*(frame+1);
        collection->Register(nullptr);
        increment.dataset = std::shared_ptr<vtkDataObject>(collection.GetPointer(),[](vtkDataObject* value)
        {
          value->Delete();
        });
        data->increments.push_back(increment);
      }
    }
    std::string command_prefix() const override
    {
      return "test";
    }
    bool open(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override
    {
      auto reference = std::get<SimResults::JobSourceReference>(request);
      reference.run_identity = data->run_identity;
      reference.result_identity = data->result_identity;
      output = {reference,"Saved",data};
      error.clear();
      return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override
    {
      return open(request,output,error);
    }
  };
}
int main(int argc, char** argv)
{
  QApplication application(argc,argv);
  using namespace Postprocessor;
  Provider provider;
  PostprocessorCore core;
  std::string error;
  check(core.open_result(provider,SimResults::JobSourceReference{"test","",1,""},std::nullopt,error),"Cannot open persistence fixture");
  const auto source = core.session().active_source_id;
  check(core.set_category_visible(source,"view1",DatasetCategory::NodeSets,true,error),"Cannot enable node sets for category-toggle verification");
  const auto initial = core.display_revision();
  check(core.set_category_visible(source,"view1",DatasetCategory::NodeSets,false,error),"Cannot hide node-set category");
  check(core.display_revision() == initial+1 && core.session().representations.begin()->second.hidden_datasets == std::set<std::size_t>{1,2},
    "Category visibility was not atomic or affected another category");
  const auto hiddenRevision = core.display_revision();
  check(core.set_category_visible(source,"view1",DatasetCategory::NodeSets,false,error) && core.display_revision() == hiddenRevision,
    "Repeated category hide changed scene revision");
  check(core.set_dataset_visibility(source,"view1",DatasetVisibilityAction::Show,1,error),"Cannot create mixed category state");
  CameraDefinition camera;
  camera.position = {8,9,10}; camera.focal_point = {1,2,3}; camera.view_up = {0,0,1};
  check(core.set_camera("view1",camera,error) && core.set_frame(source,"view1",1,error),"Cannot configure saved view");
  PostprocessorProjectState archive;
  check(core.capture_project(archive,error),"Cannot capture project definitions");
  auto& representation = archive.session.representations.begin()->second;
  representation.coloring.maximum = {true,250,{1,0,1}};
  representation.deformation.scale = 10;
  representation.animation.playing = true;
  QTemporaryDir directory;
  const auto json = project_to_json(archive,directory.path().toUtf8().toStdString());
  PostprocessorProjectState decoded;
  check(project_from_json(json,directory.path().toUtf8().toStdString(),decoded,error),"Project codec round-trip failed");
  check(decoded.session.views.at("view1").camera && CameraOperations::equal(*decoded.session.views.at("view1").camera,camera),"Codec changed camera pose");
  check(!decoded.session.representations.begin()->second.animation.playing &&
    decoded.session.representations.begin()->second.coloring.maximum.enabled,"Playback/limit persistence is incorrect");
  PreparedPostprocessorProject prepared;
  prepared.definitions = decoded;
  prepared.runtime.emplace(source,SourceRuntime{provider.data,1,0});
  check(core.bind_project(prepared,error) && core.restore_project(std::move(prepared),error),"Cannot publish resolved project");
  check(!core.session().views.at("view1").fit_pending && CameraOperations::equal(*core.session().views.at("view1").camera,camera),"Restore refitted a saved camera");
  check(core.session().representations.begin()->second.increment_index == 1 &&
    core.session().representations.begin()->second.hidden_datasets == std::set<std::size_t>{2},"Frame/dataset identity did not restore");
  // Missing sources retain all durable bindings through another save.
  PreparedPostprocessorProject missing;
  missing.definitions = decoded;
  SourceRuntime unavailable;
  unavailable.availability = SourceAvailability::Unavailable;
  unavailable.unavailable_reason = "Saved file missing";
  unavailable.datasets = decoded.catalogues.at(source);
  missing.runtime.emplace(source,unavailable);
  check(core.bind_project(missing,error) && core.restore_project(std::move(missing),error),"Cannot publish unavailable source definitions");
  PostprocessorProjectState savedAgain;
  check(core.capture_project(savedAgain,error) && savedAgain.bindings.begin()->second.hidden.size() == 1 &&
    savedAgain.session.representations.begin()->second.deformation.scale == 10 && !core.result_data(source),
    "Saving an unavailable source discarded its settings");
  PostprocessorResultsTree tree([](const QString&)
  {
  },[](const QString&,bool)
  {
  },nullptr);
  auto refreshTree = [&]()
  {
    tree.refresh(core.session(),[&](const std::string& id)
    {
      return core.result_data(id);
    },[&](const std::string& id,std::size_t frame)
    {
      return core.frame_has_mesh(id,frame);
    },{},[&](const std::string& id)
    {
      return core.mesh_datasets(id);
    });
  };
  PreparedPostprocessorProject loaded;
  loaded.definitions = decoded;
  loaded.runtime.emplace(source,SourceRuntime{provider.data,1,0});
  check(core.bind_project(loaded,error) && core.restore_project(std::move(loaded),error),"Cannot prepare category tree fixture");
  refreshTree();
  QTreeWidgetItem* nodeCategory = nullptr;
  for (QTreeWidgetItemIterator item(&tree); *item; ++item)
  {
    if ((*item)->text(0) == "Node sets")
    {
      nodeCategory = *item;
    }
  }
  check(nodeCategory && nodeCategory->checkState(0) == Qt::PartiallyChecked,"Mixed category does not show a partial checkbox");
  int categoryActions = 0;
  tree.category_action = [&](const QString&,DatasetCategory category,bool visible)
  {
    ++categoryActions;
    check(category == DatasetCategory::NodeSets && !visible,"Category checkbox dispatched the wrong action");
  };
  if (nodeCategory)
  {
    nodeCategory->setCheckState(0,Qt::Unchecked);
    QApplication::processEvents();
  }
  check(categoryActions == 1,"Category checkbox did not dispatch exactly one action");
  PreparedPostprocessorProject missingAgain;
  missingAgain.definitions = decoded;
  missingAgain.runtime.emplace(source,unavailable);
  check(core.bind_project(missingAgain,error) && core.restore_project(std::move(missingAgain),error),"Cannot restore unavailable tree fixture");
  tree.refresh(core.session(),[&](const std::string& id)
  {
    return core.result_data(id);
  },[&](const std::string& id,std::size_t frame)
  {
    return core.frame_has_mesh(id,frame);
  },{},[&](const std::string& id)
  {
    return core.mesh_datasets(id);
  },{},[&](const std::string& id)
  {
    return core.source_unavailable_reason(id);
  });
  check(tree.topLevelItem(0) && tree.topLevelItem(0)->text(0).contains("Unavailable"),"Unavailable source was not labelled");
  bool meshLayer = false, blocks = false;
  for (QTreeWidgetItemIterator item(&tree); *item; ++item)
  {
    meshLayer = meshLayer || (*item)->text(0) == "Mesh";
    blocks = blocks || (*item)->text(0) == "Blocks / Element sets";
  }
  check(!meshLayer && blocks,"Flattened source catalogue was not retained");
  auto bad = json;
  auto sources = bad.value("sources").toArray(); sources.append(sources.first()); bad.insert("sources",sources);
  const auto previous = savedAgain.session.sources.size();
  check(!project_from_json(bad,directory.path().toUtf8().toStdString(),savedAgain,error) &&
    savedAgain.session.sources.size() == previous,"Malformed project replaced committed definitions");
  return failures ? 1 : 0;
}
