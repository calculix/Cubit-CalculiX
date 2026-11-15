#pragma once
#include "MaterialCard.hpp"
#include <map>
#include <string>
struct MaterialData {
  std::string schema = "calculix";
  std::string name;
  std::string description;
  std::map<std::string, MaterialCard> cards;
};
