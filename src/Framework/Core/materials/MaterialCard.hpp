#pragma once
#include "MaterialCardValues.hpp"
#include <string>
enum class MaterialCardState { Active, Inactive };
struct MaterialCard {
  MaterialCardState state = MaterialCardState::Inactive;
  std::string model;
  MaterialCardValues values;
};
