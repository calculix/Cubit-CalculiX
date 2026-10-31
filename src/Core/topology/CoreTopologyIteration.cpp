#include "CoreTopologyIteration.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyQuery.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "CubitMessage.hpp"
#include <cmath>

bool CoreTopologyIteration::select_iteration(CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs, int topology_id, int iteration_id)
{
  if (iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology iteration selection request.\n");
    return false;
  }

  const auto entry = topology.topology_cases.find(topology_id);
  const auto collection = materials.topology_material_data.find(topology_id);

  if (entry == topology.topology_cases.end() || collection == materials.topology_material_data.end())
  {
    PRINT_ERROR("Topology %d has no prepared iteration data.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;

  for (const auto& iteration : configuration.iteration_jobs)
  {
    const int status = jobs.get_job_status(iteration.second);

    if (status == 1 || status == 5)
    {
      PRINT_ERROR("Topology %d still has an active job.\n", topology_id);
      return false;
    }
  }

  const int job_id = CoreTopologyQuery::get_iteration_job_id(topology, jobs, topology_id, iteration_id);

  if (job_id <= 0 || jobs.get_job_status(job_id) != 2 ||
      collection->second.iteration_factors.count(iteration_id) == 0 ||
      configuration.iteration_evaluations.count(iteration_id) == 0)
  {
    PRINT_ERROR("Topology %d iteration %d is not a completed, evaluated result.\n", topology_id, iteration_id);
    return false;
  }

  configuration.selected_iteration = iteration_id;

  PRINT_INFO("Topology %d selected iteration %d, job %d.\n", topology_id, iteration_id, job_id);
  return true;
}

bool CoreTopologyIteration::mark_completed(CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id)
{
  if (iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology iteration completion request.\n");
    return false;
  }

  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const auto job = entry->second.iteration_jobs.find(iteration_id);

  if (job == entry->second.iteration_jobs.end() || jobs.get_job_status(job->second) != 2 || entry->second.iteration_evaluations.count(iteration_id) == 0)
  {
    PRINT_ERROR("Topology iteration is not ready for completion.\n");
    return false;
  }

  entry->second.selected_iteration = iteration_id;
  return true;
}

bool CoreTopologyIteration::set_displacement(CoreTopology& topology, int topology_id, int iteration_id, double displacement, int node_id)
{
  if (iteration_id < 1 || !std::isfinite(displacement) || displacement < 0.0 || node_id <= 0)
  {
    PRINT_ERROR("Invalid topology displacement measurement.\n");
    return false;
  }

  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;
  const auto evaluation = configuration.iteration_evaluations.find(iteration_id);

  if (evaluation == configuration.iteration_evaluations.end() || configuration.displacement_nodeset <= 0 || configuration.displacement_component < 0 || configuration.displacement_component > 3)
  {
    PRINT_ERROR("Missing topology evaluation or invalid displacement selection.\n");
    return false;
  }

  evaluation->second.displacement_available = true;
  evaluation->second.displacement_value = displacement;
  evaluation->second.displacement_node = node_id;
  evaluation->second.displacement_nodeset = configuration.displacement_nodeset;
  evaluation->second.displacement_component = configuration.displacement_component;
  return true;
}

bool CoreTopologyIteration::set_factors(const CoreTopology& topology, CoreMaterials& materials, int topology_id, int iteration_id, const std::vector<int>& element_ids, const std::vector<double>& stiffness_factors)
{
  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (element_ids.empty() || element_ids.size() != stiffness_factors.size())
  {
    PRINT_ERROR("Invalid topology iteration factor data.\n");
    return false;
  }

  const auto& preserved = entry->second.element_preserved;

  if (preserved.empty())
  {
    PRINT_ERROR("Preservation for topology %d has not been resolved.\n", topology_id);
    return false;
  }

  // Keep the caller's data unchanged.
  std::vector<double> factors = stiffness_factors;

  for (std::size_t i = 0; i < element_ids.size(); ++i)
  {
    const int element_id = element_ids[i];
    const auto flag = preserved.find(element_id);

    if (flag == preserved.end())
    {
      PRINT_ERROR("Missing preservation flag for element %d.\n", element_id);
      return false;
    }

    if (!std::isfinite(factors[i]) || factors[i] <= 0.0 || factors[i] > 1.0)
    {
      PRINT_ERROR("Invalid stiffness factor for element %d.\n", element_id);
      return false;
    }

    if (flag->second)
    {
      factors[i] = 1.0;
    }
  }

  if (!materials.set_topology_iteration_factors(topology_id, iteration_id, element_ids, factors, entry->second.material_cap))
  {
    PRINT_ERROR("Failed to store topology %d iteration %d factors.\n", topology_id, iteration_id);
    return false;
  }

  return true;
}
