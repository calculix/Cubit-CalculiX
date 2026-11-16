#include "PlotAxisUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <algorithm>
#include <cmath>
#include <utility>
namespace Postprocessor
{
  bool record_plot_quantity(SimUnits::QuantityUnit& stored, bool& initialized,
    const SimUnits::QuantityUnit& value, std::string& error)
  {
    if (!initialized)
    {
      stored = value;
      initialized = true;
      return true;
    }
    if (stored.unit.empty() != value.unit.empty() || stored.dimension != value.dimension || stored.role != value.role ||
        (!stored.unit.empty() && stored.unit != value.unit))
    {
      error = "A curve changes physical quantity or presentation unit across frames.";
      return false;
    }
    return true;
  }
  namespace
  {
    std::string axis_title(const std::string& title, const std::string& unit)
    {
      const auto suffix = title.rfind(" [");
      if (suffix != std::string::npos && !title.empty() && title.back() == ']')
      {
        return title.substr(0,suffix)+" ["+unit+"]";
      }
      return title+" ["+unit+"]";
    }
    bool normalize_axis(PlotCurveData& curve, const PlotCurveData& reference, bool x, std::string& error)
    {
      SimUnits::QuantityUnit* quantity = &curve.y_quantity;
      const SimUnits::QuantityUnit* target = &reference.y_quantity;
      std::string* label = &curve.y_unit;
      std::string* title = &curve.y_title;
      const std::string* scope = &curve.y_scope;
      const std::string* referenceScope = &reference.y_scope;
      if (x)
      {
        quantity = &curve.x_quantity;
        target = &reference.x_quantity;
        label = &curve.x_unit;
        title = &curve.x_title;
        scope = &curve.x_scope;
        referenceScope = &reference.x_scope;
        if (curve.frame_axis != reference.frame_axis)
        {
          error = "Frame indices cannot share an axis with physical/analysis values.";
          return false;
        }
      }
      if (quantity->unit.empty() || target->unit.empty())
      {
        if (quantity->unit.empty() != target->unit.empty() || scope->empty() || *scope != *referenceScope)
        {
          error = "Unknown units do not establish a common raw-data basis.";
          return false;
        }
        return true;
      }
      if (quantity->dimension != target->dimension || quantity->role != target->role)
      {
        error = "The visible curves require incompatible axis quantities.";
        return false;
      }
      for (auto& sample : curve.samples)
      {
        if (!sample.valid)
        {
          continue;
        }
        double* value = &sample.y;
        if (x)
        {
          value = &sample.x;
        }
        double canonical = 0.0;
        double converted = 0.0;
        if (!framework_units().to_si(*value,quantity->unit,quantity->dimension,quantity->role,canonical,error) ||
            !framework_units().from_si(canonical,target->unit,target->dimension,target->role,converted,error))
        {
          return false;
        }
        *value = converted;
        if (x && curve.time_axis)
        {
          sample.time = sample.x;
        }
      }
      quantity->unit = target->unit;
      *label = target->unit;
      *title = axis_title(*title,target->unit);
      if (x && curve.time_axis)
      {
        curve.time_unit = target->unit;
      }
      return true;
    }
  }
  bool normalize_plot_axes(const PlotDefinition& definition, PlotData& data, std::string& error)
  {
    error.clear();
    auto candidate = data;
    std::optional<std::size_t> first;
    for (std::size_t i = 0; i < definition.curves.size() && i < candidate.curves.size(); ++i)
    {
      auto& curve = candidate.curves[i];
      if (!definition.curves[i].visible || std::none_of(curve.samples.begin(),curve.samples.end(),[](const PlotSample& sample)
        {
          return sample.valid;
        }))
      {
        continue;
      }
      if (!first)
      {
        first = i;
        continue;
      }
      const auto& reference = candidate.curves[*first];
      if (!normalize_axis(curve,reference,true,error) || !normalize_axis(curve,reference,false,error))
      {
        error = "Curves ["+definition.curves[*first].name+"] and ["+definition.curves[i].name+"]: "+error;
        return false;
      }
    }
    data = std::move(candidate);
    return true;
  }
}
