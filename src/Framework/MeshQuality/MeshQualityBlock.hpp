#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
class MeshQualityBlock
{
public:
  MeshQualityBlock();
  int block_id = -1;
  std::string block_name, family;
  std::size_t nodes_per_element = 0;
  std::vector<std::int64_t> element_ids;
  // Flat connectivity uses original node IDs; coordinates are shared by all blocks.
  std::vector<std::int64_t> connectivity;
  std::map<std::string, std::vector<double>> values;
  std::map<std::string, std::string> unavailable;
};
