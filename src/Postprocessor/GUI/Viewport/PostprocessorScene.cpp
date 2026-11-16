#include <type_traits>
#include "Postprocessor/Core/representations/ResultReferenceMesh.hpp"
#include "PostprocessorScene.hpp"
#include "PostprocessorScaleOverlay.hpp"
#include "Postprocessor/GUI/Filters/PostprocessorPlaneOverlay.hpp"
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

std::optional<PostprocessorScene::LegendHit> PostprocessorScene::legend_at(double x,double y) const
{
  // Reverse drawing order selects the topmost legend when two frames overlap.
  for (auto entry = displays.rbegin(); entry != displays.rend(); ++entry)
  {
    if (entry->second->coloring->legend_contains(x,y))
    {
      return LegendHit{entry->first,entry->second->source_id,entry->second->coloring->legend_frame()};
    }
  }
  return std::nullopt;
}

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

PostprocessorScene::PostprocessorScene(vtkRenderer* renderer, const std::string& view_id,bool showPlane)
  : renderer(renderer), viewId(view_id)
{
  scaleOverlay = std::make_unique<PostprocessorScaleOverlay>(renderer);
  if (showPlane)
  {
    planeOverlay = std::make_unique<PostprocessorPlaneOverlay>(renderer);
  }
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
  const Postprocessor::Representation& display, std::string& error,const Postprocessor::EvaluatedOutput* evaluated)
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
  if (evaluated)
  {
    group->warnings = evaluated->warnings;
  }
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
      message += ([&]() -> const char* {
        if (message.empty()) {
          return "";
        } else {
          return "\n";
        }
      }())+warning;
    }
    auto* mesh = [&]() -> std::common_type_t<decltype(transformed.get()), decltype(part.second)> {
      if (transformed) {
        return transformed.get();
      } else {
        return part.second;
      }
    }();
    if (transformed)
    {
      group->transformed.push_back(transformed);
    }
    const std::vector<Postprocessor::FilteredPart>::const_iterator filtered = [&]() -> std::vector<Postprocessor::FilteredPart>::const_iterator {
      if (evaluated) {
        return std::find_if(evaluated->parts.begin(), evaluated->parts.end(), [&](const auto& value) {
          return value.dataset == part.first;
        });
      }
    }();
    const auto* derived = [&]() -> std::common_type_t<decltype(&*filtered), decltype(nullptr)> {
      if (evaluated && filtered != evaluated->parts.end()) {
        return &*filtered;
      } else {
        return nullptr;
      }
    }();
    if (mesh->GetNumberOfCells() == 0)
    {
      continue;
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
    auto* nodeMesh = [&]() -> std::common_type_t<decltype(static_cast<vtkDataSet*>(derived->nodes.get())), decltype(mesh)> {
      if (derived && derived->nodes) {
        return static_cast<vtkDataSet*>(derived->nodes.get());
      } else {
        return mesh;
      }
    }();
    if (nodeMesh->GetPointData()->HasArray("OriginalNodeId") && !nodeMesh->GetPointData()->HasArray("IntegrationPointId"))
    {
      auto nodes = vtkSmartPointer<vtkPolyData>::New();
      if (auto* pointSet = vtkPointSet::SafeDownCast(nodeMesh))
      {
        nodes->SetPoints(pointSet->GetPoints());
      }
      else
      {
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetDataTypeToDouble();
        points->SetNumberOfPoints(nodeMesh->GetNumberOfPoints());
        for (vtkIdType i = 0; i < nodeMesh->GetNumberOfPoints(); ++i)
        {
          double position[3];
          nodeMesh->GetPoint(i,position);
          points->SetPoint(i,position);
        }
        nodes->SetPoints(points);
      }
      auto vertices = vtkSmartPointer<vtkCellArray>::New();
      for (vtkIdType i = 0; i < nodeMesh->GetNumberOfPoints(); ++i)
      {
        vertices->InsertNextCell(1,&i);
      }
      nodes->SetVerts(vertices);
      nodes->GetPointData()->ShallowCopy(nodeMesh->GetPointData());
      auto nodeMapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
      nodeMapper->SetInputData(nodes);
      nodeMapper->ScalarVisibilityOff();
      // CPU picking uses original solver nodes, including midside nodes.
      // Keep this actor out of the renderer; only hover/selection draws a marker.
      auto nodeActor = vtkSmartPointer<vtkOpenGLActor>::New();
      nodeActor->SetMapper(nodeMapper);
      nodeActor->PickableOn();
      nodeActor->UseBoundsOff();
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
          message += ([&]() -> const char* {
            if (message.empty()) {
              return "";
            } else {
              return "\n";
            }
          }())+referenceWarning;
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
      if (!derived)
      {
        edges->Update();
      }
      edges->RemoveObserver(edgeTag);
      auto* edgeGeometry = [&]() -> std::common_type_t<decltype(derived->edges.get()), decltype(edges->GetOutput())> {
        if (derived && derived->edges) {
          return derived->edges.get();
        } else {
          return edges->GetOutput();
        }
      }();
      if (failed || edgeGeometry == nullptr)
      {
        error = "VTK could not extract the original mesh edges.";
        return {};
      }
      if (edgeGeometry->GetNumberOfCells() != 0)
      {
        auto edgeMapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
        edgeMapper->SetInputData(edgeGeometry);
        edgeMapper->ScalarVisibilityOff();
        // Apply only a mapper-local line bias; never change the host-wide
        // coincident-topology mode. Opaque surfaces occlude interior edges.
        edgeMapper->SetRelativeCoincidentTopologyLineOffsetParameters(0.0, -1.0);
        auto edgeActor = vtkSmartPointer<vtkOpenGLActor>::New();
        edgeActor->SetMapper(edgeMapper);
        auto edgeProperty = vtkSmartPointer<vtkOpenGLProperty>::New();
        const auto& edgeColor = [&]() -> decltype((Postprocessor::DisplayDefaults::edge_color)) {
          if (edgeMesh == mesh) {
            return Postprocessor::DisplayDefaults::edge_color;
          } else {
            return Postprocessor::DeformationDefaults::overlay_color;
          }
        }();
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
    if (derived && derived->reference_edges && derived->reference_edges->GetNumberOfCells())
    {
      auto mapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
      mapper->SetInputData(derived->reference_edges.get());
      mapper->ScalarVisibilityOff();
      auto actor = vtkSmartPointer<vtkOpenGLActor>::New();
      actor->SetMapper(mapper);
      actor->PickableOff();
      const auto& color = Postprocessor::DeformationDefaults::overlay_color;
      actor->GetProperty()->SetColor(color[0],color[1],color[2]);
      actor->GetProperty()->LightingOff();
      group->actors.push_back({part.first,actor});
    }
  }
  if (group->actors.empty() && !evaluated)
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
        if (display.input.owner == Postprocessor::OutputOwner::Feature && !display.visible)
        {
          continue;
        }
        if (display.view_id != viewId || display.input.port != 0)
        {
          continue;
        }
        std::shared_ptr<const Postprocessor::EvaluatedOutput> evaluated;
        std::string evaluationError;
        if (!application.evaluate_output(display,evaluated,evaluationError))
        {
          if (application.result_data(application.output_source(display.input)))
          {
            error += ([&]() -> const char* {
              if (error.empty()) {
                return "";
              } else {
                return "\n";
              }
            }())+display.name+": "+evaluationError;
          }
          continue;
        }
        const auto data = evaluated->data;
        const bool feature = display.input.owner == Postprocessor::OutputOwner::Feature;
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
          auto geometryDisplay = display;
          if (feature)
          {
            geometryDisplay.deformation.enabled = false;
            geometryDisplay.deformation.overlay = false;
          }
          group = build(data,geometryDisplay,buildError,[&]() -> std::common_type_t<decltype(evaluated.get()), decltype(nullptr)> {
            if (feature) {
              return evaluated.get();
            } else {
              return nullptr;
            }
          }());
          if (group)
          {
            group->deformation = display.deformation;
            group->source_id = evaluated->source_id;
            group->evaluated = evaluated;
          }
          if (!group)
          {
            error += ([&]() -> const char* {
              if (error.empty()) {
                return "";
              } else {
                return "\n";
              }
            }()) + display.name + ": " + buildError;
            continue;
          }
        }
        double factor = 1;
        bool known = false;
        std::string unitError;
        if (!application.scene_scale(display,factor,known,unitError))
        {
          error += unitError;
          continue;
        }
        group->scene_scale = factor;
        for (auto& item : group->actors)
        {
          item.actor->SetScale(factor,factor,factor);
        }
        // Picking proxies are not rendered, but must use the same scene units.
        for (auto& surface : group->surfaces)
        {
          if (surface.nodes)
          {
            surface.nodes->SetScale(factor,factor,factor);
          }
        }
        auto colorDisplay = display;
        if (display.coloring.field)
        {
          for (const auto& fieldGroup : application.field_groups(evaluated->source_id,std::nullopt))
          {
            const auto& selected = *display.coloring.field;
            if (!fieldGroup.unit_issue.empty() && (fieldGroup.key == selected.group_key ||
                (selected.group_key.empty() && fieldGroup.field.array_name == selected.array_name && fieldGroup.field.association == selected.association)))
            {
              colorDisplay.coloring.field.reset();
              group->warnings[fieldGroup.field.dataset_id] = fieldGroup.unit_issue;
              break;
            }
          }
        }
        std::optional<std::array<double,2>> allFrames;
        if (colorDisplay.visible && colorDisplay.coloring.field && colorDisplay.coloring.range_mode == Postprocessor::ColorRangeMode::AllFrames)
        {
          std::string rangeError;
          allFrames = application.all_frame_range(evaluated->source_id,*display.coloring.field,rangeError,display.hidden_datasets);
          if (!rangeError.empty())
          {
            error += ([&]() -> const char* {
              if (error.empty()) {
                return "";
              } else {
                return "\n";
              }
            }())+display.name+": "+rangeError;
          }
        }
        group->coloring->apply(colorDisplay, *data, group->surfaces, legendSlot, allFrames, error, application.captured_units());
        if (colorDisplay.visible && colorDisplay.coloring.field && colorDisplay.coloring.legend_visible &&
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
              displayWarning += ([&]() -> const char* {
                if (displayWarning.empty()) {
                  return "";
                } else {
                  return "\n";
                }
              }())+definition->second.name+
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
      if (valid_bounds(bounds)) {
        visibleBounds = std::optional<std::array<double, 6>>(bounds);
      } else {
        visibleBounds = std::nullopt;
      }
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
    bool calibrated = visibleBounds.has_value() && !displays.empty();
    for (const auto& entry : displays)
    {
      const auto definition = state.representations.find(entry.first);
      if (definition != state.representations.end() && (!definition->second.visible ||
          std::none_of(entry.second->actors.begin(),entry.second->actors.end(),[](const auto& item)
          {
            return item.actor->GetVisibility() != 0;
          })))
      {
        continue;
      }
      double factor = 1;
      bool known = false;
      std::string unitError;
      if (definition == state.representations.end() ||
          !application.scene_scale(definition->second,factor,known,unitError) || !known)
      {
        calibrated = false;
      }
    }
    scaleOverlay->configure(view->second.scale_visible && calibrated,view->second.scene_unit,application.captured_units());
    update_selection(application);
    if (planeOverlay)
    {
      const auto active = state.representations.find(state.active_representation_id);
      const auto found = [&]() -> std::common_type_t<decltype(state.filters.find(active->second.input.object_id)), decltype(state.filters.end())> {
        if (active != state.representations.end() && active->second.view_id == viewId && active->second.visible &&
        active->second.input.owner == Postprocessor::OutputOwner::Feature) {
          return state.filters.find(active->second.input.object_id);
        } else {
          return state.filters.end();
        }
      }();
      const double extent = [&]() -> double {
        if (visibleBounds) {
          return std::hypot((*visibleBounds)[1]-(*visibleBounds)[0],(*visibleBounds)[3]-(*visibleBounds)[2],(*visibleBounds)[5]-(*visibleBounds)[4]);
        } else {
          return 1;
        }
      }();
      Postprocessor::FilterDefinition transformedPlane;
      const Postprocessor::FilterDefinition* shownPlane = nullptr;
      if (found != state.filters.end())
      {
        transformedPlane = found->second;
        double factor = 1;
        bool known = false;
        std::string ignored;
        if (application.scene_scale(active->second,factor,known,ignored))
        {
          for (auto& coordinate : transformedPlane.plane.origin)
          {
            coordinate *= factor;
          }
          shownPlane = &transformedPlane;
        }
      }
      planeOverlay->update(shownPlane,extent);
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
  if (appliedCamera && Postprocessor::CameraOperations::equal(*appliedCamera, definition))
  {
    scaleOverlay->update();
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
  scaleOverlay->update();
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
  scaleOverlay->update();
  for (const auto& display : displays)
  {
    display.second->coloring->update_appearance();
  }
}

void PostprocessorScene::scale_capture(double scale)
{
  scaleOverlay->set_capture_scale(scale);
  for (const auto& display : displays)
  {
    display.second->coloring->set_capture_scale(scale);
  }
}
