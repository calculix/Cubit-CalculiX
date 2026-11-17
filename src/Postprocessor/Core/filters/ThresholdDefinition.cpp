#include "ThresholdDefinition.hpp"
#include "../representations/ResultColoringUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <cmath>
#include <utility>
namespace Postprocessor
{
  bool valid_threshold(const ThresholdDefinition& value,std::string& error)
  {
    error.clear();
    if (value.field.array_name.empty() || value.field.array_name.size() > 4096 || value.field.component.size() > 4096 ||
        value.field.group_key.size() > 4096 || (value.field.dataset_id && value.dataset_kind.empty()) ||
        value.dataset_kind.size() > 4096 || value.dataset_name.size() > 4096 || value.dataset_model < -1 ||
        (!value.field.magnitude && value.field.component.empty()) ||
        value.field.association == SimResults::Association::Table)
    {
      error = "Choose a spatial threshold field and component.";
      return false;
    }
    if ((!value.lower && !value.upper) || (value.lower && !std::isfinite(*value.lower)) ||
        (value.upper && !std::isfinite(*value.upper)))
    {
      error = "Enable at least one finite threshold limit.";
      return false;
    }
    if (value.lower && value.upper && *value.lower > *value.upper)
    {
      error = "The lower threshold limit must not exceed the upper limit.";
      return false;
    }
    if ((value.node_rule != ThresholdNodeRule::All && value.node_rule != ThresholdNodeRule::Any) ||
        (value.field.association != SimResults::Association::Node &&
         value.field.association != SimResults::Association::Element &&
         value.field.association != SimResults::Association::IntegrationPoint) ||
        !value.bounds_unit || !valid_coloring_units(value.bounds_unit,error))
    {
      if (error.empty())
      {
        error = "Invalid threshold node rule, association or bound units.";
      }
      return false;
    }
    return true;
  }
  bool bind_threshold_units(ThresholdDefinition& value,const SimResults::Field& field,std::string& error)
  {
    ResultColoring coloring;
    coloring.field = value.field;
    if (!bind_coloring_units(coloring,field,error))
    {
      return false;
    }
    value.bounds_unit = coloring.bounds_unit;
    return true;
  }
  bool convert_threshold_units(ThresholdDefinition& value,const SimUnits::CoreFrameworkUnits& units,std::string& error)
  {
    if (!valid_threshold(value,error))
    {
      return false;
    }
    const auto& source = *value.bounds_unit;
    if (source.unit.empty() || source.quantity.empty())
    {
      return true;
    }
    std::string target;
    if (!units.display_unit("postprocessor",source.quantity,target,error))
    {
      return false;
    }
    auto candidate = value;
    for (auto* limit : {&candidate.lower,&candidate.upper})
    {
      if (*limit)
      {
        double canonical = 0, converted = 0;
        if (!units.to_si(**limit,source.unit,source.dimension,source.role,canonical,error) ||
            !units.from_si(canonical,target,source.dimension,source.role,converted,error))
        {
          return false;
        }
        *limit = converted;
      }
    }
    candidate.bounds_unit->unit = target;
    if (!valid_threshold(candidate,error))
    {
      return false;
    }
    value = std::move(candidate);
    return true;
  }
}
