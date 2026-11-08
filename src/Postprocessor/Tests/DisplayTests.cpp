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
#include <vtkPointData.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkMapper.h>
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
#include <iostream>

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
  check(!core.request_camera_fit("view1", error), "Fit of an empty view must fail");
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
  core_checks();
  reset_checks();
  scene_checks();
  return failures == 0 ? 0 : 1;
}
