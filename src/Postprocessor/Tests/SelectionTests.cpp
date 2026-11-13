#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "AnnotationTests.hpp"
#include "PlotTests.hpp"
#include "CaptureTests.hpp"
#include "Postprocessor/Commands/selection/PickCommandText.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorScene.hpp"
#include "Postprocessor/GUI/Viewport/CameraInteraction.hpp"
#include "Postprocessor/GUI/Selection/PostprocessorPicking.hpp"
#include "Postprocessor/GUI/Selection/PostprocessorPickControls.hpp"
#include "Postprocessor/GUI/Selection/PostprocessorProbeProperties.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include <QApplication>
#include <QToolButton>
#include <QLabel>
#include <QMouseEvent>
#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDoubleArray.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkIdTypeArray.h>
#include <vtkOpenGLRenderer.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkTypeInt64Array.h>
#include <vtkUnstructuredGrid.h>
#include <cmath>
#include <iostream>
#include <limits>

namespace
{
  int failures = 0;
  constexpr std::int64_t largeNode = 9007199254740993LL;
  void check(bool condition, const char* message)
  {
    if (!condition)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  }
  std::shared_ptr<SimResults::VtkResultsData> fixture()
  {
    auto data = std::make_shared<SimResults::VtkResultsData>();
    data->job_id = 1;
    data->run_identity = "selection-test";
    SimResults::Dataset mesh;
    mesh.name = "Mesh";
    SimResults::Dataset ip;
    ip.id = 1;
    ip.name = "Integration points";
    data->datasets = {mesh,ip};
    for (int frame = 0; frame < 2; ++frame)
    {
      auto collection = vtkSmartPointer<vtkPartitionedDataSetCollection>::New();
      SimResults::Increment increment;
      increment.step = 1;
      increment.increment = frame+1;
      increment.time = frame+0.1;
      for (unsigned int dataset = 0; dataset < 2; ++dataset)
      {
        auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetDataTypeToDouble();
        if (dataset == 0)
        {
          points->InsertNextPoint(0,0,0);
          points->InsertNextPoint(1,0,0);
          points->InsertNextPoint(0,1,0);
          points->InsertNextPoint(0,0,1);
        }
        else
        {
          points->InsertNextPoint(0.25,0.25,0.25);
        }
        grid->SetPoints(points);
        grid->Allocate(1);
        vtkIdType ids[] = {0,1,2,3};
        grid->InsertNextCell(dataset == 0 ? VTK_TETRA : VTK_VERTEX,dataset == 0 ? 4 : 1,ids);
        auto pointIds = vtkSmartPointer<vtkTypeInt64Array>::New();
        pointIds->SetName(dataset == 0 ? "OriginalNodeId" : "OriginalElementId");
        for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
        {
          pointIds->InsertNextValue(dataset == 1 ? 120 : i == 0 ? (frame == 0 ? largeNode : 999) : 42+i);
        }
        grid->GetPointData()->AddArray(pointIds);
        auto cellIds = vtkSmartPointer<vtkTypeInt64Array>::New();
        cellIds->SetName("OriginalElementId");
        cellIds->InsertNextValue(120);
        grid->GetCellData()->AddArray(cellIds);
        if (dataset == 1)
        {
          auto ipIds = vtkSmartPointer<vtkTypeInt64Array>::New();
          ipIds->SetName("IntegrationPointId");
          ipIds->InsertNextValue(frame == 0 ? 3 : 4);
          grid->GetPointData()->AddArray(ipIds);
        }
        auto values = vtkSmartPointer<vtkDoubleArray>::New();
        values->SetName(dataset == 0 ? "stress" : "ip_stress");
        auto mask = vtkSmartPointer<vtkDoubleArray>::New();
        mask->SetName("valid_stress");
        auto vectors = vtkSmartPointer<vtkDoubleArray>::New();
        vectors->SetName("motion");
        vectors->SetNumberOfComponents(4);
        auto vectorMask = vtkSmartPointer<vtkDoubleArray>::New();
        vectorMask->SetName("valid_motion");
        for (vtkIdType i = 0; i < points->GetNumberOfPoints(); ++i)
        {
          values->InsertNextValue(dataset == 0 ? 10*(i+1) : 100);
          mask->InsertNextValue(i == 1 ? 0 : 1);
          const double xyz[] = {3,999,1,2}; // Z, magnitude, X, Y.
          vectors->InsertNextTuple(xyz);
          vectorMask->InsertNextValue(i == 1 ? 0 : 1);
        }
        grid->GetPointData()->AddArray(values);
        grid->GetPointData()->AddArray(mask);
        grid->GetPointData()->AddArray(vectors);
        grid->GetPointData()->AddArray(vectorMask);
        auto cellValues = vtkSmartPointer<vtkDoubleArray>::New();
        cellValues->SetName("cell_stress");
        cellValues->InsertNextValue(84);
        grid->GetCellData()->AddArray(cellValues);
        SimResults::Field field;
        field.name = dataset == 0 ? "Stress" : "IPStress";
        field.array_name = values->GetName();
        field.validity_array = "valid_stress";
        field.components = {"SXX"};
        field.dataset_id = dataset;
        field.association = dataset == 0 ? SimResults::Association::Node : SimResults::Association::IntegrationPoint;
        increment.fields.push_back(field);
        auto motion = field;
        motion.name = dataset == 0 ? "DISP" : "IntegrationPointDisplacement";
        motion.array_name = "motion";
        motion.validity_array = "valid_motion";
        motion.components = {"Z","Magnitude","X","Y"};
        motion.vector_components = std::array<std::size_t,3>{2,3,0};
        motion.role = SimResults::FieldRole::Displacement;
        increment.fields.push_back(motion);
        if (dataset == 0)
        {
          auto cells = field;
          cells.name = "CellStress";
          cells.array_name = "cell_stress";
          cells.validity_array.clear();
          cells.association = SimResults::Association::Element;
          increment.fields.push_back(cells);
        }
        auto partition = vtkSmartPointer<vtkPartitionedDataSet>::New();
        partition->SetPartition(0,grid);
        collection->SetPartitionedDataSet(dataset,partition);
      }
      collection->Register(nullptr);
      increment.dataset = std::shared_ptr<vtkDataObject>(collection.GetPointer(),[](vtkDataObject* value)
      {
        value->Delete();
      });
      data->increments.push_back(increment);
    }
    return data;
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data = fixture();
    std::string command_prefix() const override
    {
      return "fake";
    }
    bool open(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& result,std::string& error) override
    {
      auto reference = std::get<SimResults::JobSourceReference>(request);
      reference.run_identity = data->run_identity;
      result = {reference,"Model",data};
      error.clear();
      return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& result,std::string& error) override
    {
      return open(request,result,error);
    }
  };
  class Registry : public ResultSourceRegistryInterface
  {
  public:
    mutable Provider provider;
    bool resolve_result_provider(const std::string&,ResolvedResultSourceProvider& result,std::string&) const override
    {
      result.application_id = "test";
      result.provider = &provider;
      return true;
    }
  };
}

int main(int argc,char** argv)
{
  QApplication qt(argc,argv);
  using namespace Postprocessor;
  using Selector = PostprocessorApplicationInterface::DatasetSelector;
  Registry registry;
  auto app = std::make_shared<PostprocessorApplicationInterface>(registry);
  std::string error;
  check(app->open_job("fake",1,std::nullopt,error),"Cannot open probe fixture");
  check(app->color_field("Model","stress","SXX",false,0,error),"Cannot set probe field");
  failures += annotation_cases(app);
  failures += plot_cases(app);
  failures += capture_cases(app);
  const auto source = app->session().active_source_id;
  const auto view = app->session().active_view_id;
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  PostprocessorScene scene(renderer,view);
  check(scene.synchronize(*app,error),"Cannot create selection scene");
  renderer->GetActors()->InitTraversal();
  vtkSmartPointer<vtkActor> originalActor = renderer->GetActors()->GetNextActor();
  const auto revision = app->display_revision();
  check(app->pick_entity("Model",Selector{std::string("Mesh")},EntityKind::Node,largeNode,std::nullopt,error),"Exact 64-bit node selection failed");
  auto result = app->probe(error);
  check(result && result->available && result->values && (*result->values)[0] == 10 &&
    result->entity.entity_id == largeNode && app->display_revision() == revision,"Probe lost ID precision, sample value or rebuilt display revision");
  if (!result)
  {
    return 1;
  }
  check(result->element_ids == std::vector<std::int64_t>{120},"Node probe did not report its connected element");
  const auto hoverRevision = app->session().views.at(view).selection.revision;
  check(app->probe_entity(EntityReference{source,0,EntityKind::Node,largeNode,{}},error).has_value() &&
    app->session().views.at(view).selection.revision == hoverRevision,"Read-only hover probe changed selection");
  check(app->open_table("Model","stress",true,error),"Cannot open spatial field as table");
  const auto fieldRows = app->table_data();
  check(fieldRows.rows == 4 && result_table_cell(fieldRows,0,5) == "9007199254740993" &&
    result_table_cell(fieldRows,1,6) == "Unavailable","Spatial table lost original identity or validity mask");
  check(app->show_3d(error),"Cannot return from field table to 3D");
  check(scene.synchronize(*app,error),"Cannot highlight selected node");
  renderer->GetActors()->InitTraversal();
  check(renderer->GetActors()->GetNextActor() == originalActor,"Selection rebuilt mesh actors");
  std::string command;
  check(pick_command_text(*app,result->entity,command,error) && command.find("\"9007199254740993\"") != std::string::npos &&
    command.find("dataset \"Mesh\"") != std::string::npos,"Pick journal lost exact ID or dataset name");
  auto deformation = app->deformation("Model",error);
  if (!deformation)
  {
    check(false,"Deformation fixture is unavailable");
    return 1;
  }
  deformation->enabled = true;
  deformation->scale = 2;
  check(app->set_deformation("Model",*deformation,error),"Cannot deform probe fixture");
  result = app->probe(error);
  check(result && result->original_position == std::array<double,3>{0,0,0} &&
    result->displayed_position == std::array<double,3>{2,4,6},"Probe displayed coordinates differ from deformation semantics");
  check(app->pick_entity("Model",Selector{std::size_t{0}},EntityKind::Node,43,std::nullopt,error),"Cannot select masked node");
  result = app->probe(error);
  check(result && !result->values && result->invalid_samples == 1 &&
    result->displayed_position == result->original_position,"Masked probe invented a value or displaced an invalid sample");
  check(app->pick_entity("Model",Selector{std::size_t{0}},EntityKind::Element,120,std::nullopt,error),"Cannot pick original element");
  result = app->probe(error);
  check(result && result->range_value && result->values == std::array<double,2>{10,40} && result->valid_samples == 3 &&
    result->invalid_samples == 1 && !result->original_position,"Element nodal probe averaged/extrapolated or invented coordinates");
  check(app->color_field("Model","cell_stress","SXX",false,0,error),"Cannot set element probe field");
  result = app->probe(error);
  check(result && result->values == std::array<double,2>{84,84} && !result->range_value,"Cell-associated probe did not use stored sample");
  check(app->pick_entity("Model",Selector{std::size_t{1}},EntityKind::IntegrationPoint,120,3,error) &&
    app->color_field("Model","ip_stress","SXX",false,1,error),"Cannot select integration point identity");
  result = app->probe(error);
  check(result && result->values == std::array<double,2>{100,100},"Integration point probe returned wrong stored value");
  check(app->set_frame("Model",2,error),"Cannot change probe frame");
  result = app->probe(error);
  check(result && !result->available && !result->values,"Unavailable frame retained old probe values");
  check(app->set_frame("Model",1,error),"Cannot restore a compatible frame");
  result = app->probe(error);
  check(result && result->available,"Selection did not return in a compatible frame");
  check(app->set_dataset_visibility("Model",DatasetVisibilityAction::Hide,Selector{std::size_t{1}},error),"Cannot hide selected dataset");
  result = app->probe(error);
  check(result && !result->visible,"Hidden dataset probe status was not updated");
  check(!app->pick_entity("Model",Selector{std::size_t{0}},EntityKind::Node,12345,std::nullopt,error),"Unknown original ID accepted");
  PostprocessorProbeProperties properties(app,nullptr);
  const auto* readout = properties.findChild<QLabel*>("Postprocessor.ProbeResult");
  check(readout && readout->text().contains("Integration point: 3"),"Properties did not show selection readout");
  int modeCommands = 0;
  PostprocessorPickControls controls(app,[&](const QString& text)
  {
    ++modeCommands;
    check(text == "post pick mode node","Unexpected pick mode command");
    app->set_pick_mode(EntityKind::Node,error);
  },nullptr);
  controls.refresh();
  check(modeCommands == 0,"Pick toolbar refresh submitted commands");
  auto* nodeButton = controls.findChild<QToolButton*>("Postprocessor.PickNode");
  auto* elementButton = controls.findChild<QToolButton*>("Postprocessor.PickElement");
  auto* ipButton = controls.findChild<QToolButton*>("Postprocessor.PickIntegrationPoint");
  check(nodeButton && elementButton && ipButton,"Pick mode icon buttons are missing");
  if (nodeButton && elementButton && ipButton)
  {
    nodeButton->click();
    check(nodeButton->isChecked() && !elementButton->isChecked() && !ipButton->isChecked(),"Pick mode buttons are not exclusive");
    controls.refresh();
    check(modeCommands == 1,"Pick button refresh submitted a command");
  }
  check(modeCommands == 1 && app->session().views.at(view).selection.mode == EntityKind::Node,"Pick mode action failed");
  check(app->clear_pick(error) && !app->probe(error),"Clear did not remove probe");

  deformation->enabled = false;
  check(app->set_deformation("Model",*deformation,error) &&
    app->set_dataset_visibility("Model",DatasetVisibilityAction::ShowOnly,Selector{std::size_t{1}},error),"Cannot prepare CPU picking");
  CameraDefinition camera;
  camera.position = {0.2,0.2,5};
  camera.focal_point = {0.2,0.2,0};
  check(app->set_camera(camera,error),"Cannot set picking camera");
  auto window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  window->SetSize(400,400);
  window->AddRenderer(renderer);
  check(scene.synchronize(*app,error),"Cannot synchronize CPU picking scene");
  renderer->SetWorldPoint(0.25,0.25,0.25,1);
  renderer->WorldToDisplay();
  const double x = renderer->GetDisplayPoint()[0], y = renderer->GetDisplayPoint()[1];
  std::optional<EntityReference> picked;
  check(scene.pick(x,y,EntityKind::IntegrationPoint,picked,error) && picked && picked->entity_id == 120 &&
    picked->integration_point == 3,"CPU point picking lost integration-point provenance");
  check(app->set_dataset_visibility("Model",DatasetVisibilityAction::ShowAll,std::nullopt,error) && scene.synchronize(*app,error),"Cannot show occluding mesh");
  check(scene.pick(x,y,EntityKind::IntegrationPoint,picked,error) && !picked,"Point picker selected an occluded integration point");
  renderer->SetWorldPoint(0,0,1,1);
  renderer->WorldToDisplay();
  check(scene.pick(renderer->GetDisplayPoint()[0],renderer->GetDisplayPoint()[1],EntityKind::Node,picked,error) &&
    picked && picked->entity_id == 45,"Original mesh node marker did not pick its solver node ID");
  renderer->SetWorldPoint(0.2,0.2,0.6,1);
  renderer->WorldToDisplay();
  check(scene.pick(renderer->GetDisplayPoint()[0],renderer->GetDisplayPoint()[1],EntityKind::Element,picked,error) &&
    picked && picked->entity_id == 120,"Surface element pick did not retain original element ID");
  const auto selectionBeforeHover = app->session().views.at(view).selection.revision;
  if (picked)
  {
    check(scene.set_hover(picked,*app),"Hover did not create a contour");
    check(!scene.set_hover(picked,*app),"Unchanged hover rebuilt its contour");
    check(app->session().views.at(view).selection.revision == selectionBeforeHover,"Hover mutated Core selection");
    check(scene.clear_hover() && !scene.clear_hover(),"Hover did not clear exactly once");
    scene.set_hover(picked,*app);
    check(app->set_pick_mode(EntityKind::Node,error) && scene.synchronize(*app,error) &&
      !scene.clear_hover(),"Pick mode change retained hover feedback");
  }
  for (auto kind : {EntityKind::Node,EntityKind::Element,EntityKind::IntegrationPoint})
  {
    check(scene.pick(1,1,kind,picked,error) && !picked,"Empty viewport space retained an entity");
  }
  check(scene.pick(-10,-10,EntityKind::Element,picked,error) && !picked,"Outside-viewport pick retained an entity");
  check(app->clear_pick(error) && !app->probe(error),"Background clear retained probe values");

  // A quadratic cell is expanded into intermediate surface cells by VTK. The
  // surface tuple index must never replace the copied original 64-bit solver ID.
  auto quadratic = vtkSmartPointer<vtkUnstructuredGrid>::New();
  auto quadraticPoints = vtkSmartPointer<vtkPoints>::New();
  quadraticPoints->SetDataTypeToDouble();
  const double coordinates[10][3] = {{0,0,0},{1,0,0},{0,1,0},{0,0,1},
    {0.5,0,0},{0.5,0.5,0},{0,0.5,0},{0,0,0.5},{0.5,0,0.5},{0,0.5,0.5}};
  for (const auto& position : coordinates)
  {
    quadraticPoints->InsertNextPoint(position);
  }
  quadratic->SetPoints(quadraticPoints);
  quadratic->Allocate(1);
  vtkIdType quadraticIds[] = {0,1,2,3,4,5,6,7,8,9};
  quadratic->InsertNextCell(VTK_QUADRATIC_TETRA,10,quadraticIds);
  auto originalElements = vtkSmartPointer<vtkTypeInt64Array>::New();
  originalElements->SetName("OriginalElementId");
  originalElements->InsertNextValue(largeNode);
  quadratic->GetCellData()->AddArray(originalElements);
  auto quadraticSurface = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
  Postprocessor::configure_pick_provenance(quadraticSurface);
  quadraticSurface->SetInputData(quadratic);
  quadraticSurface->Update();
  check(quadraticSurface->GetOutput()->GetNumberOfCells() > 1,"Quadratic fixture did not create intermediate faces");
  for (vtkIdType i = 0; i < quadraticSurface->GetOutput()->GetNumberOfCells(); ++i)
  {
    EntityReference original;
    check(entity_from_index(quadraticSurface->GetOutput(),static_cast<std::size_t>(i),EntityKind::Element,
      "source",0,original,error) && original.entity_id == largeNode,"Quadratic surface lost exact original element identity");
  }

  QWidget surface;
  surface.resize(800,600);
  int clicks = 0;
  CameraInteraction interaction(&surface,view,app,{}, {},[]()
  {
  },[](const std::string&)
  {
  },nullptr,{},[&](const QPointF&)
  {
    ++clicks;
  });
  QMouseEvent press(QEvent::MouseButtonPress,QPointF(400,300),QPointF(400,300),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
  QMouseEvent release(QEvent::MouseButtonRelease,QPointF(400,300),QPointF(400,300),Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
  interaction.handle(&press);
  interaction.handle(&release);
  check(clicks == 1,"Left click did not reach result picking");
  interaction.handle(&press);
  QMouseEvent move(QEvent::MouseMove,QPointF(500,300),QPointF(500,300),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
  interaction.handle(&move);
  interaction.handle(&release);
  check(clicks == 1,"Fast drag returning to its origin was misidentified as a click");
  check(app->pick_entity("Model",Selector{std::size_t{0}},EntityKind::Node,largeNode,std::nullopt,error) &&
    app->close_source("Model",error) && !app->session().views.at(view).selection.entity,"Source closure retained selection");
  PostprocessorCore core;
  SimResults::JobSourceReference request{"test","",1,""};
  check(core.open_result(registry.provider,request,std::nullopt,error),"Cannot open selection refresh fixture");
  const auto coreSource = core.session().active_source_id;
  check(core.pick_entity("view1",EntityReference{coreSource,0,EntityKind::Node,largeNode,{}},error),"Cannot select refresh fixture node");
  check(core.refresh_result(registry.provider,coreSource,error) && !core.session().views.at("view1").selection.entity,"Refresh retained a stale selection");
  check(core.reset(error) && core.session().views.at("view1").selection.mode == SelectionDefaults::mode,"Reset did not restore pick defaults");
  // No Render(): GL rendering and native command parsing await an authorized build.
  return failures == 0 ? 0 : 1;
}
