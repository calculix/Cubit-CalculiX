#include "ResultColoringUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include <algorithm>
#include <cmath>
#include <utility>
namespace Postprocessor
{
  bool same_coloring_units(const std::optional<SimUnits::QuantityUnit>& a, const std::optional<SimUnits::QuantityUnit>& b)
  {
    if (a.has_value() != b.has_value())
    {
      return false;
    }
    if (!a)
    {
      return true;
    }
    return a->quantity == b->quantity && a->unit == b->unit && a->dimension == b->dimension && a->role == b->role;
  }

  bool valid_coloring_units(const std::optional<SimUnits::QuantityUnit>& value, std::string& error)
  {
    if (!value)
    {
      return true;
    }
    const auto& descriptor = *value;
    if (descriptor.unit.size() > 256 || descriptor.quantity.size() > 256 ||
        descriptor.unit.find('\0') != std::string::npos || descriptor.quantity.find('\0') != std::string::npos ||
        !std::all_of(descriptor.dimension.begin(), descriptor.dimension.end(), [](double exponent)
        {
          return std::isfinite(exponent);
        }) || (descriptor.role != SimUnits::UnitRole::Linear && descriptor.role != SimUnits::UnitRole::AbsoluteTemperature &&
        descriptor.role != SimUnits::UnitRole::TemperatureDifference && descriptor.role != SimUnits::UnitRole::Angle))
    {
      error = "Invalid coloring bounds unit metadata.";
      return false;
    }
    if (!descriptor.quantity.empty())
    {
      SimUnits::QuantityUnit expected;
      if (!framework_units().quantity(descriptor.quantity, expected, error))
      {
        return false;
      }
      if (expected.dimension != descriptor.dimension || expected.role != descriptor.role)
      {
        error = "Coloring bounds quantity and dimension/role disagree.";
        return false;
      }
    }
    if (!descriptor.unit.empty())
    {
      double ignored = 0.0;
      if (!framework_units().to_si(0.0, descriptor.unit, descriptor.dimension, descriptor.role, ignored, error))
      {
        return false;
      }
    }
    return true;
  }

  bool bind_coloring_units(ResultColoring& coloring, const SimResults::Field& field, std::string& error)
  {
    error.clear();
    if (!coloring.field)
    {
      coloring.bounds_unit.reset();
      return true;
    }
    SimUnits::QuantityUnit descriptor;
    if (!SimResults::ResultsUnits::field_quantity(field, coloring.field->component, coloring.field->magnitude, descriptor, error))
    {
      return false;
    }
    if (!descriptor.unit.empty() && !descriptor.quantity.empty() &&
        !framework_units().display_unit("postprocessor", descriptor.quantity, descriptor.unit, error))
    {
      return false;
    }
    if (!valid_coloring_units(descriptor, error))
    {
      return false;
    }
    coloring.bounds_unit = std::move(descriptor);
    return true;
  }

  bool convert_coloring_units(ResultColoring& coloring, const SimUnits::CoreFrameworkUnits& units, std::string& error)
  {
    error.clear();
    if (!coloring.field)
    {
      return true;
    }
    if (!coloring.bounds_unit)
    {
      error = "Resolve the result field before changing units for legacy range settings.";
      return false;
    }
    const auto& source = *coloring.bounds_unit;
    if (!valid_coloring_units(coloring.bounds_unit, error))
    {
      return false;
    }
    if (source.unit.empty() || source.quantity.empty())
    {
      return true;
    }
    std::string target;
    if (!units.display_unit("postprocessor", source.quantity, target, error))
    {
      return false;
    }
    if (target == source.unit)
    {
      return true;
    }
    auto candidate = coloring;
    for (double* value : {&candidate.fixed_range[0], &candidate.fixed_range[1], &candidate.minimum.value, &candidate.maximum.value})
    {
      double canonical = 0.0;
      double converted = 0.0;
      if (!units.to_si(*value, source.unit, source.dimension, source.role, canonical, error) ||
          !units.from_si(canonical, target, source.dimension, source.role, converted, error))
      {
        return false;
      }
      *value = converted;
    }
    candidate.bounds_unit->unit = target;
    if (!valid_coloring(candidate, error))
    {
      return false;
    }
    coloring = std::move(candidate);
    return true;
  }
}
