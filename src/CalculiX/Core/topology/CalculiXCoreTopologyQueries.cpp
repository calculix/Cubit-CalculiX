#include "Core/CalculiXCore.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyQuery.hpp"
#include "CubitMessage.hpp"

std::vector<std::vector<std::string>> CalculiXCore::get_topology_iteration_data(int topology_id)
{
  if (!topology || !jobs)
  {
    PRINT_ERROR("Topology/job cores are not initialized.\n");
    return {};
  }

  return CoreTopologyQuery::get_iteration_data(*topology, *jobs, topology_id);
}

std::string CalculiXCore::get_topology_run_state(int topology_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return "";
  }

  return topology->get_topology_run_state(topology_id);
}

std::vector<std::vector<double>> CalculiXCore::get_topology_iteration_element_data(int topology_id, int iteration_id)
{
  if (!topology || !mat || !jobs)
  {
    PRINT_ERROR("Invalid topology element-data request.\n");
    return {};
  }

  return CoreTopologyQuery::get_iteration_element_data(*topology, *mat, *jobs, topology_id, iteration_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_topology_tree_data()
{
  if (!topology)
  {
    return {};
  }

  return topology->get_tree_data();
}

bool CalculiXCore::get_topology_base_run_info(int topology_id, TopologyRunInfo& information)
{
  if (!topology)
  {
    information = {};
    return false;
  }

  return topology->get_topology_base_run_info(topology_id, information);
}

int CalculiXCore::get_topology_active_iteration_job(int topology_id, int iteration_id)
{
  if (!topology || !jobs)
  {
    return -1;
  }

  return CoreTopologyQuery::get_active_iteration_job(*topology, *jobs, topology_id, iteration_id);
}

bool CalculiXCore::get_job_topology_context(int job_id, int& topology_id, int& iteration_id)
{
  topology_id = -1;
  iteration_id = -1;

  if (!topology)
  {
    PRINT_ERROR("Invalid job topology context query.\n");
    return false;
  }

  return CoreTopologyQuery::get_job_context(*topology, job_id, topology_id, iteration_id);
}

int CalculiXCore::get_topology_adjoint_job_source(int topology_id, int iteration_id)
{
  if (!topology || !jobs)
  {
    PRINT_ERROR("Invalid topology adjoint job request.\n");
    return -1;
  }

  return CoreTopologyQuery::get_adjoint_job_source(*topology, *jobs, topology_id, iteration_id);
}

int CalculiXCore::get_topology_iteration_job_id(int topology_id, int iteration_id)
{
  if (!topology || !jobs)
  {
    PRINT_ERROR("Invalid topology iteration query.\n");
    return -1;
  }

  return CoreTopologyQuery::get_iteration_job_id(*topology, *jobs, topology_id, iteration_id);
}

int CalculiXCore::get_topology_iteration_adjoint_job_id(int topology_id, int iteration_id)
{
  if (!topology || !jobs)
  {
    PRINT_ERROR("Invalid topology adjoint job query.\n");
    return -1;
  }

  return CoreTopologyQuery::get_iteration_adjoint_job_id(*topology, *jobs, topology_id, iteration_id);
}
