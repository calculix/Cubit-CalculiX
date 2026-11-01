#ifndef CORETOPOLOGYDENSITY_HPP
#define CORETOPOLOGYDENSITY_HPP

#include <map>

class CoreTopology;
class CoreMaterials;
class CoreBlocks;

class CoreTopologyDensity
{
public:
  static bool set_densities(CoreTopology& topology, CoreMaterials& materials, CoreBlocks& blocks, int topology_id, int iteration_id, const std::map<int, double>& densities, int source_iteration);
};

#endif
