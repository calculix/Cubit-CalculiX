#include "CoreTopologyTensorResults.hpp"
#include "CoreTopologyQuery.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/results/CoreResults.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <cstddef>
#include <limits>

bool CoreTopologyTensorResults::read_final(CoreResults& results, int job_id, const std::string& key, const std::map<int, int>& required_elements, std::map<int, std::map<int, std::array<double, 6>>>& tensors)
{
  tensors.clear();

  if (key != "S" && key != "ME")
  {
    PRINT_ERROR("Topology tensor reader supports S and ME only.\n");
    return false;
  }

  const int dat_index = results.get_dat_data_id_from_job_id(job_id);

  if (dat_index < 0)
  {
    PRINT_ERROR("DAT results for job %d are missing.\n", job_id);
    return false;
  }

  auto& dat = results.dat_data[dat_index];
  const int final_step = dat.current_step;
  const int final_increment = dat.current_increment;

  if (final_step <= 0 || final_increment <= 0)
  {
    PRINT_ERROR("Final solver step/increment is missing for job %d.\n", job_id);
    return false;
  }

  const std::string expected_type = key == "S" ? "stresses" : "mechanicalstrains";
  const std::array<std::string, 6> component_names = key == "S" ? std::array<std::string, 6>{"sxx", "syy", "szz", "sxy", "sxz", "syz"} : std::array<std::string, 6>{"exx", "eyy", "ezz", "exy", "exz", "eyz"};

  std::map<int, std::map<int, std::array<double, 6>>> collected;

  for (const auto& block : dat.result_blocks)
  {
    if (block.size() < 7)
    {
      PRINT_ERROR("Invalid DAT result block for job %d.\n", job_id);
      return false;
    }

    if (block[5] != final_step || block[6] != final_increment)
    {
      continue;
    }

    const int type_id = block[2];

    if (type_id < 0 || static_cast<std::size_t>(type_id) >= dat.result_block_type.size())
    {
      PRINT_ERROR("Invalid DAT result type for job %d.\n", job_id);
      return false;
    }

    if (dat.erase_whitespace(dat.result_block_type[type_id]) != expected_type)
    {
      continue;
    }

    const int data_id = block[4];

    if (data_id < 0 || static_cast<std::size_t>(data_id) >= dat.result_block_data.size() || static_cast<std::size_t>(data_id) >= dat.result_block_c1_data.size() || static_cast<std::size_t>(data_id) >= dat.result_block_components.size())
    {
      PRINT_ERROR("Invalid DAT tensor storage for job %d.\n", job_id);
      return false;
    }

    const auto& components = dat.result_block_components[data_id];

    const auto find_column = [&](const std::string& name) -> int
    {
      int column = -1;

      for (std::size_t index = 0; index < components.size(); ++index)
      {
        if (dat.erase_whitespace(components[index]) == name)
        {
          if (column >= 0)
          {
            return -1;
          }

          column = static_cast<int>(index);
        }
      }

      return column;
    };

    const int element_column = find_column("elem");
    const int integration_column = find_column("integ.pnt.");

    if (element_column < 0 || integration_column < 0)
    {
      PRINT_ERROR("Missing element/integration-point columns for job %d.\n", job_id);
      return false;
    }

    std::array<int, 6> columns{};

    for (std::size_t component = 0; component < columns.size(); ++component)
    {
      columns[component] = find_column(component_names[component]);

      if (columns[component] < 0)
      {
        PRINT_ERROR("Missing or duplicate tensor component %s for job %d.\n", component_names[component].c_str(), job_id);
        return false;
      }
    }

    for (const auto& record : dat.result_block_c1_data[data_id])
    {
      if (record.size() < 3)
      {
        PRINT_ERROR("Invalid DAT tensor record for job %d.\n", job_id);
        return false;
      }

      if (required_elements.count(record[0]) == 0)
      {
        continue;
      }

      if (record[2] != 2 || record[1] < 0 || static_cast<std::size_t>(record[1]) >= dat.result_block_data[data_id].size())
      {
        PRINT_ERROR("Invalid integration-point record for element %d.\n", record[0]);
        return false;
      }

      const auto& row = dat.result_block_data[data_id][record[1]];

      if (static_cast<std::size_t>(element_column) >= row.size() || static_cast<std::size_t>(integration_column) >= row.size())
      {
        PRINT_ERROR("Missing element/integration-point values for job %d.\n", job_id);
        return false;
      }

      if (!std::isfinite(row[element_column]) || row[element_column] != static_cast<double>(record[0]))
      {
        PRINT_ERROR("Inconsistent DAT element mapping for job %d.\n", job_id);
        return false;
      }

      const double integration_value = row[integration_column];

      if (!std::isfinite(integration_value) || integration_value < 1.0 || integration_value > static_cast<double>((std::numeric_limits<int>::max)()) || std::trunc(integration_value) != integration_value)
      {
        PRINT_ERROR("Invalid integration-point ID for element %d.\n", record[0]);
        return false;
      }

      const int integration_id = static_cast<int>(integration_value);
      std::array<double, 6> tensor{};

      for (std::size_t component = 0; component < tensor.size(); ++component)
      {
        if (static_cast<std::size_t>(columns[component]) >= row.size() || !std::isfinite(row[columns[component]]))
        {
          PRINT_ERROR("Invalid %s tensor for element %d, integration point %d.\n", key.c_str(), record[0], integration_id);
          return false;
        }

        tensor[component] = row[columns[component]];
      }

      auto& element_tensors = collected[record[0]];
      const auto existing = element_tensors.find(integration_id);

      if (existing != element_tensors.end())
      {
        if (existing->second != tensor)
        {
          PRINT_ERROR("Conflicting %s results for element %d, integration point %d.\n", key.c_str(), record[0], integration_id);
          return false;
        }

        continue;
      }

      element_tensors.emplace(integration_id, tensor);
    }
  }

  for (const auto& element : required_elements)
  {
    const auto found = collected.find(element.first);

    if (found == collected.end() || found->second.empty())
    {
      PRINT_ERROR("Missing final-increment %s results for topology element %d in job %d.\n", key.c_str(), element.first, job_id);
      return false;
    }
  }

  tensors.swap(collected);
  return true;
}

bool CoreTopologyTensorResults::read_job(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, const CoreMaterials& materials, int job_id, const std::string& key, std::map<int, std::map<int, std::array<double, 6>>>& tensors)
{
  tensors.clear();

  if (key != "S" && key != "ME")
  {
    PRINT_ERROR("Topology tensor reader supports S and ME only.\n");
    return false;
  }

  if (job_id <= 0 || jobs.get_job_status(job_id) != 2)
  {
    PRINT_ERROR("Topology tensor results require a completed job.\n");
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

  return read_final(results, job_id, key, collection->second.element_base_material, tensors);
}
