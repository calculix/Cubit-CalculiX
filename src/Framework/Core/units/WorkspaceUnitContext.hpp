#pragma once
#include "Framework/ModelExport.hpp"
#include <map>
#include <string>

namespace SimUnits {
struct CUBITSIM_MODEL_EXPORT WorkspaceUnitContext {
  WorkspaceUnitContext();
  explicit WorkspaceUnitContext(std::string workspace);
  std::string workspace;
  std::string system = "N-mm-s";
  std::string native_system;
  std::map<std::string, std::string> display_units;
};
}
