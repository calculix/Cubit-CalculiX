#pragma once
#include "Framework/Core/units/UnitTypes.hpp"
#include <string>

namespace SimUnits
{
struct QuantityUnit;
}

// A serializable assertion about an already converted cell, not a converter.
class ReportQuantity
{
public:
  ReportQuantity();
  ReportQuantity(const SimUnits::QuantityUnit& value);
  std::string quantity;
  std::string unit;
  SimUnits::Dimension dimension = SimUnits::Dimensionless;
  SimUnits::UnitRole role = SimUnits::UnitRole::Linear;
};
