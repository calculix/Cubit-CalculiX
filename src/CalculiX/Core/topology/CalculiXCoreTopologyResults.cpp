#include "Core/CalculiXCore.hpp"
#include "CoreTopologyResults.hpp"
#include "CoreTopologyDisplacementResults.hpp"
#include "CoreTopologyTensorResults.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::read_topology_job_displacements(int job_id, std::map<int, std::array<double, 3>>& displacements)
{
  displacements.clear();

  if (!results || !jobs || !topology || !mat)
  {
    PRINT_ERROR("Topology displacement results require a completed job.\n");
    return false;
  }

  return CoreTopologyResults::read_job_displacements(*results, *jobs, *topology, *mat, cb, job_id, displacements);
}

bool CalculiXCore::read_topology_displacement(int topology_id, int iteration_id, double& value, int& node_id, std::array<double, 3>* critical_displacement)
{
  value = 0.0;
  node_id = -1;

  if (critical_displacement)
  {
    critical_displacement->fill(0.0);
  }

  if (!topology || !results || !jobs || !me_iface)
  {
    PRINT_ERROR("Topology displacement interfaces are not initialized.\n");
    return false;
  }

  return CoreTopologyDisplacementResults::measure_iteration(*results, *jobs, *topology, *me_iface, topology_id, iteration_id, value, node_id, critical_displacement);
}

bool CalculiXCore::read_topology_job_element_tensors(int job_id, const std::string& key, std::map<int, std::map<int, std::array<double, 6>>>& tensors)
{
  tensors.clear();

  if (key != "S" && key != "ME")
  {
    PRINT_ERROR("Topology tensor reader supports S and ME only.\n");
    return false;
  }

  if (!topology || !mat || !results || !jobs)
  {
    PRINT_ERROR("Topology tensor results require a completed job.\n");
    return false;
  }

  return CoreTopologyTensorResults::read_job(*results, *jobs, *topology, *mat, job_id, key, tensors);
}

bool CalculiXCore::read_topology_element_energies(int topology_id, int iteration_id, std::map<int, double>& element_energies)
{
  element_energies.clear();

  if (!topology || !mat || !results || !jobs)
  {
    PRINT_ERROR("Invalid topology energy query.\n");
    return false;
  }

  return CoreTopologyResults::read_iteration_energies(*results, *jobs, *topology, *mat, topology_id, iteration_id, element_energies);
}

bool CalculiXCore::check_topology_adjoint_results(int job_id)
{
  std::map<int, std::array<double, 3>> displacements;

  return read_topology_job_displacements(job_id, displacements);
}
