#include "CoreDraw.hpp"
#include "CoreDrawScope.hpp"
#include "Core/loadCalculiXCore.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "loadUserOptions.hpp"
#include "CubitMessage.hpp"
#include "GWindowFactory.hpp"
#include <algorithm>
#include <cmath>
#include <set>
#include <utility>

namespace
{
bool validScope(const DrawScope& scope)
{
  return (scope.kind == DrawScopeKind::Model || scope.kind == DrawScopeKind::Custom) ? scope.stepId == -1 : scope.kind == DrawScopeKind::Step && scope.stepId > 0;
}

bool validFeature(DrawFeature feature)
{
  switch (feature)
  {
    case DrawFeature::Force:
    case DrawFeature::Pressure:
    case DrawFeature::Heatflux:
    case DrawFeature::Gravity:
    case DrawFeature::Centrifugal:
    case DrawFeature::Trajectory:
    case DrawFeature::Film:
    case DrawFeature::Radiation:
    case DrawFeature::SurfaceTraction:
    case DrawFeature::BodyHeatflux:
    case DrawFeature::Displacement:
    case DrawFeature::Temperature:
    case DrawFeature::Orientation:
    case DrawFeature::Equation:
    case DrawFeature::EquationGroup: return true;
    default: return false;
  }
}

bool validateSavedState(const DrawSavedState& saved, std::string& error)
{
  if (!saved.labels.validate(error))
  {
    return false;
  }
  if (!std::isfinite(saved.sizeFactor) || saved.sizeFactor <= 0 || !validScope(saved.scope) || saved.groups.size() != 4)
  {
    error = "Invalid drawing size, scope or group count.";
    return false;
  }
  std::set<DrawGroup> seen;
  for (const auto& group : saved.groups)
  {
    if ((group.group != DrawGroup::BoundaryConditions && group.group != DrawGroup::Loads && group.group != DrawGroup::Constraints && group.group != DrawGroup::Orientations) || !seen.insert(group.group).second || !validScope(group.sourceScope))
    {
      error = "Invalid or duplicate drawing group.";
      return false;
    }
    for (const auto feature : group.featureFilter)
    {
      if (!validFeature(feature) || drawing_group(feature) != group.group)
      {
        error = "Drawing feature filter does not match its group.";
        return false;
      }
    }
    for (const auto* entities : {&group.entities, &group.entityFilter})
    {
      for (const auto& entity : *entities)
      {
        if (!validFeature(entity.feature) || entity.id <= 0 || drawing_group(entity.feature) != group.group)
        {
          error = "Invalid drawing entity reference.";
          return false;
        }
      }
    }
  }
  return true;
}
}

DrawSavedState CoreDraw::savedState() const
{
  DrawSavedState saved;
  saved.scope = scene.state.scope;
  saved.labels = scene.state.labels;
  if (saved.scope.kind != DrawScopeKind::Step)
  {
    saved.scope.stepId = -1;
  }
  saved.sizeFactor = scene.state.sizeFactor;
  saved.overlayVisible = scene.state.overlayVisible;
  for (const auto& entry : scene.state.groups)
  {
    if (entry.first == DrawGroup::Topology)
    {
      continue;
    }
    const auto& group = entry.second;
    DrawSavedGroup target;
    target.group = entry.first;
    target.enabled = group.enabled;
    target.initialized = group.initialized;
    target.allInScope = group.allInScope;
    target.sourceScope = group.sourceScope;
    if (target.sourceScope.kind != DrawScopeKind::Step)
    {
      target.sourceScope.stepId = -1;
    }
    for (const auto feature : group.featureFilter)
    {
      if (drawing_group(feature) == entry.first)
      {
        target.featureFilter.push_back(feature);
      }
    }
    for (const auto& key : group.entityFilter)
    {
      if (drawing_group(key.feature) == entry.first)
      {
        target.entityFilter.push_back(key);
      }
    }
    // Scope-wide entity lists are derived; explicit drawing requests must survive a reload.
    if (!group.allInScope)
    {
      target.entities = group.entities;
    }
    if (!group.featureFilter.empty() && target.featureFilter.empty())
    {
      target.allInScope = false;
      target.initialized = true;
      target.entities.clear();
    }
    saved.groups.push_back(std::move(target));
  }
  return saved;
}

bool CoreDraw::restoreState(const DrawSavedState& saved, std::string& error)
{
  error.clear();
  if (!validateSavedState(saved, error))
  {
    return false;
  }
  CalculiXDrawState candidate;
  candidate.labels = saved.labels;
  candidate.scope = saved.scope;
  candidate.sizeFactor = saved.sizeFactor;
  candidate.overlayVisible = saved.overlayVisible;
  bool missingStep = false;
  if (candidate.scope.kind == DrawScopeKind::Step && !ccx_core.steps->check_step_exists(candidate.scope.stepId))
  {
    candidate.scope = {};
    missingStep = true;
  }
  ResolvedDrawScope model;
  if (!CoreDrawScope::resolveModel(ccx_core, model, error))
  {
    return false;
  }
  const std::set<DrawEntityKey> available(model.entities.begin(), model.entities.end());
  bool missingEntity = false;
  for (const auto& source : saved.groups)
  {
    auto& group = candidate.groups[source.group];
    group.enabled = source.enabled;
    group.initialized = source.initialized;
    group.dirty = true;
    group.allInScope = source.allInScope;
    group.sourceScope = source.sourceScope;
    group.featureFilter = source.featureFilter;
    group.entityFilter = source.entityFilter;
    group.entities = source.entities;
    if (group.sourceScope.kind == DrawScopeKind::Step && !ccx_core.steps->check_step_exists(group.sourceScope.stepId))
    {
      group.sourceScope = {};
      missingStep = true;
    }
    for (auto* references : {&group.entities, &group.entityFilter})
    {
      references->erase(std::remove_if(references->begin(), references->end(), [&](const DrawEntityKey& key)
      {
        const bool missing = available.count(key) == 0;
        missingEntity = missingEntity || missing;
        return missing;
      }), references->end());
    }
    // Removing the final restriction for a feature must not turn it into "all entities".
    std::set<DrawFeature> emptyFeatures;
    for (const auto& key : source.entityFilter)
    {
      if (std::none_of(group.entityFilter.begin(), group.entityFilter.end(), [&](const DrawEntityKey& remaining) { return remaining.feature == key.feature; }))
      {
        emptyFeatures.insert(key.feature);
      }
    }
    if (!emptyFeatures.empty() && group.allInScope)
    {
      if (group.featureFilter.empty())
      {
        for (auto feature : {DrawFeature::Force, DrawFeature::Pressure, DrawFeature::Heatflux, DrawFeature::Gravity, DrawFeature::Centrifugal, DrawFeature::Trajectory, DrawFeature::Film, DrawFeature::Radiation, DrawFeature::SurfaceTraction, DrawFeature::BodyHeatflux, DrawFeature::Displacement, DrawFeature::Temperature, DrawFeature::Orientation, DrawFeature::Equation, DrawFeature::EquationGroup})
        {
          if (drawing_group(feature) == source.group)
          {
            group.featureFilter.push_back(feature);
          }
        }
      }
      group.featureFilter.erase(std::remove_if(group.featureFilter.begin(), group.featureFilter.end(), [&](DrawFeature feature) { return emptyFeatures.count(feature) != 0; }), group.featureFilter.end());
      if (group.featureFilter.empty())
      {
        group.allInScope = false;
        group.entities.clear();
        group.initialized = true;
      }
    }
  }
  candidate.groups.emplace(DrawGroup::Topology, DrawGroupState{});
  scene.clearAll();
  scene.state = std::move(candidate);
  activeTopologyId = -1;
  activeTopologyIteration = -1;
  restorePending = true;
  if (missingStep)
  {
    PRINT_WARNING("Saved drawing step no longer exists; using model scope.\n");
  }
  if (missingEntity)
  {
    PRINT_WARNING("Removed missing entity references from saved drawing requests.\n");
  }
  return true;
}

void CoreDraw::resetToDefaults()
{
  scene.clearAll();
  const auto& defaults = ccx_uo.mDrawing;
  scene.state.labels.enabled = defaults.labelsEnabled;
  scene.state.labels.loads = defaults.labelsLoads;
  scene.state.labels.boundaryConditions = defaults.labelsBcs;
  scene.state.labels.selectedOnly = defaults.labelsSelectedOnly;
  scene.state.labels.showId = defaults.labelsIds;
  scene.state.labels.showName = defaults.labelsNames;
  scene.state.labels.showValues = defaults.labelsValues;
  scene.state.labels.showAmplitude = defaults.labelsAmplitudes;
  scene.state.labels.leaderLines = defaults.labelsLeaders;
  scene.state.labels.offsetFactor = defaults.labelsOffset;
  scene.state.labels.color = defaults.labelsColor;

  scene.state.sizeFactor = std::isfinite(defaults.sizeFactor) && defaults.sizeFactor > 0 ? defaults.sizeFactor : 1.0;
  scene.state.overlayVisible = defaults.overlayVisible;
  scene.state.groups[DrawGroup::BoundaryConditions].enabled = defaults.boundaryConditions;
  scene.state.groups[DrawGroup::Loads].enabled = defaults.loads;
  scene.state.groups[DrawGroup::Constraints].enabled = defaults.constraints;
  scene.state.groups[DrawGroup::Orientations].enabled = defaults.orientations;
  activeTopologyId = -1;
  activeTopologyIteration = -1;
  restorePending = true;
  scene.setColors(defaults.hoverColor, defaults.selectionColor);
}

bool CoreDraw::restorePendingDrawing(std::string& error)
{
  error.clear();
  auto* factory = GWindowFactory::get_window_factory();
  if (!restorePending || !scene.workspaceActive || factory == nullptr || factory->views().empty())
  {
    return true;
  }
  return refreshDrawing(error);
}

bool CoreDraw::applyDrawingOptions(std::string& error)
{
  scene.setColors(ccx_uo.mDrawing.hoverColor, ccx_uo.mDrawing.selectionColor);
  auto labels = scene.state.labels;
  labels.color = ccx_uo.mDrawing.labelsColor;
  if (!labels.validate(error))
  {
    return false;
  }
  scene.state.labels = std::move(labels);
  error.clear();
  auto* factory = GWindowFactory::get_window_factory();
  if (!scene.workspaceActive || factory == nullptr || factory->views().empty())
  {
    return true;
  }
  return scene.synchronize(error);
}
