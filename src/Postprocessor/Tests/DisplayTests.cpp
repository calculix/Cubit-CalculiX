#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorScene.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkCellType.h>
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
  };
  check(core.open_result(provider, request, std::nullopt, error), "Cannot open test collection");
  provider.reenter = {};
  const auto id = core.source_id_from_name("Mesh");
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
  check(core.open_result(provider, request, std::nullopt, error), "Second mesh opening failed");
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
  check(scene.synchronize(application, error) && renderer->GetActors()->GetNumberOfItems() == 1, "Single-grid actor missing");
  renderer->GetActors()->InitTraversal();
  auto* actor = renderer->GetActors()->GetNextActor();
  check(actor != nullptr && actor->GetMapper()->GetScalarVisibility() == 0 && actor->GetProperty()->GetEdgeVisibility(),
    "Initial display must use a neutral surface with edges");
  if (actor == nullptr)
  {
    return;
  }
  auto* raw = vtkDataSet::SafeDownCast(input->increments[0].dataset.get());
  check(raw->GetNumberOfPoints() == 4 && raw->GetNumberOfCells() == 1, "Display modified input topology");
  check(raw->GetPointData()->GetNumberOfArrays() == 1 && raw->GetPointData()->GetScalars()->GetComponent(0, 0) == 17.0,
    "Display changed published field arrays");
  check(application.set_source_visible("Mesh", false, error) && scene.synchronize(application, error), "Scene hide failed");
  check(actor->GetVisibility() == 0 && renderer->GetActors()->GetNumberOfItems() == 1, "Hide must retain actors but remove visibility");
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
    renderer->GetActors()->GetNumberOfItems() == 2, "Collection must display every nonempty mesh partition");
  // Deliberately no Render(): parser, actual GL drawing and Qt lifecycle need
  // separate native integration checks after build authorization.
}
}

int main()
{
  core_checks();
  scene_checks();
  return failures == 0 ? 0 : 1;
}
