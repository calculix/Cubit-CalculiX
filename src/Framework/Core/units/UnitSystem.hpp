#pragma once
#include "Framework/ModelExport.hpp"
#include "UnitTypes.hpp"
#include <string>

namespace SimUnits {
struct CUBITSIM_MODEL_EXPORT UnitSystem {
  UnitSystem();
  UnitSystem(std::string id, std::string lengthUnit, std::string forceUnit, std::string timeUnit, double lengthScale, double forceScale, double timeScale);
  bool scale_to_si(const Dimension& dimension, double& output, std::string& error) const;
  std::string id;
  std::string length_unit;
  std::string force_unit;
  std::string time_unit;
  std::array<double, 7> base_scale = {1, 1, 1, 1, 1, 1, 1};
};
}
