#include "Core/CalculiXCore.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyEvaluation.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::evaluate_topology_compliance(int topology_id, int iteration_id, const std::map<int, double>& energies)
{
  if (!topology || !mat)
  {
    PRINT_ERROR("Invalid topology compliance evaluation request.\n");
    return false;
  }

  return CoreTopologyEvaluation::evaluate_compliance(*topology, *mat, topology_id, iteration_id, energies);
}

bool CalculiXCore::evaluate_topology_displacement(int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals)
{
  if (!topology || !mat || !jobs)
  {
    PRINT_ERROR("Invalid topology displacement evaluation request.\n");
    return false;
  }

  return CoreTopologyEvaluation::evaluate_displacement(*topology, *mat, *jobs, topology_id, iteration_id, value, node_id, element_integrals);
}

bool CalculiXCore::evaluate_topology_displacement_projected_residual(int topology_id, int iteration_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->evaluate_displacement_projected_residual(topology_id, iteration_id);
}

bool CalculiXCore::evaluate_topology_compliance_oc_residual(int topology_id, int iteration_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->evaluate_compliance_oc_residual(topology_id, iteration_id);
}
