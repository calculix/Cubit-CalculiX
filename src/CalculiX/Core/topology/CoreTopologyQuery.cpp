#include "CoreTopologyQuery.hpp"
#include "CoreTopology.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "CubitMessage.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>

int CoreTopologyQuery::get_iteration_job_id(const CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id)
{
  if (iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology iteration query.\n");
    return -1;
  }

  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return -1;
  }

  const auto iteration = entry->second.iteration_jobs.find(iteration_id);

  if (iteration == entry->second.iteration_jobs.end())
  {
    PRINT_ERROR("Topology %d iteration %d has no job.\n", topology_id, iteration_id);
    return -1;
  }

  const int job_id = iteration->second;

  if (jobs.get_jobs_data_id_from_job_id(job_id) < 0)
  {
    PRINT_ERROR("Topology iteration job %d no longer exists.\n", job_id);
    return -1;
  }

  return job_id;
}

int CoreTopologyQuery::get_iteration_adjoint_job_id(const CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id)
{
  if (iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology adjoint job query.\n");
    return -1;
  }

  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return -1;
  }

  const auto job = entry->second.iteration_adjoint_jobs.find(iteration_id);

  if (job == entry->second.iteration_adjoint_jobs.end() || jobs.get_jobs_data_id_from_job_id(job->second) < 0)
  {
    PRINT_ERROR("Topology %d iteration %d has no available adjoint job.\n", topology_id, iteration_id);
    return -1;
  }

  return job->second;
}

int CoreTopologyQuery::get_active_iteration_job(const CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id)
{
  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    return -1;
  }

  for (const auto* associations : {&entry->second.iteration_jobs, &entry->second.iteration_adjoint_jobs})
  {
    const auto job = associations->find(iteration_id);

    if (job == associations->end())
    {
      continue;
    }

    const int status = jobs.get_job_status(job->second);

    if (status == 1 || status == 5)
    {
      return job->second;
    }
  }

  return -1;
}

std::vector<std::vector<std::string>> CoreTopologyQuery::get_iteration_data(const CoreTopology& topology, CoreJobs& jobs, int topology_id)
{
  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return {};
  }

  const auto& configuration = entry->second;
  std::vector<std::vector<std::string>> rows;

  const auto format_value = [](double value)
  {
    std::ostringstream stream;
    stream << std::scientific << std::setprecision(15) << value;
    return stream.str();
  };

  for (const auto& iteration : configuration.iteration_jobs)
  {
    std::string objective;
    std::string fraction;
    std::string density_change;
    std::string objective_change;
    std::string converged;
    std::string displacement;
    std::string displacement_node;
    std::string displacement_nodeset;
    std::string displacement_component;

    const auto evaluation = configuration.iteration_evaluations.find(iteration.first);

    if (evaluation != configuration.iteration_evaluations.end())
    {
      objective = format_value(evaluation->second.value);

      if (evaluation->second.displacement_available)
      {
        displacement = format_value(evaluation->second.displacement_value);
        displacement_node = std::to_string(evaluation->second.displacement_node);
        displacement_nodeset = std::to_string(evaluation->second.displacement_nodeset);
        displacement_component = std::to_string(evaluation->second.displacement_component);
      }

      if (evaluation->second.convergence_checked)
      {
        density_change = format_value(evaluation->second.maximum_density_change);
        objective_change = format_value(evaluation->second.relative_objective_change);
        converged = evaluation->second.converged ? "1" : "0";
      }
    }

    const auto design = configuration.iteration_density_designs.find(iteration.first);

    if (design != configuration.iteration_density_designs.end() && design->second.volume.available)
    {
      fraction = format_value(design->second.volume.volume_fraction);
    }

    rows.push_back({
        std::to_string(iteration.first),
        std::to_string(iteration.second),
        std::to_string(jobs.get_job_status(iteration.second)),
        objective,
        fraction,
        configuration.selected_iteration == iteration.first ? "1" : "0",
        density_change,
        objective_change,
        converged,
        displacement,
        displacement_node,
        displacement_nodeset,
        displacement_component
    });
  }

  return rows;
}

std::vector<std::vector<double>> CoreTopologyQuery::get_iteration_element_data(const CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs, int topology_id, int iteration_id)
{
  if (iteration_id < -1)
  {
    PRINT_ERROR("Invalid topology element-data request.\n");
    return {};
  }

  const auto entry = topology.topology_cases.find(topology_id);
  const auto collection = materials.topology_material_data.find(topology_id);

  if (entry == topology.topology_cases.end() || collection == materials.topology_material_data.end())
  {
    PRINT_ERROR("Topology %d has no prepared design data.\n", topology_id);
    return {};
  }

  const auto& configuration = entry->second;

  if (iteration_id == -1)
  {
    iteration_id = configuration.selected_iteration;
  }

  if (iteration_id < 1)
  {
    PRINT_ERROR("Topology %d has no selected iteration.\n", topology_id);
    return {};
  }

  const int job_id = get_iteration_job_id(topology, jobs, topology_id, iteration_id);

  if (job_id <= 0 || jobs.get_job_status(job_id) != 2 || configuration.iteration_evaluations.count(iteration_id) == 0)
  {
    PRINT_ERROR("Topology iteration %d is not a completed evaluation.\n", iteration_id);
    return {};
  }

  const auto design = configuration.iteration_density_designs.find(iteration_id);
  const auto factors = collection->second.iteration_factors.find(iteration_id);

  if (design == configuration.iteration_density_designs.end() || factors == collection->second.iteration_factors.end())
  {
    PRINT_ERROR("Missing topology iteration densities or factors.\n");
    return {};
  }

  const auto& densities = design->second.element_densities;
  const auto& preserved = configuration.element_preserved;
  const auto& assignments = collection->second.element_base_material;

  if (densities.empty() || densities.size() != factors->second.size() || densities.size() != preserved.size() || densities.size() != assignments.size())
  {
    PRINT_ERROR("Incomplete topology iteration element data.\n");
    return {};
  }

  std::vector<std::vector<double>> rows;
  rows.reserve(densities.size());

  for (const auto& density : densities)
  {
    const auto factor = factors->second.find(density.first);
    const auto flag = preserved.find(density.first);

    if (density.first <= 0 ||
        assignments.count(density.first) == 0 ||
        factor == factors->second.end() ||
        flag == preserved.end() ||
        !std::isfinite(density.second) ||
        density.second <= 0.0 || density.second > 1.0 ||
        !std::isfinite(factor->second) ||
        factor->second <= 0.0 || factor->second > 1.0)
    {
      PRINT_ERROR("Invalid topology data for element %d.\n", density.first);
      return {};
    }

    rows.push_back({
        static_cast<double>(density.first),
        density.second,
        factor->second,
        flag->second ? 1.0 : 0.0
    });
  }

  return rows;
}

bool CoreTopologyQuery::get_job_context(const CoreTopology& topology, int job_id, int& topology_id, int& iteration_id)
{
  topology_id = -1;
  iteration_id = -1;

  if (job_id <= 0)
  {
    PRINT_ERROR("Invalid job topology context query.\n");
    return false;
  }

  for (const auto& configuration : topology.topology_cases)
  {
    for (const auto* associations : {&configuration.second.iteration_jobs, &configuration.second.iteration_adjoint_jobs})
    {
      for (const auto& iteration : *associations)
      {
        if (iteration.second != job_id)
        {
          continue;
        }

        if (topology_id != -1 || configuration.first <= 0 || iteration.first < 1)
        {
          PRINT_ERROR("Invalid topology association for job %d.\n", job_id);
          return false;
        }

        topology_id = configuration.first;
        iteration_id = iteration.first;
      }
    }
  }

  // Ordinary jobs return true with both IDs set to -1.
  return true;
}

int CoreTopologyQuery::get_adjoint_job_source(const CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id)
{
  if (iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology adjoint job request.\n");
    return -1;
  }

  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return -1;
  }

  if (entry->second.iteration_adjoint_jobs.count(iteration_id) != 0)
  {
    PRINT_ERROR("Topology %d iteration %d already has an adjoint job.\n", topology_id, iteration_id);
    return -1;
  }

  const int physical_job = get_iteration_job_id(topology, jobs, topology_id, iteration_id);

  if (physical_job <= 0 || jobs.get_job_status(physical_job) != 2)
  {
    PRINT_ERROR("A completed physical job is required before creating an adjoint job.\n");
    return -1;
  }

  return physical_job;
}

bool CoreTopologyQuery::validate_selected_iterations(const CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs)
{
  for (const auto& entry : topology.topology_cases)
  {
    const int iteration = entry.second.selected_iteration;

    if (iteration >= 0)
    {
      if (get_iteration_element_data(topology, materials, jobs, entry.first, iteration).empty())
      {
        PRINT_ERROR("Unable to restore selected iteration for topology %d.\n", entry.first);
        return false;
      }
    }
  }

  return true;
}
