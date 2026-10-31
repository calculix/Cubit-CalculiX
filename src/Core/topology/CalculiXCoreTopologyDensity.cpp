#include "Core/CalculiXCore.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyDensity.hpp"
#include "CoreTopologyOptimization.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::set_topology_iteration_densities(int topology_id, int iteration_id, const std::map<int, double>& densities, int source_iteration)
{
  if (!topology || !mat)
  {
    PRINT_ERROR("Topology/material cores are not initialized.\n");
    return false;
  }

  if (!cb)
  {
    PRINT_ERROR("Block core is not initialized.\n");
    return false;
  }

  return CoreTopologyDensity::set_densities(*topology, *mat, *cb, topology_id, iteration_id, densities, source_iteration);
}

bool CalculiXCore::prepare_topology_density_iteration(int topology_id, int source_iteration, int next_iteration)
{
  return topologyoptimization && topologyoptimization->prepare_topology_density_iteration(topology_id, source_iteration, next_iteration);
}

bool CalculiXCore::filter_topology_density_sensitivities(int topology_id, int iteration_id, const std::map<int, double>& element_volumes, const std::map<int, std::array<double, 3>>& element_centers)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->filter_density_sensitivities(topology_id, iteration_id, element_volumes, element_centers);
}

bool CalculiXCore::build_topology_density_update(int topology_id, int source_iteration, std::map<int, double>& densities)
{
  densities.clear();

  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->build_density_update(topology_id, source_iteration, densities);
}

bool CalculiXCore::check_topology_density_convergence(int topology_id, int previous_iteration, int current_iteration, bool& converged)
{
  converged = false;

  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->check_density_convergence(topology_id, previous_iteration, current_iteration, converged);
}
