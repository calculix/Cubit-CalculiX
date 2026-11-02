#include "CoreDrawScene.hpp"
#include <utility>

CoreDrawScene::CoreDrawScene()
{
  for (auto group : {DrawGroup::BoundaryConditions, DrawGroup::Loads, DrawGroup::Constraints, DrawGroup::Orientations, DrawGroup::Topology})
  {
    state.groups.emplace(group, DrawGroupState{});
  }
}

void CoreDrawScene::replace(DrawEntityKey entity, DrawGeometry geometry)
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
  picker.deactivate();
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
  if (requested.stepId >= 0)
  {
    return group->second.allInScope && group->second.sourceScope.kind == DrawScopeKind::Step && group->second.sourceScope.stepId == requested.stepId;
  }
  return true;
}

bool CoreDrawScene::setSelection(const CalculiXDrawSelection& requested)
{
  const auto previous = selection;
  selection = requested.active ? requested : CalculiXDrawSelection{};
  if (previous.active == selection.active && previous.entity.feature == selection.entity.feature && previous.entity.id == selection.entity.id && previous.stepId == selection.stepId)
  {
    return false;
  }
  // Keep the request even when its group is hidden or its symbols are not drawn yet.
  // Only synchronize now if the visible highlighting actually changes.
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
  return workspaceActive && state.overlayVisible && state.pickingEnabled && picker.active() && picker.ownsPickMode();
}

bool CoreDrawScene::setPickingEnabled(bool enabled, std::string& error)
{
  picker.deactivate();
  state.pickingEnabled = enabled;
  return synchronize(error);
}

void CoreDrawScene::setPickCallback(std::function<void(const CalculiXDrawSelection&)> callback)
{
  picker.setCallback(std::move(callback));
}

bool CoreDrawScene::synchronize(std::string& error)
{
  DrawPickSnapshot pickData;
  pickData.revision = ++pickRevision;
  std::map<DrawGroup, DrawGeometry> groups;
  for (const auto& entity : entities)
  {
    const auto group = state.groups.find(drawing_group(entity.first.feature));
    if (state.pickingEnabled && entity.first.feature != DrawFeature::Topology && workspaceActive && state.overlayVisible && group != state.groups.end() && group->second.enabled)
    {
      CalculiXDrawSelection owner;
      owner.active = true;
      owner.entity = entity.first;
      if (group->second.allInScope && group->second.sourceScope.kind == DrawScopeKind::Step)
      {
        owner.stepId = group->second.sourceScope.stepId;
      }
      pickData.entities.push_back({owner, entity.second});
    }
    if (isHighlighted(entity.first, selection))
    {
      DrawGeometry highlighted = entity.second;
      for (auto& primitive : highlighted.primitives)
      {
        primitive.color = selectionColor;
      }
      groups[drawing_group(entity.first.feature)].append(highlighted);
    }
    else
    {
      groups[drawing_group(entity.first.feature)].append(entity.second);
    }
  }
  picker.updateSnapshot(std::move(pickData));
  if (picker.active() && !picker.ownsPickMode())
  {
    // Respect a native pick mode the user selected after enabling ours.
    state.pickingEnabled = false;
  }
  if (!state.pickingEnabled || !workspaceActive || !state.overlayVisible)
  {
    picker.deactivate();
  }
  if (!graphics.synchronize(groups, state, workspaceActive, error))
  {
    picker.deactivate();
    return false;
  }
  if (state.pickingEnabled && workspaceActive && state.overlayVisible && !picker.activate(error))
  {
    state.pickingEnabled = false;
    return false;
  }
  return true;
}

void CoreDrawScene::detach() noexcept
{
  picker.shutdown();
  graphics.detach();
}

void CoreDrawScene::clearHover() noexcept
{
  picker.clearHover();
}

void CoreDrawScene::setColors(const std::string& hoverColor, const std::string& selectedColor)
{
  selectionColor = selectedColor;
  picker.setHoverColor(hoverColor);
}

bool CoreDrawScene::pickSymbol(int viewId, double xFraction, double yFraction, CalculiXDrawSelection& selection, std::string& error)
{
  selection = {};
  error.clear();
  if (!pickingActive())
  {
    return true;
  }
  return picker.pickSymbol(viewId, xFraction, yFraction, selection, error);
}
