#include "CalculiXDrawingAdapter.hpp"
#include <array>
#include <charconv>
#include <utility>

namespace
{
const std::array<std::pair<DrawFeature, const char*>, 16> Features = {{
  {DrawFeature::Force, "force"}, {DrawFeature::Pressure, "pressure"},
  {DrawFeature::Heatflux, "heatflux"}, {DrawFeature::Gravity, "gravity"},
  {DrawFeature::Centrifugal, "centrifugal"}, {DrawFeature::Trajectory, "trajectory"},
  {DrawFeature::Film, "film"}, {DrawFeature::Radiation, "radiation"},
  {DrawFeature::SurfaceTraction, "surface_traction"}, {DrawFeature::BodyHeatflux, "body_heatflux"},
  {DrawFeature::Displacement, "displacement"}, {DrawFeature::Temperature, "temperature"},
  {DrawFeature::Orientation, "orientation"}, {DrawFeature::Equation, "equation"},
  {DrawFeature::EquationGroup, "equation_group"}, {DrawFeature::Topology, "topology"}
}};
}

DrawingEntityKey CalculiXDrawingAdapter::key(DrawEntityKey entity, int stepId)
{
  DrawingEntityKey result;
  result.workspace = "calculix";
  result.id = entity.id;
  for (const auto& feature : Features)
  {
    if (feature.first == entity.feature)
    {
      result.kind = feature.second;
      break;
    }
  }
  if (stepId >= 0)
  {
    result.context = "step:" + std::to_string(stepId);
  }
  return result;
}

bool CalculiXDrawingAdapter::selection(const DrawingSelection& input, CalculiXDrawSelection& output)
{
  output = {};
  if (!input.active)
  {
    return true;
  }
  if (input.entity.workspace != "calculix" || input.entity.id < 0)
  {
    return false;
  }
  bool found = false;
  for (const auto& feature : Features)
  {
    if (input.entity.kind == feature.second)
    {
      output.entity = {feature.first, input.entity.id};
      found = true;
      break;
    }
  }
  if (!found)
  {
    return false;
  }
  if (!input.entity.context.empty())
  {
    const auto& context = input.entity.context;
    if (context.compare(0, 5, "step:") != 0)
    {
      return false;
    }
    int step = -1;
    const auto result = std::from_chars(context.data() + 5, context.data() + context.size(), step);
    if (result.ec != std::errc{} || result.ptr != context.data() + context.size() || step < 0)
    {
      return false;
    }
    output.stepId = step;
  }
  output.active = true;
  return true;
}

bool CalculiXDrawingAdapter::component(int dof, DrawingSymbolComponent& output)
{
  switch (dof)
  {
    case 1: output = DrawingSymbolComponent::TranslationX; return true;
    case 2: output = DrawingSymbolComponent::TranslationY; return true;
    case 3: output = DrawingSymbolComponent::TranslationZ; return true;
    case 4: output = DrawingSymbolComponent::RotationX; return true;
    case 5: output = DrawingSymbolComponent::RotationY; return true;
    case 6: output = DrawingSymbolComponent::RotationZ; return true;
    case 11: output = DrawingSymbolComponent::Temperature; return true;
    default: return false;
  }
}

bool CalculiXDrawingAdapter::displacement(DrawingGeometry& geometry, std::vector<double> point, int dof, std::string color, double size)
{
  DrawingSymbolComponent value;
  return component(dof, value) && DrawingSymbols::displacement(geometry, std::move(point), value, std::move(color), size);
}

bool CalculiXDrawingAdapter::equationNode(DrawingGeometry& geometry, std::vector<double> point, int dof, std::string color, double size)
{
  DrawingSymbolComponent value;
  return component(dof, value) && DrawingSymbols::equationNode(geometry, std::move(point), value, std::move(color), size);
}
