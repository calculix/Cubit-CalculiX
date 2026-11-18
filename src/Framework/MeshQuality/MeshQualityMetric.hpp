#pragma once
#include <optional>
#include <string>
class MeshQualityMetric
{
public:
  MeshQualityMetric();
  // IDs identify stable native definitions; changed semantics require a new ID.
  std::string family, id, native_name, label;
  std::optional<double> minimum, maximum;
  bool enabled_by_default = false;
  bool requires_group_reference = false;
  // Dimensions are expressed by quantity/unit; limits are stored in this unit.
  std::string quantity = "dimensionless";
  std::string unit = "1";
  std::string key() const;
};
