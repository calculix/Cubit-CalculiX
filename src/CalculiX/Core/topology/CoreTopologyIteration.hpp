#ifndef CORETOPOLOGYITERATION_HPP
#define CORETOPOLOGYITERATION_HPP

#include <vector>

class CoreTopology;
class CoreMaterials;
class CoreJobs;

class CoreTopologyIteration
{
public:
  static bool select_iteration(CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs, int topology_id, int iteration_id);
  static bool mark_completed(CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id);
  static bool set_displacement(CoreTopology& topology, int topology_id, int iteration_id, double displacement, int node_id);
  static bool set_factors(const CoreTopology& topology, CoreMaterials& materials, int topology_id, int iteration_id, const std::vector<int>& element_ids, const std::vector<double>& stiffness_factors);
};

#endif
