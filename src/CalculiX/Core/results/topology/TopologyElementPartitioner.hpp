#pragma once
#include "Framework/Results/VtkResultsData.hpp"
#include <map>
class TopologyElementPartitioner
{
public:
  // Global element ID -> block slot, cell type and ordered original node IDs.
  using MeshIdentity = std::map<long long,std::vector<long long>>;
  static bool split(const SimResults::VtkResultsData& child, const SimResults::Increment& frame,
    const std::vector<std::vector<double>>& elements, const std::vector<std::vector<double>>& nodes,
    bool levelSet, double threshold, SimResults::Increment& output,
    std::vector<SimResults::Dataset>& datasets, MeshIdentity& identity, std::string& error);
};
