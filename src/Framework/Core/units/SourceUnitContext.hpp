#pragma once
#include "Framework/ModelExport.hpp"
#include <string>

namespace SimUnits {
struct CUBITSIM_MODEL_EXPORT SourceUnitContext {
  SourceUnitContext();
  bool known() const noexcept;
  std::string system;
  std::string coordinate_unit;
  std::string time_unit;
};
}
