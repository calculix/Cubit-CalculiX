#include "CoreTopologyResults.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "CoreTopologyQuery.hpp"
#include "CoreTopologyGeometry.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/results/CoreResults.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <set>
#include <string>

bool CoreTopologyResults::read_final_displacements(CoreResults& results, int job_id, std::map<int, std::array<double, 3>>& displacements)
{
  displacements.clear();

  const int frd_index = results.get_frd_data_id_from_job_id(job_id);

  if (frd_index < 0)
  {
    PRINT_ERROR("FRD results for job %d are missing.\n", job_id);
    return false;
  }

  const auto& frd = results.frd_data[frd_index];

  if (frd.current_total_increment < 1)
  {
    PRINT_ERROR("Job %d has no completed result increment.\n", job_id);
    return false;
  }

  std::map<int, std::array<double, 3>> measured;
  const std::array<std::string, 3> component_names = {"D1", "D2", "D3"};

  for (const auto& block : frd.result_blocks)
  {
    if (block.size() < 7)
    {
      PRINT_ERROR("Invalid FRD result block for job %d.\n", job_id);
      return false;
    }

    if (block[3] != frd.current_total_increment)
    {
      continue;
    }

    const int type = block[5];

    if (type < 0 || static_cast<std::size_t>(type) >= frd.result_block_type.size())
    {
      PRINT_ERROR("Invalid FRD result type for job %d.\n", job_id);
      return false;
    }

    if (frd.result_block_type[type] != "DISP")
    {
      continue;
    }

    const int data_id = block[6];

    if (data_id < 0 || static_cast<std::size_t>(data_id) >= frd.result_block_data.size() || static_cast<std::size_t>(data_id) >= frd.result_block_node_data.size() || static_cast<std::size_t>(data_id) >= frd.result_block_components.size())
    {
      PRINT_ERROR("Invalid displacement result storage for job %d.\n", job_id);
      return false;
    }

    const auto& values = frd.result_block_data[data_id];
    const auto& mapping = frd.result_block_node_data[data_id];
    const auto& components = frd.result_block_components[data_id];

    if (values.empty() || mapping.size() != values.size())
    {
      PRINT_ERROR("Incomplete displacement node mapping for job %d.\n", job_id);
      return false;
    }

    std::array<std::size_t, 3> component_ids{};

    for (std::size_t axis = 0; axis < component_names.size(); ++axis)
    {
      const auto component = std::find(components.begin(), components.end(), component_names[axis]);

      if (component == components.end())
      {
        PRINT_ERROR("Displacement component %s is missing for job %d.\n", component_names[axis].c_str(), job_id);
        return false;
      }

      component_ids[axis] = static_cast<std::size_t>(component - components.begin());
    }

    std::set<int> used_rows;

    for (const auto& record : mapping)
    {
      if (record.size() < 2 || record[0] <= 0 || record[1] < 0 || static_cast<std::size_t>(record[1]) >= values.size())
      {
        PRINT_ERROR("Invalid displacement node record for job %d.\n", job_id);
        return false;
      }

      if (!used_rows.insert(record[1]).second)
      {
        PRINT_ERROR("Duplicate displacement data mapping for job %d.\n", job_id);
        return false;
      }

      const auto& row = values[record[1]];
      std::array<double, 3> displacement{};

      for (std::size_t axis = 0; axis < displacement.size(); ++axis)
      {
        if (component_ids[axis] >= row.size() || !std::isfinite(row[component_ids[axis]]))
        {
          PRINT_ERROR("Invalid displacement for node %d in job %d.\n", record[0], job_id);
          return false;
        }

        displacement[axis] = row[component_ids[axis]];
      }

      if (!measured.emplace(record[0], displacement).second)
      {
        PRINT_ERROR("Duplicate displacement result for node %d in job %d.\n", record[0], job_id);
        return false;
      }
    }
  }

  if (measured.empty())
  {
    PRINT_ERROR("Final-increment displacement results are missing for job %d.\n", job_id);
    return false;
  }

  displacements.swap(measured);
  return true;
}

bool CoreTopologyResults::read_job_displacements(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, const CoreMaterials& materials, CoreBlocks* blocks, int job_id, std::map<int, std::array<double, 3>>& displacements)
{
  displacements.clear();

  if (job_id <= 0 || jobs.get_job_status(job_id) != 2)
  {
    PRINT_ERROR("Topology displacement results require a completed job.\n");
    return false;
  }

  int topology_id = -1;
  int iteration_id = -1;

  if (!CoreTopologyQuery::get_job_context(topology, job_id, topology_id, iteration_id))
  {
    return false;
  }

  if (topology_id <= 0 || iteration_id < 1)
  {
    PRINT_ERROR("Job %d is not associated with a topology iteration.\n", job_id);
    return false;
  }

  const auto collection = materials.topology_material_data.find(topology_id);

  if (collection == materials.topology_material_data.end() || collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Prepared elements for topology %d are missing.\n", topology_id);
    return false;
  }

  std::map<int, std::array<double, 3>> measured;

  if (!read_final_displacements(results, job_id, measured))
  {
    return false;
  }

  if (!blocks)
  {
    PRINT_ERROR("Block core is not initialized.\n");
    return false;
  }

  std::set<int> required_nodes;

  if (!CoreTopologyGeometry::collect_element_nodes(*blocks, collection->second.element_base_material, required_nodes))
  {
    return false;
  }

  for (int node_id : required_nodes)
  {
    if (measured.count(node_id) == 0)
    {
      PRINT_ERROR("Job %d is missing displacement results for topology node %d.\n", job_id, node_id);
      return false;
    }
  }

  displacements.swap(measured);
  return true;
}

bool CoreTopologyResults::read_final_element_energies(CoreResults& results, int job_id, const std::map<int, int>& element_materials, std::map<int, double>& element_energies)
{
  element_energies.clear();

  const int dat_index = results.get_dat_data_id_from_job_id(job_id);

  if (dat_index < 0)
  {
    PRINT_ERROR("DAT results for job %d are missing.\n", job_id);
    return false;
  }

  auto& dat = results.dat_data[dat_index];

  // The parser updates these when reading STEP/INCREMENT headers.
  const int last_step = dat.current_step;
  const int last_increment = dat.current_increment;

  if (last_step <= 0 || last_increment <= 0)
  {
    PRINT_ERROR("Final solver step/increment is missing in job %d.\n", job_id);
    return false;
  }

  int energy_type = -1;

  for (std::size_t i = 0; i < dat.result_block_type.size(); ++i)
  {
    if (dat.erase_whitespace(dat.result_block_type[i]) == "internalenergy")
    {
      energy_type = static_cast<int>(i);
      break;
    }
  }

  if (energy_type < 0)
  {
    PRINT_ERROR("Job %d has no element internal energy results.\n", job_id);
    return false;
  }

  const int energy_component = dat.get_result_block_component_id(energy_type, "energy");

  if (energy_component < 0)
  {
    PRINT_ERROR("Internal energy component is missing.\n");
    return false;
  }

  std::map<int, double> collected;

  // Read each result block once.
  for (const auto& block : dat.result_blocks)
  {
    if (block.size() < 7 || block[1] < 0 || static_cast<std::size_t>(block[1]) >= dat.total_times.size())
    {
      PRINT_ERROR("Invalid DAT result block.\n");
      return false;
    }

    if (block[2] != energy_type || block[5] != last_step || block[6] != last_increment)
    {
      continue;
    }

    const int data_id = block[4];

    if (data_id < 0 || static_cast<std::size_t>(data_id) >= dat.result_block_data.size() || static_cast<std::size_t>(data_id) >= dat.result_block_c1_data.size())
    {
      PRINT_ERROR("Invalid DAT energy data reference.\n");
      return false;
    }

    for (const auto& record : dat.result_block_c1_data[data_id])
    {
      if (record.size() < 3)
      {
        PRINT_ERROR("Invalid DAT element record.\n");
        return false;
      }

      // Whole-element results only.
      if (record[2] != 3 || element_materials.count(record[0]) == 0)
      {
        continue;
      }

      const int row_id = record[1];

      if (row_id < 0 || static_cast<std::size_t>(row_id) >= dat.result_block_data[data_id].size())
      {
        PRINT_ERROR("Invalid DAT energy row reference.\n");
        return false;
      }

      const auto& values = dat.result_block_data[data_id][row_id];

      if (static_cast<std::size_t>(energy_component) >= values.size())
      {
        PRINT_ERROR("Missing energy value for element %d.\n", record[0]);
        return false;
      }

      const double energy = values[energy_component];

      if (!std::isfinite(energy))
      {
        PRINT_ERROR("Non-finite energy for element %d.\n", record[0]);
        return false;
      }

      if (!collected.emplace(record[0], energy).second)
      {
        PRINT_ERROR("Multiple energy values for element %d in final step %d, increment %d.\n", record[0], last_step, last_increment);
        return false;
      }
    }
  }

  for (const auto& element : element_materials)
  {
    if (collected.count(element.first) == 0)
    {
      PRINT_ERROR("Missing energy for topology element %d in final step %d, increment %d.\n", element.first, last_step, last_increment);
      return false;
    }
  }

  element_energies.swap(collected);
  return true;
}

bool CoreTopologyResults::select_maximum_displacement(const std::map<int, std::array<double, 3>>& measured, int component, double& value, int& node_id, std::array<double, 3>* critical_displacement)
{
  value = 0.0;
  node_id = -1;

  if (critical_displacement)
  {
    critical_displacement->fill(0.0);
  }

  if (component < 0 || component > 3 || measured.empty())
  {
    PRINT_ERROR("Invalid topology displacement request.\n");
    return false;
  }

  for (const auto& measurement : measured)
  {
    const auto& displacement = measurement.second;
    const double measured_value = component == 0 ? std::hypot(displacement[0], displacement[1], displacement[2]) : std::abs(displacement[component - 1]);

    if (node_id < 0 || measured_value > value)
    {
      value = measured_value;
      node_id = measurement.first;

      if (critical_displacement)
      {
        *critical_displacement = displacement;
      }
    }
  }

  return true;
}

bool CoreTopologyResults::read_iteration_energies(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, const CoreMaterials& materials, int topology_id, int iteration_id, std::map<int, double>& element_energies)
{
  element_energies.clear();

  const int job_id = CoreTopologyQuery::get_iteration_job_id(topology, jobs, topology_id, iteration_id);

  if (job_id <= 0 || jobs.get_job_status(job_id) != 2)
  {
    PRINT_ERROR("Topology iteration has no successfully completed job.\n");
    return false;
  }

  const auto collection = materials.topology_material_data.find(topology_id);

  if (collection == materials.topology_material_data.end() || collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Topology material assignments are missing.\n");
    return false;
  }

  return read_final_element_energies(results, job_id, collection->second.element_base_material, element_energies);
}
