#pragma once
#include "Framework/ModelExport.hpp"
#include "UnitTypes.hpp"
#include <string>

namespace SimUnits {
struct CUBITSIM_MODEL_EXPORT QuantityUnit {
  QuantityUnit();
  QuantityUnit(std::string quantity, std::string unit, Dimension dimension, UnitRole role = UnitRole::Linear);
  std::string quantity;
  std::string unit;
  Dimension dimension = Dimensionless;
  UnitRole role = UnitRole::Linear;
};
}
