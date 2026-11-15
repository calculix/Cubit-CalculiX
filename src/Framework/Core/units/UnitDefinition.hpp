#pragma once
#include "Framework/ModelExport.hpp"
#include "UnitTypes.hpp"
#include <string>

namespace SimUnits {
struct CUBITSIM_MODEL_EXPORT UnitDefinition {
  UnitDefinition();
  UnitDefinition(std::string id, Dimension dimension, double scale, UnitRole role = UnitRole::Linear, double offset = 0.0);
  std::string id;
  Dimension dimension = Dimensionless;
  double scale = 1.0;
  UnitRole role = UnitRole::Linear;
  double offset = 0.0;
};
}
