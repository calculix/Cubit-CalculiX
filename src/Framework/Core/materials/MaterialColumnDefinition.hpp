#pragma once
#include <string>
struct MaterialColumnDefinition {
  std::string key;
  std::string title;
  std::string quantity;
  bool optional = false;
};
