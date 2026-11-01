#include "CoreTopologyJobs.hpp"
#include "Core/sections/CoreSections.hpp"
#include "CoreTopologyOptimization.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyQuery.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "CubitMessage.hpp"
#include <initializer_list>
#include <string>

bool CoreTopologyJobs::register_adjoint_job(CoreTopology& topology, CoreJobs& jobs, int topology_id, int iteration_id, int& job_id)
{
  job_id = -1;

  const int physical_job = CoreTopologyQuery::get_adjoint_job_source(topology, jobs, topology_id, iteration_id);

  if (physical_job <= 0)
  {
    return false;
  }

  const int physical_index = jobs.get_jobs_data_id_from_job_id(physical_job);

  if (physical_index < 0 || jobs.jobs_data[physical_index].size() < 2)
  {
    PRINT_ERROR("Physical job data is missing.\n");
    return false;
  }

  const std::string name = jobs.jobs_data[physical_index][1] + "_adjoint";

  for (const auto& job : jobs.jobs_data)
  {
    if (job.size() > 1 && job[1] == name)
    {
      PRINT_ERROR("Job name '%s' already exists.\n", name.c_str());
      return false;
    }
  }

  const auto entry = topology.topology_cases.find(topology_id);

  if (!jobs.create_job({name, ""}, job_id))
  {
    return false;
  }

  // Register ownership before running or exporting.
  entry->second.iteration_adjoint_jobs.emplace(iteration_id, job_id);
  return true;
}

int CoreTopologyJobs::create_iteration_job(CoreTopology& topology, const CoreMaterials& materials, CoreJobs& jobs, int topology_id, int iteration_id, const std::string& job_name, const std::function<bool()>& validate_model)
{
  if (iteration_id < 1 || job_name.empty())
  {
    PRINT_ERROR("Invalid topology iteration job request.\n");
    return -1;
  }

  const auto entry = topology.topology_cases.find(topology_id);
  const auto collection = materials.topology_material_data.find(topology_id);

  if (entry == topology.topology_cases.end() || collection == materials.topology_material_data.end())
  {
    PRINT_ERROR("Topology %d is not prepared.\n", topology_id);
    return -1;
  }

  auto& configuration = entry->second;

  if ((configuration.objective == TopologyObjective::Compliance ||
       configuration.objective == TopologyObjective::Displacement) &&
      !validate_model())
  {
    return -1;
  }

  if (collection->second.iteration_factors.count(iteration_id) == 0)
  {
    PRINT_ERROR("Factors for topology %d iteration %d are missing.\n", topology_id, iteration_id);
    return -1;
  }

  if (configuration.iteration_jobs.count(iteration_id) != 0)
  {
    PRINT_ERROR("Topology %d iteration %d already has a job.\n", topology_id, iteration_id);
    return -1;
  }

  for (const auto& iteration : configuration.iteration_jobs)
  {
    const int status = jobs.get_job_status(iteration.second);

    if (status == 1 || status == 5)
    {
      PRINT_ERROR("Topology %d already has an active job.\n", topology_id);
      return -1;
    }
  }

  for (const auto& job : jobs.jobs_data)
  {
    if (job.size() > 1 && job[1] == job_name)
    {
      PRINT_ERROR("Job name '%s' already exists.\n", job_name.c_str());
      return -1;
    }
  }

  int job_id = -1;

  if (!jobs.create_job({job_name, ""}, job_id))
  {
    return -1;
  }

  // Register before launch so export can retrieve the iteration context.
  configuration.iteration_jobs.emplace(iteration_id, job_id);
  return job_id;
}

bool CoreTopologyJobs::reset_run(CoreTopology& topology, CoreJobs& jobs, CoreMaterials* materials, CoreSections* sections, CoreTopologyOptimization* optimization, int topology_id)
{
  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (entry->second.run_state == TopologyRunState::Running)
  {
    PRINT_ERROR("Topology %d is running.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;

  // Check every associated job before deleting any.
  for (const auto* associations : {&configuration.iteration_jobs, &configuration.iteration_adjoint_jobs})
  {
    for (const auto& iteration : *associations)
    {
      const int status = jobs.get_job_status(iteration.second);

      if (status == 1 || status == 5)
      {
        PRINT_ERROR("Topology %d has an active associated job.\n", topology_id);
        return false;
      }
    }
  }

  for (auto* associations : {&configuration.iteration_adjoint_jobs, &configuration.iteration_jobs})
  {
    for (auto iterator = associations->begin(); iterator != associations->end();)
    {
      const int job_id = iterator->second;

      if (jobs.get_jobs_data_id_from_job_id(job_id) >= 0 && !jobs.delete_job(job_id))
      {
        PRINT_ERROR("Failed to delete topology job %d.\n", job_id);
        return false;
      }

      iterator = associations->erase(iterator);
    }
  }

  configuration.selected_iteration = -1;
  configuration.element_preserved.clear();
  configuration.iteration_element_energies.clear();
  configuration.iteration_total_energies.clear();
  configuration.iteration_evaluations.clear();
  configuration.iteration_density_designs.clear();
  configuration.run_state = TopologyRunState::NotRun;

  if (materials)
  {
    materials->topology_material_data.erase(topology_id);
  }

  if (sections)
  {
    sections->topology_solid_sections.erase(topology_id);
  }

  if (optimization)
  {
    optimization->forget_topology_run(topology_id);
  }

  return true;
}

bool CoreTopologyJobs::delete_topology(CoreTopology& topology, CoreJobs& jobs, CoreMaterials* materials, CoreSections* sections, CoreTopologyOptimization* optimization, int topology_id)
{
  if (!reset_run(topology, jobs, materials, sections, optimization, topology_id))
  {
    return false;
  }

  return topology.delete_topology(topology_id);
}

void CoreTopologyJobs::invalidate_saved_processes(const CoreTopology& topology, CoreJobs& jobs)
{
  // Saved process IDs do not identify solvers launched by this session.
  for (const auto& configuration : topology.topology_cases)
  {
    for (const auto* associations : {&configuration.second.iteration_jobs, &configuration.second.iteration_adjoint_jobs})
    {
      for (const auto& iteration : *associations)
      {
        const int index = jobs.get_jobs_data_id_from_job_id(iteration.second);

        if (index >= 0 && (jobs.jobs_data[index][3] == "1" || jobs.jobs_data[index][3] == "5"))
        {
          jobs.jobs_data[index][3] = "4";
          jobs.jobs_data[index][4] = "-1";
        }
      }
    }
  }
}
