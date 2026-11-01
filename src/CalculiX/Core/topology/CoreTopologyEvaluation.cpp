#include "CoreTopologyEvaluation.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyQuery.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "CubitMessage.hpp"
#include <cmath>

bool CoreTopologyEvaluation::evaluate_compliance(CoreTopology& topology, const CoreMaterials& materials, int topology_id, int iteration_id, const std::map<int, double>& energies)
{
  if (iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology compliance evaluation request.\n");
    return false;
  }

  const auto collection = materials.topology_material_data.find(topology_id);

  if (collection == materials.topology_material_data.end())
  {
    PRINT_ERROR("Topology material data is missing.\n");
    return false;
  }

  const auto factors = collection->second.iteration_factors.find(iteration_id);

  if (factors == collection->second.iteration_factors.end())
  {
    PRINT_ERROR("Topology iteration stiffness factors are missing.\n");
    return false;
  }

  if (!topology.set_iteration_element_energies(topology_id, iteration_id, energies))
  {
    return false;
  }

  return topology.evaluate_compliance(topology_id, iteration_id, factors->second);
}

bool CoreTopologyEvaluation::evaluate_displacement(CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs, int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals)
{
  if (iteration_id < 1 || !std::isfinite(value) || value < 0.0 || node_id <= 0)
  {
    PRINT_ERROR("Invalid topology displacement evaluation request.\n");
    return false;
  }

  const int physical_job = CoreTopologyQuery::get_iteration_job_id(topology, jobs, topology_id, iteration_id);

  if (physical_job <= 0 || jobs.get_job_status(physical_job) != 2)
  {
    PRINT_ERROR("Completed physical results are required for displacement evaluation.\n");
    return false;
  }

  if (value > 0.0)
  {
    const int adjoint_job = CoreTopologyQuery::get_iteration_adjoint_job_id(topology, jobs, topology_id, iteration_id);

    if (adjoint_job <= 0 || jobs.get_job_status(adjoint_job) != 2)
    {
      PRINT_ERROR("Completed adjoint results are required for displacement evaluation.\n");
      return false;
    }
  }

  const auto collection = materials.topology_material_data.find(topology_id);

  if (collection == materials.topology_material_data.end() || collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Topology material data is missing.\n");
    return false;
  }

  if (element_integrals.size() != collection->second.element_base_material.size())
  {
    PRINT_ERROR("Incomplete topology displacement integrals.\n");
    return false;
  }

  for (const auto& assignment : collection->second.element_base_material)
  {
    if (element_integrals.count(assignment.first) == 0)
    {
      PRINT_ERROR("Missing displacement integral for element %d.\n", assignment.first);
      return false;
    }
  }

  const auto factors = collection->second.iteration_factors.find(iteration_id);

  if (factors == collection->second.iteration_factors.end())
  {
    PRINT_ERROR("Topology iteration stiffness factors are missing.\n");
    return false;
  }

  return topology.evaluate_displacement(topology_id, iteration_id, value, node_id, element_integrals, factors->second, collection->second.iteration_material_caps.count(iteration_id) != 0);
}
