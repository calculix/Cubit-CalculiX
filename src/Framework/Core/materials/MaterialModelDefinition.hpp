#pragma once
#include "MaterialColumnDefinition.hpp"
#include <string>
#include <vector>
struct MaterialModelDefinition {
  std::string key;
  std::string title;
  std::string property;
  std::string selectionProperty;
  std::vector<MaterialColumnDefinition> columns;
  bool scalar = false;
};
