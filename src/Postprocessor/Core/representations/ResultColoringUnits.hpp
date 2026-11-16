#pragma once
#include "ResultColoring.hpp"
namespace SimUnits
{
  class CoreFrameworkUnits;
}
namespace Postprocessor
{
  // Bind numbers entered in the current Postprocessor presentation unit. This
  // does not convert values when choosing a different field/component.
  bool bind_coloring_units(ResultColoring& coloring, const SimResults::Field& field, std::string& error);
  bool valid_coloring_units(const std::optional<SimUnits::QuantityUnit>& unit, std::string& error);
  bool same_coloring_units(const std::optional<SimUnits::QuantityUnit>& a, const std::optional<SimUnits::QuantityUnit>& b);
  // Strong guarantee: failure preserves coloring. Use role-aware conversion
  // through SI so absolute temperatures and differences remain distinct.
  bool convert_coloring_units(ResultColoring& coloring, const SimUnits::CoreFrameworkUnits& units, std::string& error);
}
