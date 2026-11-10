#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorScene.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorTriad.hpp"
#include <vtkRendererCollection.h>
#include <vtkCamera.h>
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkCellType.h>
#include <vtkIdList.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkUnsignedCharArray.h>
#include <limits>
#include <vtkPointData.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkLookupTable.h>
#include <vtkOpenGLRenderer.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPoints.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkQuadraticHexahedron.h>
#include <vtkTable.h>
#include <vtkUnstructuredGrid.h>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <vtkPropCollection.h>

namespace
{
int failures = 0;
void check(bool condition, const char* message)
{
  if (!condition)
  {
    std::cerr << message << '\n';
    ++failures;
  }
}

std::shared_ptr<vtkDataObject> grid(double offset)
{
  auto* object = vtkUnstructuredGrid::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataTypeToDouble();
  points->InsertNextPoint(offset, 0, 0);
  points->InsertNextPoint(offset + 1, 0, 0);
  points->InsertNextPoint(offset, 1, 0);
  points->InsertNextPoint(offset, 0, 1);
  object->SetPoints(points);
  object->Allocate(1);
  auto values = vtkSmartPointer<vtkDoubleArray>::New();
  values->SetName("test_scalar");
  values->SetNumberOfTuples(4);
  values->FillComponent(0, 17.0);
  object->GetPointData()->SetScalars(values);
  vtkIdType ids[] = {0, 1, 2, 3};
  object->InsertNextCell(VTK_TETRA, 4, ids);
  return std::shared_ptr<vtkDataObject>(object, [](vtkDataObject* value)
  {
    value->Delete();
  });
}

std::shared_ptr<SimResults::VtkResultsData> result(bool collection, bool leadingEmpty = false)
{
  auto data = std::make_shared<SimResults::VtkResultsData>();
  data->job_id = 1;
  data->run_identity = "test-run";
  if (leadingEmpty)
  {
    data->increments.emplace_back();
  }
  SimResults::Increment frame;
  if (collection)
  {
    auto* object = vtkPartitionedDataSetCollection::New();
    auto parts = vtkSmartPointer<vtkPartitionedDataSet>::New();
    const auto first = grid(0);
    const auto second = grid(10);
    parts->SetPartition(0, first.get());
    parts->SetPartition(1, second.get());
    object->SetPartitionedDataSet(0, parts);
    object->SetPartitionedDataSet(1, vtkSmartPointer<vtkPartitionedDataSet>::New());
    frame.dataset = std::shared_ptr<vtkDataObject>(object, [](vtkDataObject* value)
    {
      value->Delete();
    });
  }
  else
  {
    frame.dataset = grid(0);
  }
  data->increments.push_back(frame);
  return data;
}

class Provider : public ResultSourceProviderInterface
{
public:
  std::shared_ptr<const SimResults::VtkResultsData> data;
  std::string name = "Mesh";
  bool fail = false;
  std::function<void()> reenter;
  std::string command_prefix() const override
  {
    return "fake";
  }
  bool open(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override
  {
    if (reenter)
    {
      reenter();
    }
    if (fail)
    {
      error = "Injected provider failure";
      return false;
    }
    auto reference = std::get<SimResults::JobSourceReference>(request);
    reference.run_identity = data->run_identity;
    output = {reference, name, data};
    error.clear();
    return true;
  }
  bool refresh(const SimResults::ResultSourceReference& request, SimResults::PublishedResultSource& output, std::string& error) override
  {
    return open(request, output, error);
  }
};

class Registry : public ResultSourceRegistryInterface
{
public:
  explicit Registry(Provider& value) : provider(value)
  {
  }
  Provider& provider;
  bool resolve_result_provider(const std::string& prefix, ResolvedResultSourceProvider& output, std::string& error) const override
  {
    if (prefix != "fake")
    {
      error = "Unknown test provider";
      return false;
    }
    output.application_id = "test";
    output.provider = &provider;
    error.clear();
    return true;
  }
};

void coloring_scene_checks()
{
  Provider provider;
  auto fixture = result(false);
  SimResults::Field field;
  field.name = "Stress";
  field.array_name = "test_scalar";
  field.components = {"SXX"};
  fixture->increments[0].fields.push_back(field);
  auto missing = fixture->increments[0];
  missing.fields.clear();
  fixture->increments.push_back(missing);
  provider.data = fixture;
  Registry registry(provider);
  PostprocessorApplicationInterface application(registry);
  std::string error;
  check(application.open_job("fake",1,std::nullopt,error),"Cannot open coloring fixture");
  check(!application.color_field("Mesh","Stress","",true,0,error),"Tensor/scalar magnitude was accepted");
  check(application.color_field("Mesh","Stress","SXX",false,0,error),"Cannot select stress component");
  auto settings = application.coloring("Mesh",error);
  if (!settings)
  {
    check(false,"Color settings missing");
    return;
  }
  settings->maximum = {true,10,{1,0,1}};
  check(application.set_coloring("Mesh",*settings,error),"Cannot configure stress limit");
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  PostprocessorScene scene(renderer,"view1");
  check(scene.synchronize(application,error),"Cannot synchronize scalar coloring");
  renderer->GetActors()->InitTraversal();
  auto* actor = renderer->GetActors()->GetNextActor();
  auto* mapper = actor ? vtkPolyDataMapper::SafeDownCast(actor->GetMapper()) : nullptr;
  check(mapper && mapper->GetScalarVisibility(),"Result mapper did not enable scalars");
  if (mapper)
  {
    auto* geometry = mapper->GetInput();
    const auto stamp = geometry->GetMTime();
    double rgb[3];
    mapper->GetLookupTable()->GetColor(17,rgb);
    check(rgb[0] > 0.99 && rgb[1] < 0.01 && rgb[2] > 0.99,"Maximum stress color missing");
    settings->maximum.enabled = false;
    settings->palette = Postprocessor::ColorPalette::Grayscale;
    check(application.set_coloring("Mesh",*settings,error) && scene.synchronize(application,error),"Cannot disable maximum limit");
    check(mapper->GetInput() == geometry && geometry->GetMTime() == stamp,"Palette update rebuilt scalar geometry");
    mapper->GetLookupTable()->GetColor(17,rgb);
    check(std::abs(rgb[0]-rgb[1]) < 0.01 && std::abs(rgb[1]-rgb[2]) < 0.01,"Disabled limit still overrode palette");
  }
  check(application.set_frame("Mesh",2,error) && scene.synchronize(application,error),"Cannot change to unavailable-field frame");
  renderer->GetActors()->InitTraversal();
  actor = renderer->GetActors()->GetNextActor();
  check(actor && !actor->GetMapper()->GetScalarVisibility(),"Missing field left stale scalar colors");
  const auto retained = application.coloring("Mesh",error);
  check(retained && retained->field.has_value(),"Frame change discarded requested field identity");
  auto* original = vtkDataSet::SafeDownCast(fixture->increments[0].dataset.get());
  check(original->GetPointData()->GetNumberOfArrays() == 1,"Color display changed immutable input arrays");
  check(application.reset(error) && scene.synchronize(application,error) && renderer->GetViewProps()->GetNumberOfItems() == 0,
    "Reset left color legends or actors attached");
}

std::shared_ptr<SimResults::VtkResultsData> ranged_result(double peak,bool invalid = false)
{
  auto data = std::make_shared<SimResults::VtkResultsData>();
  data->job_id = 1;
  data->run_identity = "range-test";
  data->layout = SimResults::DatasetLayout::Collection;
  for (int frame = 0; frame < 2; ++frame)
  {
    auto* collection = vtkPartitionedDataSetCollection::New();
    SimResults::Increment increment;
    for (unsigned int domain = 0; domain < 2; ++domain)
    {
      const auto object = grid(domain*10);
      auto* mesh = vtkDataSet::SafeDownCast(object.get());
      auto* values = mesh->GetPointData()->GetScalars();
      values->SetComponent(0,0,domain == 1 ? peak : (frame == 0 ? -10 : 20));
      values->SetComponent(1,0,domain == 1 ? 5 : (frame == 0 ? 4 : 40));
      values->SetComponent(2,0,9000); // Masked outlier must never set the range.
      values->SetComponent(3,0,std::numeric_limits<double>::quiet_NaN());
      auto mask = vtkSmartPointer<vtkUnsignedCharArray>::New();
      mask->SetName("valid");
      mask->InsertNextValue(invalid ? 0 : 1);
      mask->InsertNextValue(invalid ? 0 : 1);
      mask->InsertNextValue(0);
      mask->InsertNextValue(1);
      mesh->GetPointData()->AddArray(mask);
      auto parts = vtkSmartPointer<vtkPartitionedDataSet>::New();
      parts->SetPartition(0,object.get());
      collection->SetPartitionedDataSet(domain,parts);
      SimResults::Field field;
      field.name = "Stress";
      field.array_name = "test_scalar";
      field.validity_array = "valid";
      field.components = {"SXX"};
      field.dataset_id = domain;
      increment.fields.push_back(field);
    }
    increment.dataset = std::shared_ptr<vtkDataObject>(collection,[](vtkDataObject* object)
    {
      object->Delete();
    });
    data->increments.push_back(increment);
  }
  return data;
}

void all_frame_range_checks()
{
  Provider provider;
  provider.data = ranged_result(1000);
  Registry registry(provider);
  PostprocessorApplicationInterface application(registry);
  std::string error;
  check(application.open_job("fake",1,std::nullopt,error) &&
    application.color_field("Mesh","Stress","SXX",false,std::nullopt,error),"Cannot prepare grouped all-frame range fixture");
  const auto source = application.session().active_source_id;
  auto coloring = application.coloring("Mesh",error);
  if (!coloring || !coloring->field)
  {
    check(false,"Range fixture has no field selection");
    return;
  }
  const auto range = application.all_frame_range(source,*coloring->field,error);
  check(range && (*range)[0] == -10 && (*range)[1] == 1000 && error.empty(),
    "All-frame range did not combine valid samples across datasets and frames");
  auto scoped = *coloring->field;
  scoped.dataset_id = 0;
  const auto subset = application.all_frame_range(source,scoped,error);
  check(subset && (*subset)[0] == -10 && (*subset)[1] == 40,"Explicit dataset range scope was ignored");
  coloring->range_mode = Postprocessor::ColorRangeMode::AllFrames;
  check(application.set_coloring("Mesh",*coloring,error),"Cannot select all-frame range mode");
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  PostprocessorScene scene(renderer,"view1");
  for (std::size_t frame = 1; frame <= 2; ++frame)
  {
    check(application.set_frame("Mesh",frame,error) && scene.synchronize(application,error),"Cannot show all-frame coloring");
    renderer->GetActors()->InitTraversal();
    auto* actor = renderer->GetActors()->GetNextActor();
    auto* mapper = actor ? actor->GetMapper() : nullptr;
    check(mapper && mapper->GetLookupTable()->GetRange()[0] == -10 && mapper->GetLookupTable()->GetRange()[1] == 1000,
      "Color/legend range changed while navigating frames");
  }
  Postprocessor::PostprocessorCore core;
  SimResults::JobSourceReference request{"test","",1,""};
  check(core.open_result(provider,request,std::nullopt,error),"Cannot open range refresh fixture");
  const auto id = core.source_id_from_name("Mesh");
  const auto initial = core.all_frame_range(id,*coloring->field,error);
  provider.data = ranged_result(2000);
  check(core.refresh_result(provider,id,error),"Cannot refresh cached range source");
  const auto updated = core.all_frame_range(id,*coloring->field,error);
  check(initial && updated && (*initial)[1] == 1000 && (*updated)[1] == 2000,"Source refresh retained a stale global range");
  provider.data = ranged_result(2000,true);
  check(core.refresh_result(provider,id,error),"Cannot load an all-invalid range fixture");
  check(!core.all_frame_range(id,*coloring->field,error) && error.empty(),"All-invalid samples produced a fabricated numeric range");
}

void dataset_visibility_checks()
{
  using Action = Postprocessor::DatasetVisibilityAction;
  using Selector = PostprocessorApplicationInterface::DatasetSelector;
  Provider provider;
  auto fixture = ranged_result(1000);
  SimResults::Dataset ordinary;
  ordinary.id = 0;
  ordinary.name = "all"; // Quoted name must resolve a single dataset.
  SimResults::Dataset integration;
  integration.id = 1;
  integration.name = "Integration points — ELSET_A";
  fixture->datasets = {integration,ordinary}; // Metadata order is not the ID.
  auto* first = vtkPartitionedDataSetCollection::SafeDownCast(fixture->increments[0].dataset.get());
  auto* third = vtkPartitionedDataSetCollection::New();
  third->SetPartitionedDataSet(0,first->GetPartitionedDataSet(0));
  auto absentFrame = fixture->increments[0];
  absentFrame.dataset = std::shared_ptr<vtkDataObject>(third,[](vtkDataObject* value)
  {
    value->Delete();
  });
  absentFrame.fields.erase(std::remove_if(absentFrame.fields.begin(),absentFrame.fields.end(),[](const SimResults::Field& field)
  {
    return field.dataset_id == 1;
  }),absentFrame.fields.end());
  fixture->increments.push_back(absentFrame);
  provider.data = fixture;
  Registry registry(provider);
  PostprocessorApplicationInterface application(registry);
  std::string error;
  check(application.open_job("fake",1,std::nullopt,error),"Cannot open dataset visibility fixture");
  const auto source = application.session().active_source_id;
  const auto catalogue = application.mesh_datasets(source);
  check(catalogue.size() == 2 && catalogue[0].id == 0 && catalogue[0].name == "all" &&
    catalogue[1].id == 1 && catalogue[1].name == integration.name,"Dataset catalogue used metadata vector positions");
  check(application.color_field("Mesh","Stress","SXX",false,std::nullopt,error),"Cannot color visibility fixture");
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  PostprocessorScene scene(renderer,"view1");
  check(scene.synchronize(application,error),"Cannot show visibility fixture");
  renderer->GetActors()->InitTraversal();
  vtkSmartPointer<vtkActor> firstActor = renderer->GetActors()->GetNextActor();
  auto* scalarGeometry = firstActor ? firstActor->GetMapper()->GetInput() : nullptr;
  check(application.set_dataset_visibility("",Action::Hide,Selector{integration.name},error),"Named dataset hide failed");
  const auto& display = application.session().representations.begin()->second;
  check(display.hidden_datasets == std::set<std::size_t>{1} && display.visible,"Named hide changed master visibility or wrong dataset");
  check(scene.synchronize(application,error),"Cannot synchronize hidden dataset");
  renderer->GetActors()->InitTraversal();
  auto* retained = renderer->GetActors()->GetNextActor();
  check(retained == firstActor && retained->GetMapper()->GetInput() == scalarGeometry,"Visibility rebuilt actors or scalar geometry");
  check(retained->GetMapper()->GetLookupTable()->GetRange()[1] == 4,"Current-frame range included hidden samples");
  std::array<double,6> bounds;
  check(application.visible_bounds("view1",bounds,error) && bounds[0] == 0 && bounds[1] == 1,"Fit included hidden dataset");
  auto coloring = application.coloring("Mesh",error);
  coloring->range_mode = Postprocessor::ColorRangeMode::AllFrames;
  check(application.set_coloring("Mesh",*coloring,error) && scene.synchronize(application,error),"Cannot set visible all-frame range");
  check(firstActor->GetMapper()->GetLookupTable()->GetRange()[1] == 40,"All-frame range included hidden samples");
  check(application.set_frame("Mesh",2,error) && scene.synchronize(application,error),"Cannot animate hidden dataset selection");
  renderer->GetActors()->InitTraversal();
  check(renderer->GetActors()->GetNextActor()->GetMapper()->GetLookupTable()->GetRange()[1] == 40,
    "Visible all-frame range changed during playback");
  check(application.set_dataset_visibility("Mesh",Action::ShowOnly,Selector{std::size_t{1}},error),"Dataset isolation failed");
  check(application.visible_bounds("view1",bounds,error) && bounds[0] == 10 && bounds[1] == 11,"Isolation fit used the wrong dataset");
  check(application.set_frame("Mesh",3,error) && !application.visible_bounds("view1",bounds,error),"Absent isolated dataset fabricated geometry");
  check(application.session().representations.begin()->second.hidden_datasets == std::set<std::size_t>{0},"Frame navigation reset dataset preferences");
  const auto revision = application.display_revision();
  check(!application.set_dataset_visibility("Mesh",Action::Hide,Selector{std::size_t{99}},error) &&
    !application.set_dataset_visibility("Mesh",Action::Hide,Selector{std::string("missing")},error) &&
    application.display_revision() == revision,"Invalid dataset selection mutated state");
  check(application.set_dataset_visibility("Mesh",Action::ShowAll,std::nullopt,error),"Show all datasets failed");
  check(application.set_dataset_visibility("Mesh",Action::Hide,Selector{std::string("all")},error) &&
    application.session().representations.begin()->second.hidden_datasets == std::set<std::size_t>{0},
    "Quoted all name was treated as Show all");
  check(application.set_source_visible("Mesh",false,error) && application.set_source_visible("Mesh",true,error) &&
    application.session().representations.begin()->second.hidden_datasets.count(0) == 1,"Master switch reset child preferences");
  check(application.set_dataset_visibility("Mesh",Action::Show,Selector{std::string("all")},error),"Named show failed");
  check(application.result_data(source) == fixture,"Visibility replaced result data");
  auto fixed = application.coloring("Mesh",error);
  fixed->range_mode = Postprocessor::ColorRangeMode::Fixed;
  fixed->fixed_range = {-5,500};
  check(application.set_coloring("Mesh",*fixed,error) && application.set_frame("Mesh",1,error) &&
    application.set_dataset_visibility("Mesh",Action::Hide,Selector{std::size_t{1}},error) && scene.synchronize(application,error),
    "Cannot test fixed range with hidden datasets");
  renderer->GetActors()->InitTraversal();
  check(renderer->GetActors()->GetNextActor()->GetMapper()->GetLookupTable()->GetRange()[1] == 500,"Visibility changed fixed palette bounds");
  check(application.set_dataset_visibility("Mesh",Action::Hide,Selector{std::size_t{0}},error) &&
    scene.synchronize(application,error) && !application.visible_bounds("view1",bounds,error),"Hiding all datasets left fit geometry");
  renderer->GetActors()->InitTraversal();
  while (auto* actor = renderer->GetActors()->GetNextActor())
  {
    check(!actor->GetVisibility(),"Hiding all datasets left a visible actor");
  }
  Postprocessor::PostprocessorCore core;
  SimResults::JobSourceReference request{"test","",1,""};
  check(core.open_result(provider,request,std::nullopt,error),"Cannot open refresh visibility fixture");
  const auto id = core.session().active_source_id;
  check(core.set_dataset_visibility(id,"view1",Action::Hide,0,error),"Cannot hide before refresh");
  check(core.refresh_result(provider,id,error) && core.session().representations.begin()->second.hidden_datasets.empty(),
    "Source refresh retained old dataset IDs");
  auto ambiguous = ranged_result(1000);
  ordinary.name = "Duplicate";
  integration.name = "Duplicate";
  ambiguous->datasets = {ordinary,integration};
  Provider other;
  other.data = ambiguous;
  Registry otherRegistry(other);
  PostprocessorApplicationInterface otherApp(otherRegistry);
  check(otherApp.open_job("fake",1,std::nullopt,error),"Cannot open duplicate name fixture");
  const auto beforeAmbiguous = otherApp.display_revision();
  check(!otherApp.set_dataset_visibility("Mesh",Action::ShowOnly,Selector{std::string("Duplicate")},error) &&
    error.find("ambiguous") != std::string::npos && otherApp.display_revision() == beforeAmbiguous,
    "Duplicate name silently chose a dataset");
  check(otherApp.set_dataset_visibility("Mesh",Action::ShowOnly,Selector{std::size_t{1}},error),"Numeric ID could not disambiguate duplicate names");
  Provider unnamed;
  unnamed.data = result(false);
  Registry unnamedRegistry(unnamed);
  PostprocessorApplicationInterface unnamedApp(unnamedRegistry);
  check(unnamedApp.open_job("fake",1,std::nullopt,error) &&
    unnamedApp.mesh_datasets(unnamedApp.session().active_source_id).size() == 1 &&
    unnamedApp.set_dataset_visibility("Mesh",Action::Hide,Selector{std::size_t{0}},error),"Independent grid without metadata is not selectable by ID");
}

void animation_checks()
{
  Provider provider;
  auto data = result(false);
  data->increments.emplace_back();
  data->increments.push_back(data->increments.front());
  provider.data = data;
  Registry registry(provider);
  PostprocessorApplicationInterface application(registry);
  std::string error;
  check(application.open_job("fake",1,std::nullopt,error),"Cannot open animation fixture");
  auto settings = application.animation("Mesh",error);
  check(settings && settings->frames_per_second == 10 && !settings->loop && !settings->playing,"Animation defaults differ from the agreed values");
  if (!settings)
  {
    return;
  }
  check(application.set_frame("Mesh",3,error),"Cannot prepare last animation frame");
  settings->playing = true;
  check(application.set_animation("Mesh",*settings,error) && application.session().representations.begin()->second.increment_index == 0,
    "Play at the final frame must restart at the first displayable frame");
  const auto revision = application.display_revision();
  settings->frames_per_second = 0;
  check(!application.set_animation("Mesh",*settings,error) && application.display_revision() == revision,"Invalid FPS changed animation state");
  settings->frames_per_second = 5;
  settings->loop = true;
  check(application.set_animation("Mesh",*settings,error),"Cannot change live FPS/loop settings");
  settings = application.animation("Mesh",error);
  check(settings && settings->loop && settings->playing && settings->frames_per_second == 5,"Live animation settings were not retained");
  check(application.open_job("fake",1,std::string("Second"),error),"Cannot open a second animation source");
  const auto previous = application.animation("Mesh",error);
  const auto second = application.animation("Second",error);
  check(previous && !previous->playing && second && !second->loop && second->frames_per_second == 10,
    "Source changes failed to stop playback or leaked settings into another representation");
  check(application.reset(error) && !application.animation("",error),"Reset retained animation state");
}

void frame_checks()
{
  Provider provider;
  provider.data = result(false);
  auto second = provider.data->increments[0];
  second.dataset = grid(10);
  second.step = 2;
  second.increment = 1;
  second.time = 0.3;
  auto fixture = std::make_shared<SimResults::VtkResultsData>(*provider.data);
  fixture->increments.emplace_back(); // An unavailable frame retains its ordinal.
  fixture->increments.push_back(second);
  provider.data = fixture;
  Registry registry(provider);
  PostprocessorApplicationInterface application(registry);
  std::string error;
  check(application.open_job("fake", 1, std::nullopt, error), "Cannot open frame fixture");
  const auto source = application.session().active_source_id;
  Postprocessor::CameraDefinition camera;
  camera.position = {5, 5, 5};
  check(application.set_camera(camera, error), "Cannot initialize frame camera");
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  PostprocessorScene scene(renderer, "view1");
  check(scene.synchronize(application, error), "Cannot create frame fixture scene");
  check(application.set_source_visible("Mesh", false, error), "Cannot hide frame fixture");
  const auto before = application.display_revision();
  check(!application.set_frame("Mesh", 0, error) && !application.set_frame("Mesh", 2, error) &&
    !application.set_frame("Mesh", 4, error) && !application.set_frame("missing", 1, error) &&
    application.display_revision() == before, "Invalid frame selection changed display state");
  check(application.set_frame("", 3, error), "Active-source frame selection failed");
  const auto& display = application.session().representations.begin()->second;
  check(display.increment_index == 2 && !display.visible && application.display_revision() == before + 1,
    "Frame selection lost its ordinal, changed visibility or failed to advance revision");
  check(application.session().views.at("view1").camera->position == camera.position,
    "Frame selection moved an established camera");
  const auto revision = application.display_revision();
  check(application.set_frame("Mesh", 3, error) && application.display_revision() == revision,
    "Repeated frame selection must be a no-op");
  check(application.set_source_visible("Mesh", true, error) && scene.synchronize(application, error),
    "Cannot show newly selected frame");
  double bounds[6];
  renderer->ComputeVisiblePropBounds(bounds);
  check(bounds[0] == 10 && bounds[1] == 11, "Scene retained the previous frame's mesh");
  check(application.result_data(source) == fixture, "Frame selection replaced or copied the published result");
  check(application.open_job("fake", 1, std::string("Second"), error), "Cannot open independent source instance");
  check(application.set_frame("Second", 1, error), "Cannot change the second source frame");
  for (const auto& entry : application.session().representations)
  {
    if (entry.second.input.object_id == source)
    {
      check(entry.second.increment_index == 2, "Changing another source changed this frame selection");
    }
  }
  check(application.reset(error) && !application.set_frame("", 1, error), "Reset retained active frame context");
}

void reset_checks()
{
  Provider provider;
  provider.data = result(false);
  Registry registry(provider);
  PostprocessorApplicationInterface application(registry);
  std::string error;
  check(application.open_job("fake", 1, std::nullopt, error), "Cannot open reset fixture");
  const auto source = application.session().active_source_id;
  const auto data = application.result_data(source);
  Postprocessor::CameraDefinition camera;
  camera.position = {4, 5, 6};
  check(application.set_camera(camera, error), "Cannot initialize reset fixture camera");
  check(application.request_camera_fit(error), "Cannot request pre-reset fit");
  const auto oldView = application.session().views.at("view1");
  const auto oldRevision = application.display_revision();
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  PostprocessorScene scene(renderer, "view1");
  check(scene.synchronize(application, error) && renderer->GetActors()->GetNumberOfItems() == 2,
    "Reset fixture actors missing");
  check(application.reset(error), "Application reset failed");
  const auto& state = application.session();
  check(state.sources.empty() && state.representations.empty() && state.active_source_id.empty() &&
    state.active_representation_id.empty(), "Reset retained source/display/selection definitions");
  check(state.views.size() == 1 && state.active_view_id == "view1" && !state.views.at("view1").camera,
    "Reset must restore one empty default view");
  check(application.result_data(source) == nullptr && application.display_revision() > oldRevision,
    "Reset retained runtime data or reused the display revision");
  check(state.views.at("view1").camera_revision > oldView.camera_revision &&
    state.views.at("view1").fit_request > oldView.fit_request,
    "Reset reused camera/fit tokens");
  check(!application.complete_camera_fit("view1", oldView.fit_request, oldRevision, camera, error),
    "Pre-reset camera fit survived reset");
  check(scene.synchronize(application, error) && renderer->GetActors()->GetNumberOfItems() == 0,
    "Reset retained scene actors");
  check(data != nullptr && !data->increments.empty(), "Reset invalidated externally owned published data");
  const auto resetRevision = application.display_revision();
  check(application.reset(error) && application.display_revision() > resetRevision, "Repeated reset failed");
  check(application.open_job("fake", 1, std::nullopt, error) && application.session().active_source_id != source,
    "Opening after reset failed or reused an old source identity");
  check(scene.synchronize(application, error) && renderer->GetActors()->GetNumberOfItems() == 2,
    "Opening after reset did not restore display actors");
}

void core_checks()
{
  Provider provider;
  provider.data = result(true, true);
  Postprocessor::PostprocessorCore core;
  std::string error;
  SimResults::JobSourceReference request{"test", "", 1, ""};
  provider.reenter = [&core]()
  {
    std::string nested;
    check(!core.set_source_visible("missing", "view1", true, nested), "Provider reentry changed display state");
    check(!core.reset(nested), "Provider reentry reset an in-flight session");
  };
  check(core.open_result(provider, request, std::nullopt, error), "Cannot open test collection");
  provider.reenter = {};
  const auto id = core.source_id_from_name("Mesh");
  const auto retained = core.result_data(id);
  check(core.set_source_visible(id, "view1", false, error), "Cannot hide source before reopening");
  check(core.open_result(provider, request, std::nullopt, error), "Cannot reopen retained snapshot");
  check(core.session().sources.size() == 1 && core.session().active_source_id == id && core.result_data(id) == retained,
    "Reopening duplicated or replaced a retained snapshot");
  check(!core.session().representations.empty() && core.session().representations.begin()->second.visible,
    "Reopening did not show the retained snapshot");
  check(!core.open_result(provider, request, std::string("Mesh"), error), "Explicit duplicate source name was accepted");
  check(core.source_has_mesh(id) && core.session().representations.size() == 1, "Initial mesh display missing");
  if (core.session().representations.size() != 1)
  {
    return;
  }
  check(core.session().representations.begin()->second.increment_index == 1, "Initial frame must skip empty increments");
  std::array<double, 6> bounds;
  check(core.visible_bounds("view1", bounds, error) && bounds[0] == 0 && bounds[1] == 11, "Partition bounds did not combine");
  const auto beforeHide = core.display_revision();
  Postprocessor::CameraDefinition camera;
  camera.position = {5, 5, 5};
  camera.view_up = {0, 0, 1};
  check(core.set_source_visible(id, "view1", false, error), "Hide failed");
  check(!core.complete_camera_fit("view1", 0, beforeHide, camera, error), "Stale fit completion was accepted");
  const auto emptyFitState = core.session().views.at("view1");
  error = "stale error";
  check(core.request_camera_fit("view1", error) && error.empty(), "Fit of an empty view must be a successful no-op");
  const auto& afterEmptyFit = core.session().views.at("view1");
  check(afterEmptyFit.fit_request == emptyFitState.fit_request &&
    afterEmptyFit.fit_pending == emptyFitState.fit_pending &&
    afterEmptyFit.camera_revision == emptyFitState.camera_revision,
    "Empty Fit changed camera revision or fit state");
  bounds = {1,2,3,4,5,6};
  check(!core.visible_bounds("view1",bounds,error) && error.empty() &&
    bounds == std::array<double,6>{1,2,3,4,5,6},"Empty bounds query changed output or returned an error");
  check(!core.request_camera_fit("missing-view",error) && !error.empty(),"Invalid Fit view stopped reporting errors");
  const auto hiddenRevision = core.display_revision();
  check(core.set_source_visible(id, "view1", false, error) && core.display_revision() == hiddenRevision, "Repeated hide must be idempotent");
  check(core.set_source_visible(id, "view1", true, error) && core.session().representations.size() == 1, "Show duplicated a display");
  check(core.request_camera_fit("view1", error), "Fit request failed");
  const auto superseded = core.session().views.at("view1").fit_request;
  check(core.request_camera_fit("view1", error), "Replacement fit request failed");
  check(!core.complete_camera_fit("view1", superseded, core.display_revision(), camera, error), "Superseded fit token was accepted");
  const auto token = core.session().views.at("view1").fit_request;
  auto invalid = camera;
  invalid.position = invalid.focal_point;
  check(!core.complete_camera_fit("view1", token, core.display_revision(), invalid, error), "Invalid camera was accepted");
  check(core.complete_camera_fit("view1", token, core.display_revision(), camera, error), "Valid camera completion failed");
  check(!core.session().views.at("view1").fit_pending, "Completed fit remains pending");
  provider.name = "Second";
  check(core.open_result(provider, request, std::string("Second"), error), "Second mesh opening failed");
  check(core.session().views.at("view1").camera->position == camera.position && !core.session().views.at("view1").fit_pending,
    "Opening another source reset the camera");
  const auto oldData = core.result_data(id);
  const auto revision = core.display_revision();
  provider.fail = true;
  check(!core.refresh_result(provider, id, error) && core.result_data(id) == oldData && core.display_revision() == revision,
    "Failed refresh changed data or display revision");
  provider.fail = false;
  provider.data = result(false);
  check(core.set_source_visible(id, "view1", false, error), "Hide before refresh failed");
  check(core.refresh_result(provider, id, error), "Refresh failed");
  for (const auto& entry : core.session().representations)
  {
    if (entry.second.input.object_id == id)
    {
      check(!entry.second.visible && entry.second.increment_index == 0, "Refresh lost visibility or retained invalid frame");
    }
  }
  check(core.close_source(id, error), "Close failed");
  for (const auto& entry : core.session().representations)
  {
    check(entry.second.input.object_id != id, "Closed source retained a representation");
  }
  auto table = std::make_shared<SimResults::VtkResultsData>();
  table->job_id = 2;
  table->run_identity = "table-run";
  table->tables.emplace_back(vtkTable::New(), [](vtkTable* value)
  {
    value->Delete();
  });
  provider.data = table;
  provider.name = "Table";
  request.job_id = 2;
  const auto displayCount = core.session().representations.size();
  check(core.open_result(provider, request, std::nullopt, error), "Table-only opening failed");
  const auto tableId = core.source_id_from_name("Table");
  check(!core.source_has_mesh(tableId) && core.session().representations.size() == displayCount, "Table-only source created a mesh display");
  check(!core.set_source_visible(tableId, "view1", true, error), "Showing table-only source must fail");
  check(core.close_all_sources(error) && core.session().representations.empty(), "Close all retained displays");
}

void scene_checks()
{
  Provider provider;
  Registry registry(provider);
  auto input = result(false);
  registry.provider.data = input;
  PostprocessorApplicationInterface application(registry);
  std::string error;
  check(application.open_job("fake", 1, std::nullopt, error), "Application opening failed");
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  auto window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  window->SetSize(800, 600);
  window->AddRenderer(renderer);
  PostprocessorScene scene(renderer, "view1");
  scene.apply_appearance({1, 1, 1}, {1, 1, 1});
  check(renderer->GetBackground()[0] == 1 && !renderer->GetGradientBackground(), "White/white background must be uniform.");
  scene.apply_appearance({0, 0, 0}, {1, 1, 1});
  check(renderer->GetGradientBackground() && renderer->GetBackground2()[0] == 0, "Different colors must enable the top/bottom gradient.");
  const auto rendererCount = window->GetRenderers()->GetNumberOfItems();
  {
    PostprocessorTriad triad(window);
    triad.update(renderer->GetActiveCamera(), true, {1, 1, 1}, 800, 600, 1);
    check(window->GetRenderers()->GetNumberOfItems() == rendererCount + 1 && renderer->GetActors()->GetNumberOfItems() == 0,
      "Triad must use a separate renderer without adding scene actors.");
    auto* collection = window->GetRenderers();
    collection->InitTraversal();
    vtkRenderer* overlay = nullptr;
    while (auto* next = collection->GetNextItem())
    {
      if (next != renderer.GetPointer())
      {
        overlay = next;
      }
    }
    check(overlay != nullptr, "Triad overlay must be discoverable.");
    if (overlay != nullptr)
    {
      overlay->GetActors()->InitTraversal();
      auto* arrows = overlay->GetActors()->GetNextActor();
      auto* data = arrows ? arrows->GetMapper()->GetInput() : nullptr;
      check(data != nullptr, "Triad geometry must be available without rendering.");
      if (data != nullptr)
      {
        const auto stamp = data->GetMTime();
        triad.update(renderer->GetActiveCamera(), true, {1, 1, 1}, 800, 600, 1);
        check(data->GetMTime() == stamp, "Unchanged triad orientation must not rebuild its geometry.");
        renderer->GetActiveCamera()->Azimuth(10);
        triad.update(renderer->GetActiveCamera(), true, {1, 1, 1}, 800, 600, 1);
        check(arrows->GetMapper()->GetInput() == data && data->GetMTime() > stamp,
          "Rotated triad must update its existing dataset instead of replacing buffers.");
      }
    }
    triad.update(renderer->GetActiveCamera(), false, {1, 1, 1}, 800, 600, 1);
  }
  check(window->GetRenderers()->GetNumberOfItems() == rendererCount, "Triad destruction must detach its overlay.");
  check(scene.synchronize(application, error) && renderer->GetActors()->GetNumberOfItems() == 2, "Single-grid surface and original edges missing");
  renderer->GetActors()->InitTraversal();
  auto* actor = renderer->GetActors()->GetNextActor();
  check(actor != nullptr && actor->GetMapper()->GetScalarVisibility() == 0 && !actor->GetProperty()->GetEdgeVisibility(),
    "Surface must not display tessellation edges");
  if (actor == nullptr)
  {
    return;
  }
  auto* raw = vtkDataSet::SafeDownCast(input->increments[0].dataset.get());
  check(raw->GetNumberOfPoints() == 4 && raw->GetNumberOfCells() == 1, "Display modified input topology");
  check(raw->GetPointData()->GetNumberOfArrays() == 1 && raw->GetPointData()->GetScalars()->GetComponent(0, 0) == 17.0,
    "Display changed published field arrays");
  check(application.set_source_visible("Mesh", false, error) && scene.synchronize(application, error), "Scene hide failed");
  check(actor->GetVisibility() == 0 && renderer->GetActors()->GetNumberOfItems() == 2, "Hide must retain surface and edge actors");
  check(application.set_source_visible("Mesh", true, error) && scene.synchronize(application, error), "Scene show failed");
  check(actor->GetVisibility() == 1, "Show did not restore actor visibility");
  std::array<double, 6> bounds;
  Postprocessor::CameraDefinition camera;
  check(application.visible_bounds("view1", bounds, error) && scene.prepare_camera_fit(bounds, camera, error), "Camera preparation failed");
  check(application.complete_camera_fit("view1", 0, application.display_revision(), camera, error), "Camera completion failed");
  check(application.close_all_sources(error) && scene.synchronize(application, error) && renderer->GetActors()->GetNumberOfItems() == 0,
    "Closing sources left actors behind");
  registry.provider.data = result(true);
  check(application.open_job("fake", 1, std::nullopt, error) && scene.synchronize(application, error) &&
    renderer->GetActors()->GetNumberOfItems() == 4, "Collection must display surfaces and edges for every partition");
  check(application.close_all_sources(error) && scene.synchronize(application, error), "Cannot clear partition display");
  auto quadratic = vtkSmartPointer<vtkQuadraticHexahedron>::New();
  auto quadraticGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  auto quadraticPoints = vtkSmartPointer<vtkPoints>::New();
  const auto* coordinates = quadratic->GetParametricCoords();
  for (vtkIdType i = 0; i < 20; ++i)
  {
    // Bow one midside node: edge display must retain it, not replace the
    // quadratic element with a corner-only linear hex.
    quadraticPoints->InsertNextPoint(coordinates[3 * i], coordinates[3 * i + 1],
      coordinates[3 * i + 2] + (i == 8 ? -0.2 : 0.0));
    quadratic->GetPointIds()->SetId(i, i);
  }
  quadraticGrid->SetPoints(quadraticPoints);
  quadraticGrid->Allocate(1);
  quadraticGrid->InsertNextCell(VTK_QUADRATIC_HEXAHEDRON, quadratic->GetPointIds());
  auto quadraticResult = result(false);
  quadraticGrid->Register(nullptr);
  quadraticResult->increments[0].dataset = std::shared_ptr<vtkDataObject>(quadraticGrid.GetPointer(), [](vtkDataObject* value)
  {
    value->Delete();
  });
  registry.provider.data = quadraticResult;
  check(application.open_job("fake", 1, std::nullopt, error) && scene.synchronize(application, error), "Cannot display quadratic hex");
  check(renderer->GetActors()->GetNumberOfItems() == 2, "Quadratic hex needs separate surface and original edge actors");
  renderer->GetActors()->InitTraversal();
  auto* quadraticSurface = renderer->GetActors()->GetNextActor();
  auto* quadraticEdges = renderer->GetActors()->GetNextActor();
  check(quadraticSurface != nullptr && !quadraticSurface->GetProperty()->GetEdgeVisibility(), "Quadratic surface exposes tessellation diagonals");
  auto* edgeData = quadraticEdges ? vtkPolyData::SafeDownCast(quadraticEdges->GetMapper()->GetInput()) : nullptr;
  check(edgeData != nullptr && edgeData->GetNumberOfLines() == 24 && edgeData->GetNumberOfPolys() == 0,
    "HEX20 must display only its 12 element edges split at midside nodes");
  if (edgeData != nullptr && edgeData->GetNumberOfPoints() > 8)
  {
    double midpoint[3];
    edgeData->GetPoint(8, midpoint);
    check(midpoint[2] < -0.19, "Original midside curvature was lost");
  }
  check(quadraticGrid->GetCellType(0) == VTK_QUADRATIC_HEXAHEDRON && quadraticGrid->GetNumberOfPoints() == 20,
    "Display changed original HEX20 topology");
  // Deliberately no Render(): parser, actual GL drawing and Qt lifecycle need
  // separate native integration checks after build authorization.
}
}

int main()
{
  check(Postprocessor::SelectionState{}.mode == Postprocessor::EntityKind::Node,"New views must default to node picking");
  check(Postprocessor::ResultColoring{}.range_mode == Postprocessor::ColorRangeMode::AllFrames,"New displays must default to All frames range");
  core_checks();
  reset_checks();
  frame_checks();
  animation_checks();
  all_frame_range_checks();
  dataset_visibility_checks();
  coloring_scene_checks();
  scene_checks();
  return failures == 0 ? 0 : 1;
}
