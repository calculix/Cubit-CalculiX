#include "PostprocessorScene.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/representations/DisplayDefaults.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDataSet.h>
#include <vtkOpenGLActor.h>
#include <vtkOpenGLCamera.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLProperty.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <algorithm>
#include <cmath>
#include <exception>

namespace
{
  bool valid_bounds(const std::array<double, 6>& bounds)
  {
    return std::all_of(bounds.begin(), bounds.end(), [](double value)
    {
      return std::isfinite(value);
    }) && bounds[0] <= bounds[1] && bounds[2] <= bounds[3] && bounds[4] <= bounds[5];
  }
}

PostprocessorScene::PostprocessorScene(vtkRenderer* renderer, const std::string& view_id)
  : renderer(renderer), viewId(view_id)
{
}

PostprocessorScene::~PostprocessorScene()
{
  clear();
}

void PostprocessorScene::clear()
{
  // The renderer belongs exclusively to this scene, including partial updates.
  renderer->RemoveAllViewProps();
  displays.clear();
}

std::shared_ptr<PostprocessorScene::RepresentationActors> PostprocessorScene::build(
  const std::shared_ptr<const SimResults::VtkResultsData>& data, std::size_t increment, std::string& error)
{
  if (!data || increment >= data->increments.size())
  {
    error = "The display input increment is unavailable.";
    return {};
  }
  auto group = std::make_shared<RepresentationActors>();
  group->data = data;
  group->increment = increment;
  for (auto* mesh : Postprocessor::result_meshes(data->increments[increment].dataset.get()))
  {
    auto surface = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    surface->SetInputData(mesh);
    bool failed = false;
    auto observer = vtkSmartPointer<vtkCallbackCommand>::New();
    observer->SetClientData(&failed);
    observer->SetCallback([](vtkObject*, unsigned long, void* client, void*)
    {
      *static_cast<bool*>(client) = true;
    });
    const auto tag = surface->AddObserver(vtkCommand::ErrorEvent, observer);
    surface->Update();
    surface->RemoveObserver(tag);
    if (failed || surface->GetOutput() == nullptr || surface->GetOutput()->GetNumberOfCells() == 0)
    {
      error = "VTK could not extract the mesh surface.";
      return {};
    }
    auto mapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
    mapper->SetInputConnection(surface->GetOutputPort());
    mapper->ScalarVisibilityOff();
    auto actor = vtkSmartPointer<vtkOpenGLActor>::New();
    actor->SetMapper(mapper);
    auto property = vtkSmartPointer<vtkOpenGLProperty>::New();
    const auto& color = Postprocessor::DisplayDefaults::surface_color;
    property->SetColor(color[0], color[1], color[2]);
    property->SetEdgeColor(Postprocessor::DisplayDefaults::edge_color.data());
    property->SetLineWidth(Postprocessor::DisplayDefaults::edge_width);
    property->SetRepresentationToSurface();
    property->EdgeVisibilityOn();
    actor->SetProperty(property);
    group->actors.push_back(actor);
  }
  if (group->actors.empty())
  {
    error = "The display has no supported nonempty mesh partitions.";
    return {};
  }
  return group;
}

bool PostprocessorScene::synchronize(const PostprocessorApplicationInterface& application, std::string& error)
{
  error.clear();
  try
  {
    const auto revision = application.display_revision();
    const auto state = application.session();
    const auto view = state.views.find(viewId);
    if (view == state.views.end())
    {
      clear();
      error = "The viewport's view definition is unavailable.";
      return false;
    }
    std::map<std::string, std::shared_ptr<RepresentationActors>> next;
    for (const auto& entry : state.representations)
    {
      const auto& display = entry.second;
      if (display.view_id != viewId || display.input.owner != Postprocessor::OutputOwner::Source || display.input.port != 0)
      {
        continue;
      }
      const auto data = application.result_data(display.input.object_id);
      const auto existing = displays.find(entry.first);
      std::shared_ptr<RepresentationActors> group;
      if (existing != displays.end() && existing->second->data == data && existing->second->increment == display.increment_index)
      {
        group = existing->second;
      }
      else
      {
        if (!display.visible)
        {
          // Retain an unchanged hidden group above, but build new geometry only
          // when it is visible. Changed hidden inputs must not retain old actors.
          continue;
        }
        std::string buildError;
        group = build(data, display.increment_index, buildError);
        if (!group)
        {
          error += (error.empty() ? "" : "\n") + display.name + ": " + buildError;
          continue;
        }
      }
      for (const auto& actor : group->actors)
      {
        actor->SetVisibility(display.visible);
      }
      next.emplace(entry.first, group);
    }
    // Remove closed/replaced/failed inputs even when another display failed.
    // Never leave actors from an old source snapshot masquerading as current.
    for (const auto& entry : displays)
    {
      const auto replacement = next.find(entry.first);
      if (replacement == next.end() || replacement->second != entry.second)
      {
        for (const auto& actor : entry.second->actors)
        {
          renderer->RemoveActor(actor);
        }
      }
    }
    for (const auto& entry : next)
    {
      const auto previous = displays.find(entry.first);
      if (previous == displays.end() || previous->second != entry.second)
      {
        for (const auto& actor : entry.second->actors)
        {
          renderer->AddActor(actor);
        }
      }
    }
    displays.swap(next);
    if (application.display_revision() != revision)
    {
      clear();
      error = "The display inputs changed during scene synchronization.";
      return false;
    }
    if (view->second.camera)
    {
      apply_camera(*view->second.camera);
    }
    else
    {
      Postprocessor::CameraDefinition initial;
      initial.position = Postprocessor::DisplayDefaults::initial_position;
      initial.view_up = Postprocessor::DisplayDefaults::initial_view_up;
      apply_camera(initial);
    }
    return error.empty();
  }
  catch (const std::exception& exception)
  {
    clear();
    error = std::string("Cannot synchronize the Postprocessor scene: ") + exception.what();
    return false;
  }
  catch (...)
  {
    clear();
    error = "Cannot synchronize the Postprocessor scene: unknown failure.";
    return false;
  }
}

void PostprocessorScene::apply_camera(const Postprocessor::CameraDefinition& definition)
{
  auto* camera = renderer->GetActiveCamera();
  camera->SetPosition(definition.position.data());
  camera->SetFocalPoint(definition.focal_point.data());
  camera->SetViewUp(definition.view_up.data());
  camera->SetParallelProjection(definition.parallel_projection);
  camera->SetViewAngle(definition.vertical_view_angle_degrees);
  camera->SetParallelScale(definition.parallel_scale);
  std::array<double, 6> bounds;
  renderer->ComputeVisiblePropBounds(bounds.data());
  if (valid_bounds(bounds))
  {
    renderer->ResetCameraClippingRange(bounds.data());
  }
}

bool PostprocessorScene::prepare_camera_fit(const std::array<double, 6>& bounds,
  Postprocessor::CameraDefinition& definition, std::string& error)
{
  error.clear();
  if (!valid_bounds(bounds))
  {
    error = "Cannot fit invalid mesh bounds.";
    return false;
  }
  struct RestoreCamera
  {
    vtkRenderer* renderer;
    vtkSmartPointer<vtkCamera> camera;
    ~RestoreCamera()
    {
      renderer->SetActiveCamera(camera);
    }
  } restore{renderer, renderer->GetActiveCamera()};
  auto fitted = vtkSmartPointer<vtkOpenGLCamera>::New();
  fitted->DeepCopy(restore.camera);
  renderer->SetActiveCamera(fitted);
  renderer->ResetCamera(bounds.data());
  renderer->ResetCameraClippingRange(bounds.data());
  fitted->GetPosition(definition.position.data());
  fitted->GetFocalPoint(definition.focal_point.data());
  fitted->GetViewUp(definition.view_up.data());
  definition.parallel_projection = fitted->GetParallelProjection() != 0;
  definition.vertical_view_angle_degrees = fitted->GetViewAngle();
  definition.parallel_scale = fitted->GetParallelScale();
  return true;
}
