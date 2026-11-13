#include "Postprocessor/Core/representations/ResultReferenceMesh.hpp"
#include "PostprocessorScene.hpp"
#include "Postprocessor/GUI/Selection/PostprocessorPicking.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/representations/DisplayDefaults.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "Postprocessor/Core/views/CameraOperations.hpp"
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPointSet.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkDataSet.h>
#include <vtkExtractEdges.h>
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
  geometryRevision.reset();
  visibleBounds.reset();
  appliedCamera.reset();
  displayWarning.clear();
  highlight = nullptr;
  hoverHighlight = nullptr;
  hoveredEntity.reset();
  highlightedSelection.reset();
  highlightedGeometry.reset();
}

const std::string& PostprocessorScene::warning() const noexcept
{
  return displayWarning;
}

std::shared_ptr<PostprocessorScene::RepresentationActors> PostprocessorScene::build(
  const std::shared_ptr<const SimResults::VtkResultsData>& data,
  const Postprocessor::Representation& display, std::string& error)
{
  const auto increment = display.increment_index;
  if (!data || increment >= data->increments.size())
  {
    error = "The display input increment is unavailable.";
    return {};
  }
  auto group = std::make_shared<RepresentationActors>();
  group->data = data;
  group->source_id = display.input.object_id;
  group->increment = increment;
  group->deformation = display.deformation;
  group->coloring = std::make_unique<PostprocessorColorMap>(renderer);
  for (const auto& part : Postprocessor::result_mesh_parts(data->increments[increment].dataset.get()))
  {
    std::shared_ptr<vtkDataSet> transformed;
    std::string warning;
    if (!Postprocessor::deform_mesh(part.second,part.first,data->increments[increment],
        display.deformation,transformed,warning,error))
    {
      return {};
    }
    if (!warning.empty())
    {
      auto& message = group->warnings[part.first];
      message += (message.empty() ? "" : "\n")+warning;
    }
    auto* mesh = transformed ? transformed.get() : part.second;
    if (transformed)
    {
      group->transformed.push_back(transformed);
    }
    auto surface = vtkSmartPointer<vtkDataSetSurfaceFilter>::New();
    Postprocessor::configure_pick_provenance(surface);
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
    property->SetPointSize(Postprocessor::DisplayDefaults::point_size_pixels);
    property->SetRenderPointsAsSpheres(Postprocessor::DisplayDefaults::rounded_points);
    // Surface tessellation is needed for nonlinear faces, but its diagonals
    // are not finite-element edges. Draw original cell edges separately.
    property->EdgeVisibilityOff();
    actor->SetProperty(property);
    group->actors.push_back({part.first,actor});
    group->surfaces.push_back({part.first, mesh, actor, surface->GetOutput(), nullptr});
    // Keep original points, including higher-order midside nodes, separate from
    // the tessellated surface. Tuple order and integral solver IDs stay intact.
    if (mesh->GetPointData()->HasArray("OriginalNodeId") && !mesh->GetPointData()->HasArray("IntegrationPointId"))
    {
      auto nodes = vtkSmartPointer<vtkPolyData>::New();
      if (auto* pointSet = vtkPointSet::SafeDownCast(mesh))
      {
        nodes->SetPoints(pointSet->GetPoints());
      }
      else
      {
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetDataTypeToDouble();
        points->SetNumberOfPoints(mesh->GetNumberOfPoints());
        for (vtkIdType i = 0; i < mesh->GetNumberOfPoints(); ++i)
        {
          double position[3];
          mesh->GetPoint(i,position);
          points->SetPoint(i,position);
        }
        nodes->SetPoints(points);
      }
      auto vertices = vtkSmartPointer<vtkCellArray>::New();
      for (vtkIdType i = 0; i < mesh->GetNumberOfPoints(); ++i)
      {
        vertices->InsertNextCell(1,&i);
      }
      nodes->SetVerts(vertices);
      nodes->GetPointData()->ShallowCopy(mesh->GetPointData());
      auto nodeMapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
      nodeMapper->SetInputData(nodes);
      nodeMapper->ScalarVisibilityOff();
      nodeMapper->SetRelativeCoincidentTopologyPointOffsetParameter(-1);
      auto nodeActor = vtkSmartPointer<vtkOpenGLActor>::New();
      nodeActor->SetMapper(nodeMapper);
      auto nodeProperty = vtkSmartPointer<vtkOpenGLProperty>::New();
      nodeProperty->SetColor(color[0],color[1],color[2]);
      nodeProperty->SetPointSize(Postprocessor::DisplayDefaults::point_size_pixels);
      nodeProperty->SetRenderPointsAsSpheres(Postprocessor::DisplayDefaults::rounded_points);
      nodeActor->SetProperty(nodeProperty);
      group->actors.push_back({part.first,nodeActor});
      group->surfaces.back().nodes = nodeActor;
      group->surfaces.back().node_data = nodes;
    }

    std::vector<vtkDataSet*> edgeMeshes = {mesh};
    std::vector<std::shared_ptr<vtkDataSet>> referenceMeshes;
    if (display.deformation.enabled && display.deformation.overlay)
    {
      if (part.second->GetPointData()->HasArray("IntegrationPointId"))
      {
        std::string referenceWarning;
        if (!Postprocessor::integration_point_reference_meshes(data->increments[increment],part.second,
            referenceMeshes,referenceWarning))
        {
          error = referenceWarning;
          return {};
        }
        for (const auto& reference : referenceMeshes)
        {
          edgeMeshes.push_back(reference.get());
        }
        if (!referenceWarning.empty())
        {
          auto& message = group->warnings[part.first];
          message += (message.empty() ? "" : "\n")+referenceWarning;
        }
      }
      else if (transformed)
      {
        edgeMeshes.push_back(part.second);
      }
    }
    for (auto* edgeMesh : edgeMeshes)
    {
      auto edges = vtkSmartPointer<vtkExtractEdges>::New();
      edges->SetInputData(edgeMesh);
      edges->UseAllPointsOn();
      const auto edgeTag = edges->AddObserver(vtkCommand::ErrorEvent, observer);
      edges->Update();
      edges->RemoveObserver(edgeTag);
      if (failed || edges->GetOutput() == nullptr)
      {
        error = "VTK could not extract the original mesh edges.";
        return {};
      }
      if (edges->GetOutput()->GetNumberOfCells() != 0)
      {
        auto edgeMapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
        edgeMapper->SetInputConnection(edges->GetOutputPort());
        edgeMapper->ScalarVisibilityOff();
        // Apply only a mapper-local line bias; never change the host-wide
        // coincident-topology mode. Opaque surfaces occlude interior edges.
        edgeMapper->SetRelativeCoincidentTopologyLineOffsetParameters(0.0, -1.0);
        auto edgeActor = vtkSmartPointer<vtkOpenGLActor>::New();
        edgeActor->SetMapper(edgeMapper);
        auto edgeProperty = vtkSmartPointer<vtkOpenGLProperty>::New();
        const auto& edgeColor = edgeMesh == mesh ? Postprocessor::DisplayDefaults::edge_color :
          Postprocessor::DeformationDefaults::overlay_color;
        edgeProperty->SetColor(edgeColor[0], edgeColor[1], edgeColor[2]);
        edgeProperty->SetLineWidth(Postprocessor::DisplayDefaults::edge_width);
        edgeProperty->LightingOff();
        edgeActor->SetProperty(edgeProperty);
        if (edgeMesh != mesh)
        {
          edgeActor->PickableOff();
        }
        // Reference geometry follows its owning IP dataset, even when the
        // ordinary parent block representation is hidden. The mapper pipeline
        // retains its extracted input after referenceMeshes leaves this scope.
        group->actors.push_back({part.first,edgeActor});
      }
    }
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
    const auto& state = application.session();
    const auto view = state.views.find(viewId);
    if (view == state.views.end())
    {
      clear();
      error = "The viewport's view definition is unavailable.";
      return false;
    }
    if (!geometryRevision || *geometryRevision != revision)
    {
      clear_hover();
      std::map<std::string, std::shared_ptr<RepresentationActors>> next;
      unsigned int legendSlot = 0;
      for (const auto& entry : state.representations)
      {
        const auto& display = entry.second;
        if (display.view_id != viewId || display.input.owner != Postprocessor::OutputOwner::Source || display.input.port != 0)
        {
          continue;
        }
        const auto data = application.result_data(display.input.object_id);
        if (!data)
        {
          continue;
        }
        const auto existing = displays.find(entry.first);
        std::shared_ptr<RepresentationActors> group;
        if (existing != displays.end() && existing->second->data == data && existing->second->increment == display.increment_index &&
            Postprocessor::same_deformation(existing->second->deformation,display.deformation))
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
          group = build(data,display,buildError);
          if (!group)
          {
            error += (error.empty() ? "" : "\n") + display.name + ": " + buildError;
            continue;
          }
        }
        std::optional<std::array<double,2>> allFrames;
        if (display.visible && display.coloring.field && display.coloring.range_mode == Postprocessor::ColorRangeMode::AllFrames)
        {
          std::string rangeError;
          allFrames = application.all_frame_range(display.input.object_id,*display.coloring.field,rangeError,display.hidden_datasets);
          if (!rangeError.empty())
          {
            error += (error.empty() ? "" : "\n")+display.name+": "+rangeError;
          }
        }
        group->coloring->apply(display, *data, group->surfaces, legendSlot, allFrames, error);
        if (display.visible && display.coloring.field && display.coloring.legend_visible &&
            std::any_of(group->surfaces.begin(),group->surfaces.end(),[&](const PostprocessorColorSurface& surface)
            {
              return Postprocessor::dataset_visible(display,surface.dataset) &&
                (!display.coloring.field->dataset_id || *display.coloring.field->dataset_id == surface.dataset);
            }))
        {
          ++legendSlot;
        }
        for (const auto& actor : group->actors)
        {
          actor.actor->SetVisibility(Postprocessor::dataset_visible(display,actor.dataset));
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
            renderer->RemoveActor(actor.actor);
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
            renderer->AddActor(actor.actor);
          }
        }
      }
      displays.swap(next);
      displayWarning.clear();
      for (const auto& entry : displays)
      {
        const auto definition = state.representations.find(entry.first);
        if (definition != state.representations.end())
        {
          for (const auto& message : entry.second->warnings)
          {
            if (Postprocessor::dataset_visible(definition->second,message.first))
            {
              displayWarning += (displayWarning.empty() ? "" : "\n")+definition->second.name+
                " / Dataset "+std::to_string(message.first)+": "+message.second;
            }
          }
        }
      }
      if (application.display_revision() != revision)
      {
        clear();
        error = "The display inputs changed during scene synchronization.";
        return false;
      }
      std::array<double, 6> bounds;
      renderer->ComputeVisiblePropBounds(bounds.data());
      visibleBounds = valid_bounds(bounds) ? std::optional<std::array<double, 6>>(bounds) : std::nullopt;
      appliedCamera.reset();
      if (error.empty())
      {
        geometryRevision = revision;
      }
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
    update_selection(application);
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
  if (appliedCamera && Postprocessor::CameraOperations::equal(*appliedCamera, definition))
  {
    return;
  }
  clear_hover();
  auto* camera = renderer->GetActiveCamera();
  camera->SetPosition(definition.position.data());
  camera->SetFocalPoint(definition.focal_point.data());
  camera->SetViewUp(definition.view_up.data());
  camera->SetParallelProjection(definition.parallel_projection);
  camera->SetViewAngle(definition.vertical_view_angle_degrees);
  camera->SetParallelScale(definition.parallel_scale);
  if (visibleBounds)
  {
    renderer->ResetCameraClippingRange(visibleBounds->data());
  }
  appliedCamera = definition;
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

void PostprocessorScene::apply_appearance(const std::array<double, 3>& top, const std::array<double, 3>& bottom)
{
  renderer->SetBackground(bottom[0], bottom[1], bottom[2]);
  renderer->SetBackground2(top[0], top[1], top[2]);
  renderer->SetGradientBackground(top != bottom);
  for (const auto& display : displays)
  {
    display.second->coloring->update_appearance();
  }
}

void PostprocessorScene::scale_capture(double scale)
{
  for (const auto& display : displays)
  {
    display.second->coloring->set_capture_scale(scale);
  }
}
