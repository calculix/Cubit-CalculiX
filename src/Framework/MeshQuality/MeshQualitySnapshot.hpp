#pragma once
#include "MeshQualityBlock.hpp"
#include <array>
#include <memory>
class MeshQualitySnapshot
{
public:
  MeshQualitySnapshot();
  std::string mesh_fingerprint, input_fingerprint, coordinate_unit;
  std::string evaluator_version = "Cubit native quality";
  std::vector<std::int64_t> node_ids;
  std::vector<std::array<double, 3>> coordinates;
  std::vector<MeshQualityBlock> blocks;
  std::vector<std::string> diagnostics;
  std::string fingerprint() const;
  bool validate(std::string& error) const;
};
