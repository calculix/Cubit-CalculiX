#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/persistence/PostprocessorSessionCodec.hpp"
#include "Postprocessor/Commands/display/AppearanceCommandText.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorScene.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorStyleDelegate.hpp"
#include "Postprocessor/GUI/Properties/PostprocessorAppearanceControls.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include <QApplication>
#include <QComboBox>
#include <QPushButton>
#include <QTreeWidgetItemIterator>
#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkProperty.h>
#include <vtkTypeInt64Array.h>
#include <vtkUnstructuredGrid.h>
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
  std::shared_ptr<SimResults::VtkResultsData> fixture()
  {
    auto data = std::make_shared<SimResults::VtkResultsData>();
    data->job_id = 1;
    data->run_identity = "appearance-fixture";
    data->layout = SimResults::DatasetLayout::Collection;
    data->datasets = {{0,"Block_1","Blocks",11},{1,"Block_2","Blocks",12},{2,"Support","Nodesets",13}};
    data->datasets[0].solid_color = std::array<double,3>{0,1,0};
    data->datasets[1].solid_color = std::array<double,3>{1,1,0};
    auto collection = vtkSmartPointer<vtkPartitionedDataSetCollection>::New();
    SimResults::Increment frame;
    frame.step = frame.increment = 1;
    frame.time = .1;
    for (unsigned int dataset = 0; dataset < 3; ++dataset)
    {
      auto mesh = vtkSmartPointer<vtkUnstructuredGrid>::New();
      auto points = vtkSmartPointer<vtkPoints>::New();
      points->SetDataTypeToDouble();
      const double x = dataset*2;
      points->InsertNextPoint(x,0,0);
      points->InsertNextPoint(x+1,0,0);
      points->InsertNextPoint(x,1,0);
      points->InsertNextPoint(x,0,1);
      mesh->SetPoints(points);
      mesh->Allocate(4);
      auto ids = vtkSmartPointer<vtkTypeInt64Array>::New();
      ids->SetName("OriginalNodeId");
      auto values = vtkSmartPointer<vtkDoubleArray>::New();
      values->SetName("VALUE");
      for (vtkIdType i = 0; i < 4; ++i)
      {
        ids->InsertNextValue(1000+dataset*10+i);
        values->InsertNextValue(i);
        if (dataset == 2)
        {
          mesh->InsertNextCell(VTK_VERTEX,1,&i);
        }
      }
      mesh->GetPointData()->AddArray(ids);
      mesh->GetPointData()->AddArray(values);
      if (dataset < 2)
      {
        vtkIdType nodes[] = {0,1,2,3};
        mesh->InsertNextCell(VTK_TETRA,4,nodes);
        auto elements = vtkSmartPointer<vtkTypeInt64Array>::New();
        elements->SetName("OriginalElementId");
        elements->InsertNextValue(100+dataset);
        mesh->GetCellData()->AddArray(elements);
        SimResults::Field field;
        field.name = field.array_name = "VALUE";
        field.components = {"V"};
        field.dataset_id = dataset;
        frame.fields.push_back(field);
      }
      collection->SetPartition(dataset,0,mesh);
    }
    collection->Register(nullptr);
    frame.dataset = std::shared_ptr<vtkDataObject>(collection.GetPointer(),[](vtkDataObject* value)
    {
      value->Delete();
    });
    data->increments.push_back(frame);
    return data;
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data = fixture();
    std::string command_prefix() const override
    {
      return "test";
    }
    bool open(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      auto resolved = std::get<SimResults::JobSourceReference>(request);
      resolved.run_identity = data->run_identity;
      output = {resolved,"Model",data};
      error.clear();
      return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      return open(request,output,error);
    }
  };
  class Registry : public ResultSourceRegistryInterface
  {
  public:
    mutable Provider provider;
    bool resolve_result_provider(const std::string&,ResolvedResultSourceProvider& output,std::string&) const override
    {
      output.application_id = "test";
      output.provider = &provider;
      return true;
    }
  };
}
int main(int argc,char** argv)
{
  QApplication qt(argc,argv);
  using namespace Postprocessor;
  Registry registry;
  auto app = std::make_shared<PostprocessorApplicationInterface>(registry);
  std::string error;
  check(app->open_job("test",1,std::nullopt,error),"Cannot open appearance fixture");
  const auto source = app->session().active_source_id;
  const auto view = app->session().active_view_id;
  std::vector<AppearanceTarget> blocks{{source,view,0},{source,view,1}};
  AppearancePatch wire;
  wire.style = DisplayStyle::Wireframe;
  const auto initial = app->display_revision();
  check(app->change_dataset_appearance(blocks,wire,error) && app->display_revision() == initial+1,
    "Batch appearance did not commit once");
  check(app->dataset_appearance(source,view,0).style == DisplayStyle::Wireframe &&
    app->dataset_appearance(source,view,1).style == DisplayStyle::Wireframe,"Batch missed a block");
  AppearancePatch shaded;
  shaded.style = DisplayStyle::Shaded;
  const auto revision = app->display_revision();
  check(!app->change_dataset_appearance({blocks[0],{source,view,2}},shaded,error) && app->display_revision() == revision &&
    app->dataset_appearance(source,view,0).style == DisplayStyle::Wireframe,"Incompatible mixed batch partially committed");
  AppearancePatch color;
  color.color_change = ColorChange::Override;
  color.color = {.2,.4,.6};
  check(app->change_dataset_appearance(blocks,color,error),"Cannot set per-block solid colors");
  const auto journal = appearance_commands(*app,blocks,wire,error);
  check(journal.size() == 1 && journal.front() == "post display style wireframe source \"Model\" blocks \"Block_1\" \"Block_2\"",
    "Appearance journal is not readable or lost explicit targets");
  AppearancePatch points;
  points.style = DisplayStyle::Points;
  check(app->change_dataset_appearance({blocks[0]},points,error),"Cannot change individual block style");
  check(app->dataset_appearance(source,view,1).style == DisplayStyle::Wireframe,"Individual change altered another block");
  CameraDefinition camera;
  camera.position = {.2,.2,5};
  camera.focal_point = {.2,.2,0};
  check(app->set_camera(camera,error),"Cannot set appearance picking camera");
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  auto window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  window->SetSize(400,400);
  window->AddRenderer(renderer);
  PostprocessorScene scene(renderer,view);
  check(scene.synchronize(*app,error),"Cannot build styled scene");
  renderer->SetWorldPoint(0,0,1,1);
  renderer->WorldToDisplay();
  std::optional<EntityReference> picked;
  check(scene.pick(renderer->GetDisplayPoint()[0],renderer->GetDisplayPoint()[1],EntityKind::Node,picked,error) &&
    picked && picked->entity_id == 1003,"Point style lost solver-node picking");
  if (picked)
  {
    check(scene.set_hover(picked,*app) && scene.clear_hover(),"Styled geometry lost hover feedback");
  }
  PostprocessorResultsTree tree([](const QString&)
  {
  },[](const QString&,bool)
  {
  });
  tree.appearance_value = [&](const AppearanceTarget& target)
  {
    return app->dataset_appearance(target.source_id,target.view_id,target.dataset_id);
  };
  const auto refreshTree = [&]()
  {
    tree.refresh(app->session(),[&](const std::string& id)
    {
      return app->result_data(id);
    },[&](const std::string& id,std::size_t frame)
    {
      return app->frame_has_mesh(id,frame);
    });
  };
  refreshTree();
  check(tree.columnCount() == 3 && tree.isColumnHidden(1),"Style column did not replace default ID visibility");
  tree.clearSelection();
  for (QTreeWidgetItemIterator item(&tree); *item; ++item)
  {
    if ((*item)->text(0).startsWith("Block_"))
    {
      (*item)->setSelected(true);
    }
  }
  check(tree.appearance_targets().size() == 2,"Tree did not support multiple dataset selections");
  refreshTree();
  check(tree.appearance_targets().size() == 2,"Tree refresh discarded multiple selection");
  auto reordered = fixture();
  reordered->datasets[0].id = 1;
  reordered->datasets[1].id = 0;
  auto* collection = vtkPartitionedDataSetCollection::SafeDownCast(reordered->increments[0].dataset.get());
  vtkSmartPointer<vtkPartitionedDataSet> first = collection->GetPartitionedDataSet(0);
  vtkSmartPointer<vtkPartitionedDataSet> second = collection->GetPartitionedDataSet(1);
  collection->SetPartitionedDataSet(0,second);
  collection->SetPartitionedDataSet(1,first);
  for (auto& field : reordered->increments[0].fields)
  {
    field.dataset_id = 1-field.dataset_id;
  }
  registry.provider.data = reordered;
  check(app->refresh_source("Model",error) && app->dataset_appearance(source,view,1).style == DisplayStyle::Points &&
    app->dataset_appearance(source,view,0).style == DisplayStyle::Wireframe,"Refresh attached appearance to reused collection indices");
  refreshTree();
  check(tree.appearance_targets().size() == 2,"Dataset reordering lost stable tree selection");
  int appliedCommands = 0;
  const auto selectedTargets = tree.appearance_targets();
  const auto beforeColor = app->dataset_appearance(source,view,selectedTargets.front().dataset_id).solid_color;
  PostprocessorAppearanceControls controls(app,[&](const std::string& command,std::string& issue)
  {
    ++appliedCommands;
    check(command.find("post display style shaded") == 0,"Properties journaled an untouched mixed property");
    return app->change_dataset_appearance(selectedTargets,shaded,issue);
  },nullptr);
  controls.set_targets(selectedTargets);
  auto* styleEditor = controls.findChild<QComboBox*>("Postprocessor.Appearance.Style");
  auto* apply = controls.findChild<QPushButton*>("Postprocessor.Appearance.Apply");
  check(styleEditor && apply,"Appearance source properties are missing");
  if (styleEditor && apply)
  {
    const int option = styleEditor->findData(static_cast<int>(DisplayStyle::Shaded));
    styleEditor->setCurrentIndex(option);
    QMetaObject::invokeMethod(styleEditor,"activated",Qt::DirectConnection,Q_ARG(int,option));
    apply->click();
    check(appliedCommands == 1 && app->dataset_appearance(source,view,selectedTargets.front().dataset_id).solid_color == beforeColor,
      "Mixed appearance edit changed untouched colors or emitted extra commands");
  }
  PostprocessorCore core;
  check(core.open_result(registry.provider,SimResults::JobSourceReference{"test","",1,""},std::nullopt,error),"Cannot open persistence fixture");
  const auto savedSource = core.session().active_source_id;
  check(core.change_dataset_appearance({{savedSource,"view1",0},{savedSource,"view1",1}},color,error),"Cannot set saved appearance");
  PostprocessorProjectState saved,decoded;
  check(core.capture_project(saved,error) && project_from_json(project_to_json(saved,"."),".",decoded,error),"Appearance codec failed");
  PreparedPostprocessorProject prepared;
  prepared.definitions = decoded;
  prepared.runtime.emplace(savedSource,SourceRuntime{registry.provider.data,1,0});
  check(core.bind_project(prepared,error) && core.restore_project(std::move(prepared),error) &&
    core.dataset_appearance(savedSource,"view1",0).solid_color == std::optional<std::array<double,3>>(color.color),
    "Project restore lost solid color overrides");
  AppearancePatch native;
  native.color_change = ColorChange::Native;
  check(core.change_dataset_appearance({{savedSource,"view1",0}},native,error) &&
    !core.dataset_appearance(savedSource,"view1",0).solid_color,"Native reset did not remove manual RGB");
  return failures;
}
