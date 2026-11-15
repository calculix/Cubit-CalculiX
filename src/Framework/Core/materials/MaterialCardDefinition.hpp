#pragma once
#include "MaterialModelDefinition.hpp"
#include <string>
#include <vector>
struct FrameworkMaterialCardDefinition {
  std::string key;
  std::string title;
  std::string prefix;
  std::vector<MaterialModelDefinition> models;
};
