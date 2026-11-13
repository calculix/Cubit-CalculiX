#include "CoreDrawSnapshotCapture.hpp"
#include "CalculiXDrawLabels.hpp"
#include "CalculiXDrawingAdapter.hpp"
#include <algorithm>

CoreDrawSnapshotCapture::CoreDrawSnapshotCapture() = default;

bool CoreDrawSnapshotCapture::buildFrame(const CalculiXJobSetupSnapshot& snapshot, const CalculiXCaptureRequest& request,
  DrawingFrame& output, std::string& error)
{
  if (!request.labels.validate(error) || request.labels.selectedOnly ||
      (request.scope.kind != DrawScopeKind::Model && request.scope.kind != DrawScopeKind::Step))
  {
    if (error.empty())
    {
      error = "Frozen captures require a model/step scope and selection-independent labels.";
    }
    return false;
  }
  if (request.content != CalculiXCaptureContent::Model && request.content != CalculiXCaptureContent::Setup &&
      request.content != CalculiXCaptureContent::Loads && request.content != CalculiXCaptureContent::BoundaryConditions)
  {
    error = "Invalid frozen capture content.";
    return false;
  }
  DrawingFrame frame;
  frame.style = {"black", "black"};
  frame.layers["model"] = {true, false};
  frame.layers[drawing_group_name(DrawGroup::Loads)] = {true, false};
  frame.layers[drawing_group_name(DrawGroup::BoundaryConditions)] = {true, false};
  frame.entities.push_back({{"calculix", "model", 0, snapshot.id}, "model", snapshot.model, false});
  if (request.content != CalculiXCaptureContent::Model)
  {
    const CalculiXJobSetupScene* scene = &snapshot.modelDefinitions;
    if (request.scope.kind == DrawScopeKind::Step)
    {
      const auto found = std::find_if(snapshot.steps.begin(), snapshot.steps.end(), [&](const CalculiXJobSetupScene& step)
      {
        return step.stepId == request.scope.stepId;
      });
      if (found == snapshot.steps.end())
      {
        error = "The selected analysis step is absent from the frozen job setup.";
        return false;
      }
      scene = &*found;
    }
    if (!scene->available)
    {
      error = scene->unavailableReason;
      return false;
    }
    for (const auto& item : scene->symbols)
    {
      const auto group = drawing_group(item.first.feature);
      if ((group != DrawGroup::Loads && group != DrawGroup::BoundaryConditions) ||
          (request.content == CalculiXCaptureContent::Loads && group != DrawGroup::Loads) ||
          (request.content == CalculiXCaptureContent::BoundaryConditions && group != DrawGroup::BoundaryConditions))
      {
        continue;
      }
      auto geometry = item.second;
      const bool show = request.labels.enabled && (group == DrawGroup::Loads ? request.labels.loads : request.labels.boundaryConditions);
      for (auto& primitive : geometry.primitives)
      {
        if (primitive.kind == DrawingPrimitiveKind::Label)
        {
          primitive.color = request.labels.color;
        }
      }
      if (!show)
      {
        geometry.primitives.erase(std::remove_if(geometry.primitives.begin(), geometry.primitives.end(), [](const DrawingPrimitive& primitive)
        {
          return primitive.kind == DrawingPrimitiveKind::Label;
        }), geometry.primitives.end());
      }
      else
      {
        const auto label = scene->labels.find(item.first);
        if (label != scene->labels.end() && !CalculiXDrawLabels::append(label->second, request.labels, snapshot.symbolSize, geometry, error))
        {
          return false;
        }
      }
      frame.entities.push_back({CalculiXDrawingAdapter::key(item.first, scene->stepId), drawing_group_name(group), std::move(geometry), false});
    }
  }
  output = std::move(frame);
  error.clear();
  return true;
}
