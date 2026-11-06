#include "CoreDrawScene.hpp"
#include "CalculiXDrawingAdapter.hpp"
#include <utility>

CoreDrawScene::CoreDrawScene()
{
  for (auto group : {DrawGroup::BoundaryConditions, DrawGroup::Loads, DrawGroup::Constraints, DrawGroup::Orientations, DrawGroup::Topology})
  {
    state.groups.emplace(group, DrawGroupState{});
  }
}

void CoreDrawScene::bind(DrawingInterface* service) noexcept
{
  if (drawings != service)
  {
    detach();
    drawings = service;
  }
}

void CoreDrawScene::replace(DrawEntityKey entity, DrawingGeometry geometry)
{
  entities[entity] = std::move(geometry);
}

void CoreDrawScene::clear(DrawGroup group)
{
  for (auto entry = entities.begin(); entry != entities.end();)
  {
    if (drawing_group(entry->first.feature) == group)
    {
      entry = entities.erase(entry);
    }
    else
    {
      ++entry;
    }
  }
}

void CoreDrawScene::clearAll()
{
  if (drawings != nullptr)
  {
    drawings->clear("calculix");
  }
  entities.clear();
  selection = {};
  state = CalculiXDrawState{};
  for (auto group : {DrawGroup::BoundaryConditions, DrawGroup::Loads, DrawGroup::Constraints, DrawGroup::Orientations, DrawGroup::Topology})
  {
    state.groups.emplace(group, DrawGroupState{});
  }
}

bool CoreDrawScene::isHighlighted(DrawEntityKey entity, const CalculiXDrawSelection& requested) const
{
  if (!requested.active || requested.entity.feature != entity.feature || requested.entity.id != entity.id)
  {
    return false;
  }
  const auto group = state.groups.find(drawing_group(entity.feature));
  if (group == state.groups.end() || !group->second.enabled || !workspaceActive || !state.overlayVisible)
  {
    return false;
  }
  return requested.stepId < 0 || (group->second.allInScope && group->second.sourceScope.kind == DrawScopeKind::Step && group->second.sourceScope.stepId == requested.stepId);
}

bool CoreDrawScene::setSelection(const CalculiXDrawSelection& requested)
{
  const auto previous = selection;
  selection = requested.active ? requested : CalculiXDrawSelection{};
  if (previous.active == selection.active && previous.entity.feature == selection.entity.feature && previous.entity.id == selection.entity.id && previous.stepId == selection.stepId)
  {
    return false;
  }
  for (const auto& entity : entities)
  {
    if (isHighlighted(entity.first, previous) != isHighlighted(entity.first, selection))
    {
      return true;
    }
  }
  return false;
}

bool CoreDrawScene::pickingActive() const
{
  return drawings != nullptr && workspaceActive && drawings->picking_active("calculix");
}

bool CoreDrawScene::setPickingEnabled(bool enabled, std::string& error)
{
  state.pickingEnabled = enabled;
  clearHover();
  return synchronize(error);
}

void CoreDrawScene::setPickCallback(std::function<void(const CalculiXDrawSelection&)> callback)
{
  pickCallback = std::move(callback);
  if (drawings != nullptr)
  {
    const auto target = pickCallback;
    drawings->set_pick_callback("calculix", target ? DrawingPickCallback([target](const DrawingSelection& picked)
    {
      CalculiXDrawSelection selected;
      if (CalculiXDrawingAdapter::selection(picked, selected))
      {
        target(selected);
      }
    }) : DrawingPickCallback{});
  }
}

bool CoreDrawScene::synchronize(std::string& error)
{
  error.clear();
  if (drawings == nullptr)
  {
    if (!workspaceActive || entities.empty())
    {
      return true;
    }
    error = "The CalculiX drawing service is unavailable.";
    return false;
  }
  DrawingFrame frame;
  frame.style = {hoverColor, selectionColor};
  frame.overlayVisible = state.overlayVisible && workspaceActive;
  frame.pickingEnabled = state.pickingEnabled;
  frame.selection = {selection.active, CalculiXDrawingAdapter::key(selection.entity, selection.stepId)};
  for (const auto& group : state.groups)
  {
    frame.layers[drawing_group_name(group.first)] = {group.second.enabled, group.first == DrawGroup::Topology};
  }
  for (const auto& entity : entities)
  {
    const auto group = state.groups.find(drawing_group(entity.first.feature));
    int step = -1;
    if (group != state.groups.end() && group->second.allInScope && group->second.sourceScope.kind == DrawScopeKind::Step)
    {
      step = group->second.sourceScope.stepId;
    }
    frame.entities.push_back({CalculiXDrawingAdapter::key(entity.first, step),
      drawing_group_name(drawing_group(entity.first.feature)), entity.second, entity.first.feature != DrawFeature::Topology});
  }
  // Reset may have cleared the shared scene and its callback.
  setPickCallback(pickCallback);
  if (!drawings->submit("calculix", std::move(frame), error))
  {
    return false;
  }
  if (workspaceActive && state.overlayVisible && state.pickingEnabled && !drawings->picking_active("calculix"))
  {
    state.pickingEnabled = false;
  }
  return true;
}

void CoreDrawScene::detach() noexcept
{
  if (drawings != nullptr)
  {
    drawings->clear("calculix");
  }
}

void CoreDrawScene::clearHover() noexcept
{
  if (drawings != nullptr)
  {
    drawings->clear_hover("calculix");
  }
}

void CoreDrawScene::setColors(const std::string& hover, const std::string& selected)
{
  hoverColor = hover;
  selectionColor = selected;
}

bool CoreDrawScene::pickSymbol(int viewId, double x, double y, CalculiXDrawSelection& selected, std::string& error)
{
  selected = {};
  error.clear();
  if (!pickingActive())
  {
    return true;
  }
  DrawingSelection picked;
  return drawings->pick("calculix", viewId, x, y, picked, error) && CalculiXDrawingAdapter::selection(picked, selected);
}
