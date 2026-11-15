#pragma once
#include "MaterialTableRow.hpp"
#include <optional>
#include <vector>
struct MaterialCardValues {
  std::vector<MaterialTableRow> rows;
  std::optional<double> scalar;
  std::optional<double> referenceTemperature;
};
