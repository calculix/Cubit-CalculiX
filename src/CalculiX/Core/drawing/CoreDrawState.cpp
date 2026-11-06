#include "CoreDraw.hpp"
#include "CoreDrawScope.hpp"
#include "Core/loadCalculiXCore.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "GWindowFactory.hpp"
#include <QScopedValueRollback>
#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <utility>

namespace
{
bool sameKey(const DrawEntityKey& a, const DrawEntityKey& b)
{
  return !(a < b) && !(b < a);
}
}

double CoreDraw::modelSymbolSize() const
{
  const auto nodes = CubitInterface::parse_cubit_list("node", "all");
  if (!nodes.empty())
  {
    // Cubit's geometry bounding-box API does not accept mesh nodes.
    std::array<double, 3> minimum{};
    std::array<double, 3> maximum{};
    bool initialized = false;
    for (int node : nodes)
    {
      const auto point = CubitInterface::get_nodal_coordinates(node);
      if (!std::all_of(point.begin(), point.end(), [](double value) { return std::isfinite(value); }))
      {
        continue;
      }
      if (!initialized)
      {
        minimum = maximum = point;
        initialized = true;
      }
      else
      {
        for (std::size_t axis = 0; axis < minimum.size(); ++axis)
        {
          minimum[axis] = (std::min)(minimum[axis], point[axis]);
          maximum[axis] = (std::max)(maximum[axis], point[axis]);
        }
      }
    }
    const double extent = (std::max)({maximum[0] - minimum[0], maximum[1] - minimum[1], maximum[2] - minimum[2]});
    return initialized && std::isfinite(extent) && extent > 0 ? extent * 0.10 : 1.0;
  }
  const auto vertices = CubitInterface::parse_cubit_list("vertex", "all");
  if (vertices.empty())
  {
    return 1.0;
  }
  const auto box = CubitInterface::get_total_bounding_box("vertex", vertices);
  const double extent = (std::max)({box[2], box[5], box[8]});
  return std::isfinite(extent) && extent > 0 ? extent * 0.10 : 1.0;
}

double CoreDraw::defaultSymbolSize() const
{
  return modelSymbolSize() * scene.state.sizeFactor;
}

CalculiXDrawState CoreDraw::drawingState() const
{
  return scene.state;
}

bool CoreDraw::storeGeometry(DrawEntityKey key, DrawingGeometry geometry, double size)
{
  if (!std::isfinite(size) || size <= 0)
  {
    return false;
  }
  scene.replace(key, std::move(geometry));
  if (!rebuilding)
  {
    auto& group = scene.state.groups[drawing_group(key.feature)];
    if (std::none_of(group.entities.begin(), group.entities.end(), [&](const DrawEntityKey& current) { return sameKey(current, key); }))
    {
      group.entities.push_back(key);
    }
    group.enabled = true;
    group.initialized = true;
    group.dirty = false;
    group.allInScope = false;
    group.sourceScope = {};
    group.featureFilter.clear();
    group.entityFilter.clear();
    scene.state.scope.kind = DrawScopeKind::Custom;
    scene.state.sizeFactor = size / modelSymbolSize();
  }
  return true;
}

bool CoreDraw::buildEntity(DrawEntityKey key, double size)
{
  switch (key.feature)
  {
    case DrawFeature::Force: return draw_load_force(key.id, size);
    case DrawFeature::Pressure: return draw_load_pressure(key.id, size);
    case DrawFeature::Heatflux: return draw_load_heatflux(key.id, size);
    case DrawFeature::Gravity: return draw_load_gravity(key.id, size);
    case DrawFeature::Centrifugal: return draw_load_centrifugal(key.id, size);
    case DrawFeature::Trajectory: return draw_load_trajectory(key.id, size);
    case DrawFeature::Film: return draw_load_film(key.id, size);
    case DrawFeature::Radiation: return draw_load_radiation(key.id, size);
    case DrawFeature::SurfaceTraction: return draw_load_surface_traction(key.id, size);
    case DrawFeature::BodyHeatflux: return draw_load_bodyheatflux(key.id, size);
    case DrawFeature::Displacement: return draw_bc_displacement(key.id, size);
    case DrawFeature::Temperature: return draw_bc_temperature(key.id, size);
    case DrawFeature::Orientation: return draw_orientation(key.id, size);
    case DrawFeature::Equation: return draw_equation(key.id, size);
    case DrawFeature::EquationGroup: return draw_equationgroup(key.id, size);
    case DrawFeature::Topology: return true;
  }
  return false;
}

bool CoreDraw::refreshDrawing(std::string& error)
{
  error.clear();
  auto* factory = GWindowFactory::get_window_factory();
  if (!scene.workspaceActive || factory == nullptr || factory->views().empty())
  {
    restorePending = true;
    return true;
  }
  restorePending = false;
  ResolvedDrawScope model;
  if (!CoreDrawScope::resolveModel(ccx_core, model, error))
  {
    return false;
  }
  if (scene.state.scope.kind == DrawScopeKind::Step && !ccx_core.steps->check_step_exists(scene.state.scope.stepId))
  {
    scene.state.scope = {};
    for (auto& group : scene.state.groups)
    {
      if (group.second.sourceScope.kind == DrawScopeKind::Step && !ccx_core.steps->check_step_exists(group.second.sourceScope.stepId))
      {
        scene.clear(group.first);
        group.second = {};
      }
    }
  }
  const std::set<DrawEntityKey> available(model.entities.begin(), model.entities.end());
  const QScopedValueRollback<bool> guard(rebuilding, true);
  const double size = defaultSymbolSize();
  bool success = true;
  std::string firstError;
  // Resolve each requested step once even when several drawing groups use it.
  std::map<int, ResolvedDrawScope> steps;
  std::set<int> failedSteps;
  for (auto& entry : scene.state.groups)
  {
    if (entry.first == DrawGroup::Topology || !entry.second.enabled)
    {
      continue;
    }
    auto& group = entry.second;
    const ResolvedDrawScope* resolved = &model;
    if (group.allInScope && group.sourceScope.kind == DrawScopeKind::Step)
    {
      const int step = group.sourceScope.stepId;
      if (steps.count(step) == 0 && failedSteps.count(step) == 0)
      {
        if (!CoreDrawScope::resolveStep(ccx_core, step, size, steps[step], error))
        {
          steps.erase(step);
          failedSteps.insert(step);
        }
      }
      if (failedSteps.count(step) != 0)
      {
        scene.clear(entry.first);
        success = false;
        if (firstError.empty())
        {
          firstError = error.empty() ? "The selected step drawing is unavailable." : error;
        }
        continue;
      }
      resolved = &steps.at(step);
    }
    const auto accepted = [&](const DrawEntityKey& entity)
    {
      if (drawing_group(entity.feature) != entry.first || (!group.featureFilter.empty() && std::find(group.featureFilter.begin(), group.featureFilter.end(), entity.feature) == group.featureFilter.end()))
      {
        return false;
      }
      const bool filtered = std::any_of(group.entityFilter.begin(), group.entityFilter.end(), [&](const DrawEntityKey& key) { return key.feature == entity.feature; });
      return !filtered || std::any_of(group.entityFilter.begin(), group.entityFilter.end(), [&](const DrawEntityKey& key) { return sameKey(key, entity); });
    };
    std::vector<DrawEntityKey> selected;
    if (group.allInScope)
    {
      for (const auto& entity : resolved->entities)
      {
        if (accepted(entity))
        {
          selected.push_back(entity);
        }
      }
    }
    else
    {
      for (const auto& entity : group.entities)
      {
        if (available.count(entity) != 0)
        {
          selected.push_back(entity);
        }
      }
    }
    scene.clear(entry.first);
    for (const auto& entity : selected)
    {
      if (!buildEntity(entity, size))
      {
        success = false;
        if (firstError.empty())
        {
          firstError = "Unable to rebuild a drawing entity (ID " + std::to_string(entity.id) + ").";
        }
      }
    }
    if (group.allInScope)
    {
      for (const auto& geometry : resolved->geometry)
      {
        if (accepted(geometry.first))
        {
          scene.replace(geometry.first, geometry.second);
          selected.push_back(geometry.first);
        }
      }
    }
    group.entities = std::move(selected);
    group.initialized = true;
    group.dirty = false;
  }
  if (scene.state.groups[DrawGroup::Topology].enabled && activeTopologyId >= 0)
  {
    if (!draw_topology(activeTopologyId, activeTopologyIteration, activeTopologyThreshold))
    {
      scene.clear(DrawGroup::Topology);
      scene.state.groups[DrawGroup::Topology].enabled = false;
      success = false;
      if (firstError.empty())
      {
        firstError = "The topology preview is no longer available.";
      }
    }
  }
  std::string renderError;
  if (!scene.synchronize(renderError))
  {
    error = renderError;
    return false;
  }
  error = firstError;
  return success;
}

bool CoreDraw::setScope(const DrawScope& scope, std::string& error)
{
  error.clear();
  if (scope.kind == DrawScopeKind::Custom || (scope.kind == DrawScopeKind::Step && !ccx_core.steps->check_step_exists(scope.stepId)))
  {
    error = "Select an existing step or the entire model.";
    return false;
  }
  scene.state.scope = scope;
  for (auto& group : scene.state.groups)
  {
    if (group.first != DrawGroup::Topology)
    {
      group.second.sourceScope = scope;
      group.second.allInScope = true;
      group.second.featureFilter.clear();
      group.second.entityFilter.clear();
      group.second.dirty = true;
    }
  }
  return refreshDrawing(error);
}

bool CoreDraw::setGroupVisible(DrawGroup group, bool visible, std::string& error)
{
  error.clear();
  if (group == DrawGroup::Topology && visible && activeTopologyId < 0)
  {
    error = "Draw a topology result before enabling its preview.";
    return false;
  }
  auto& state = scene.state.groups[group];
  state.enabled = visible;
  if (visible && !state.initialized)
  {
    state.sourceScope = scene.state.scope.kind == DrawScopeKind::Custom ? DrawScope{} : scene.state.scope;
    state.allInScope = true;
  }
  if (visible)
  {
    return refreshDrawing(error);
  }
  return scene.synchronize(error);
}

bool CoreDraw::setSizeFactor(double factor, std::string& error)
{
  if (!std::isfinite(factor) || factor <= 0)
  {
    error = "Drawing size must be positive and finite.";
    return false;
  }
  scene.state.sizeFactor = factor;
  return refreshDrawing(error);
}

bool CoreDraw::drawScopeAll(std::string& error)
{
  for (auto& entry : scene.state.groups)
  {
    if (entry.first != DrawGroup::Topology)
    {
      entry.second.enabled = true;
      entry.second.allInScope = true;
      entry.second.featureFilter.clear();
      entry.second.entityFilter.clear();
      entry.second.sourceScope = scene.state.scope.kind == DrawScopeKind::Custom ? DrawScope{} : scene.state.scope;
    }
  }
  if (scene.state.scope.kind == DrawScopeKind::Custom)
  {
    scene.state.scope = {};
  }
  return refreshDrawing(error);
}

bool CoreDraw::pickingActive() const
{
  return scene.pickingActive();
}

bool CoreDraw::setPickingEnabled(bool enabled, std::string& error)
{
  return scene.setPickingEnabled(enabled, error);
}

void CoreDraw::setPickCallback(std::function<void(const CalculiXDrawSelection&)> callback)
{
  scene.setPickCallback(std::move(callback));
}

bool CoreDraw::setHighlight(const CalculiXDrawSelection& selection, std::string& error)
{
  error.clear();
  if (!scene.setSelection(selection))
  {
    return true;
  }
  return scene.synchronize(error);
}

bool CoreDraw::clearHighlight(std::string& error)
{
  return setHighlight({}, error);
}

bool CoreDraw::setOverlayVisible(bool visible, std::string& error)
{
  scene.state.overlayVisible = visible;
  return scene.synchronize(error);
}

void CoreDraw::setWorkspaceActive(bool active)
{
  scene.workspaceActive = active;
  if (!active)
  {
    scene.state.pickingEnabled = false;
    scene.clearHover();
    scene.setSelection({});
  }
  std::string error;
  const bool enabled = std::any_of(scene.state.groups.begin(), scene.state.groups.end(), [](const auto& group) { return group.second.enabled; });
  if (active && (enabled || restorePending))
  {
    if (!refreshDrawing(error))
    {
      PRINT_ERROR("Drawing refresh failed: %s\n", error.c_str());
    }
  }
  else if (!scene.synchronize(error) && enabled)
  {
    PRINT_WARNING("Drawing visibility could not be updated: %s\n", error.c_str());
  }
}

bool CoreDraw::renderPending()
{
  return graphics_flush();
}

void CoreDraw::resetDrawing()
{
  resetToDefaults();
  // Hide old graphics now; regeneration waits until model reset/load has completed.
  std::string error;
  // The scene owns native visibility restoration, including topology previews.
  auto* factory = GWindowFactory::get_window_factory();
  if (factory != nullptr && !factory->views().empty())
  {
    scene.synchronize(error);
  }
}

void CoreDraw::shutdownDrawing() noexcept
{
  shutdown_topology_animation();
  scene.detach();
}


bool CoreDraw::drawForStep(int stepId, const std::vector<DrawFeature>& features, double size, std::string& error, const std::vector<DrawEntityKey>& entities)
{
  error.clear();
  if (!ccx_core.steps->check_step_exists(stepId) || !std::isfinite(size) || size <= 0)
  {
    error = "Invalid step drawing request.";
    return false;
  }
  scene.state.scope = {DrawScopeKind::Step, stepId};
  scene.state.sizeFactor = size / modelSymbolSize();
  for (auto& entry : scene.state.groups)
  {
    if (entry.first == DrawGroup::Topology)
    {
      continue;
    }
    auto& group = entry.second;
    group.enabled = features.empty() || std::any_of(features.begin(), features.end(), [&](DrawFeature feature) { return drawing_group(feature) == entry.first; });
    group.sourceScope = scene.state.scope;
    group.allInScope = true;
    group.featureFilter = features;
    group.entityFilter = entities;
  }
  return refreshDrawing(error);
}

bool CoreDraw::setAllVisible(bool visible, std::string& error)
{
  if (visible)
  {
    return drawScopeAll(error);
  }
  for (auto& group : scene.state.groups)
  {
    if (group.first != DrawGroup::Topology)
    {
      group.second.enabled = false;
    }
  }
  return scene.synchronize(error);
}

void CoreDraw::clearHover() noexcept
{
  scene.clearHover();
}

bool CoreDraw::pickSymbol(int viewId, double xFraction, double yFraction, CalculiXDrawSelection& selection, std::string& error)
{
  selection = {};
  error.clear();
  if (!pickingActive())
  {
    return true;
  }
  return scene.pickSymbol(viewId, xFraction, yFraction, selection, error);
}

void CoreDraw::bindDrawing(DrawingInterface* drawings) noexcept
{
  scene.bind(drawings);
}
