#include "Core/CalculiXCore.hpp"
#include "CoreTopologyJobs.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::reset_topology_run(int topology_id)
{
  if (!topology || !jobs)
  {
    PRINT_ERROR("Topology or job core is not initialized.\n");
    return false;
  }

  return CoreTopologyJobs::reset_run(*topology, *jobs, mat, sections, topologyoptimization, topology_id);
}

bool CalculiXCore::delete_topology(int topology_id)
{
  if (!topology || !jobs)
  {
    PRINT_ERROR("Topology or job core is not initialized.\n");
    return false;
  }

  return CoreTopologyJobs::delete_topology(*topology, *jobs, mat, sections, topologyoptimization, topology_id);
}

int CalculiXCore::create_topology_iteration_job(int topology_id, int iteration_id, const std::string& job_name)
{
  if (!topology || !mat || !jobs)
  {
    PRINT_ERROR("Invalid topology iteration job request.\n");
    return -1;
  }

  const auto validate_model = [this]()
  {
    return validate_topology_compliance_model();
  };

  return CoreTopologyJobs::create_iteration_job(*topology, *mat, *jobs, topology_id, iteration_id, job_name, validate_model);
}

bool CalculiXCore::register_topology_adjoint_job(int topology_id, int iteration_id, int& job_id)
{
  job_id = -1;

  if (!topology || !jobs)
  {
    PRINT_ERROR("Invalid topology adjoint job request.\n");
    return false;
  }

  return CoreTopologyJobs::register_adjoint_job(*topology, *jobs, topology_id, iteration_id, job_id);
}
