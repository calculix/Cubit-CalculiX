#include "Core/CalculiXCore.hpp"
#include "CoreTopologyIteration.hpp"
#include "Core/topology/CoreTopologyConsoleMessages.hpp"

bool CalculiXCore::set_topology_iteration_displacement(int topology_id, int iteration_id, double displacement, int node_id)
{
  if (!topology)
  {
    TOPOLOGY_ERROR("Invalid topology displacement measurement.\n");
    return false;
  }

  return CoreTopologyIteration::set_displacement(*topology, topology_id, iteration_id, displacement, node_id);
}

bool CalculiXCore::mark_topology_iteration_completed(int topology_id, int iteration_id)
{
  if (!topology || !jobs)
  {
    TOPOLOGY_ERROR("Invalid topology iteration completion request.\n");
    return false;
  }

  return CoreTopologyIteration::mark_completed(*topology, *jobs, topology_id, iteration_id);
}

bool CalculiXCore::set_topology_iteration_factors(int topology_id, int iteration_id, const std::vector<int>& element_ids, const std::vector<double>& stiffness_factors)
{
  if (!topology || !mat)
  {
    TOPOLOGY_ERROR("Topology/material cores are not initialized.\n");
    return false;
  }

  return CoreTopologyIteration::set_factors(*topology, topology_id, iteration_id, element_ids, stiffness_factors);
}

bool CalculiXCore::select_topology_iteration(int topology_id, int iteration_id)
{
  if (!topology || !mat || !jobs)
  {
    TOPOLOGY_ERROR("Invalid topology iteration selection request.\n");
    return false;
  }

  return CoreTopologyIteration::select_iteration(*topology, *mat, *jobs, topology_id, iteration_id);
}
