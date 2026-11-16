#include "PlotAxisUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <algorithm>
#include <cmath>
namespace Postprocessor
{
  namespace
  {
    const PlotCurveData* reference_curve(const PlotDefinition& definition,const PlotData& data)
    {
      for (std::size_t i = 0; i < definition.curves.size() && i < data.curves.size(); ++i)
      {
        const auto& curve = data.curves[i];
        if (definition.curves[i].visible && std::any_of(curve.samples.begin(),curve.samples.end(),[](const PlotSample& sample)
        {
          return sample.valid;
        }))
        {
          return &curve;
        }
      }
      return nullptr;
    }
    void convert_range(std::optional<std::array<double,2>>& range,const std::optional<SimUnits::QuantityUnit>& source,
      const SimUnits::QuantityUnit& target,const std::string& scope,const std::string& targetScope)
    {
      if (!range || !source)
      {
        return;
      }
      if (source->unit.empty() || target.unit.empty())
      {
        if (source->unit.empty() != target.unit.empty() || scope.empty() || scope != targetScope)
        {
          range.reset();
        }
        return;
      }
      if (source->dimension != target.dimension || source->role != target.role)
      {
        range.reset();
        return;
      }
      for (auto& value : *range)
      {
        double canonical = 0;
        double converted = 0;
        std::string error;
        if (!framework_units().to_si(value,source->unit,source->dimension,source->role,canonical,error) ||
            !framework_units().from_si(canonical,target.unit,target.dimension,target.role,converted,error) || !std::isfinite(converted))
        {
          range.reset();
          return;
        }
        value = converted;
      }
    }
  }
  bool bind_plot_axes(const PlotDefinition& definition,const PlotData& data,PlotAxes& axes,std::string& error)
  {
    error.clear();
    axes.x_unit.reset();
    axes.y_unit.reset();
    axes.x_scope.clear();
    axes.y_scope.clear();
    axes.x_frame_axis = false;
    if (!axes.x && !axes.y)
    {
      return true;
    }
    const auto* curve = reference_curve(definition,data);
    if (!curve)
    {
      error = "Resolve a visible curve before setting manual plot ranges.";
      return false;
    }
    if (axes.x)
    {
      axes.x_unit = curve->x_quantity;
      axes.x_scope = curve->x_scope;
      axes.x_frame_axis = curve->frame_axis;
    }
    if (axes.y)
    {
      axes.y_unit = curve->y_quantity;
      axes.y_scope = curve->y_scope;
    }
    return true;
  }
  PlotAxes presented_plot_axes(const PlotDefinition& definition,const PlotData& data)
  {
    auto axes = definition.axes;
    const auto* curve = reference_curve(definition,data);
    if (!curve)
    {
      axes.x.reset();
      axes.y.reset();
      return axes;
    }
    if (axes.x_unit && axes.x_frame_axis != curve->frame_axis)
    {
      axes.x.reset();
    }
    convert_range(axes.x,axes.x_unit,curve->x_quantity,axes.x_scope,curve->x_scope);
    convert_range(axes.y,axes.y_unit,curve->y_quantity,axes.y_scope,curve->y_scope);
    if (!valid_plot_axes(axes))
    {
      axes.x.reset();
      axes.y.reset();
    }
    return axes;
  }
}
