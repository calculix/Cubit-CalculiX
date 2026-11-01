#include "CoreTopologyFieldOutputPreparation.hpp"
#include "Core/steps/CoreFieldOutputs.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cstddef>
#include <set>

bool CoreTopologyFieldOutputPreparation::ensure_displacement(CoreFieldOutputs& outputs, CoreSteps& steps, int topology_id, int step_id, const std::function<bool(const std::string&)>& command)
{
  const auto key = std::find(outputs.node_keys.begin(), outputs.node_keys.end(), "U");

  if (key == outputs.node_keys.end())
  {
    PRINT_ERROR("Displacement field output key U is unavailable.\n");
    return false;
  }

  const std::size_t key_column = 11 + static_cast<std::size_t>(key - outputs.node_keys.begin());
  const int step_index = steps.get_steps_data_id_from_step_id(step_id);

  if (step_index < 0 || steps.steps_data[step_index].size() < 9)
  {
    PRINT_ERROR("Missing or invalid step %d.\n", step_id);
    return false;
  }

  std::set<int> assigned_ids;

  for (int index : steps.get_fieldoutput_data_ids_from_fieldoutputs_id(steps.steps_data[step_index][8]))
  {
    if (index < 0 || static_cast<std::size_t>(index) >= steps.fieldoutputs_data.size() || steps.fieldoutputs_data[index].size() < 2)
    {
      PRINT_ERROR("Invalid field output assignment for step %d.\n", step_id);
      return false;
    }

    assigned_ids.insert(steps.fieldoutputs_data[index][1]);
  }

  const auto compatible_node = [&](int output_id) -> int
  {
    const int index = outputs.get_outputs_data_id_from_output_id(output_id);

    if (index < 0)
    {
      return -1;
    }

    const auto& output = outputs.outputs_data[index];

    if (output.size() < 4 || output[2] != 1)
    {
      return -1;
    }

    const int node_index = outputs.get_node_data_id_from_node_id(output[3]);

    if (node_index < 0)
    {
      return -1;
    }

    const auto& node = outputs.node_data[node_index];

    if (node.size() < 11 + outputs.node_keys.size())
    {
      return -1;
    }

    if (!node[1].empty() || !node[3].empty() || !node[8].empty() || !node[9].empty() || !node[10].empty())
    {
      return -1;
    }

    if ((!node[5].empty() && node[5] != "YES") || (!node[6].empty() && node[6] != "OUTPUT=3D"))
    {
      return -1;
    }

    return node_index;
  };

  int output_id = -1;

  // Prefer a complete output already assigned to this step.
  for (int candidate : assigned_ids)
  {
    const int node_index = compatible_node(candidate);

    if (node_index >= 0)
    {
      const auto& node = outputs.node_data[node_index];

      if (node[5] == "YES" && node[2] == "1" && node[key_column] == "U")
      {
        return true;
      }

      if (output_id < 0)
      {
        output_id = candidate;
      }
    }
  }

  std::set<int> existing_ids;

  for (const auto& output : outputs.outputs_data)
  {
    if (output.size() >= 4)
    {
      existing_ids.insert(output[0]);

      if (output_id < 0 && compatible_node(output[0]) >= 0)
      {
        output_id = output[0];
      }
    }
  }

  if (output_id < 0)
  {
    const std::string name = "Topology_" + std::to_string(topology_id) + "_Displacement";

    if (!command("ccx create fieldoutput name " + name + " node"))
    {
      return false;
    }

    for (const auto& output : outputs.outputs_data)
    {
      if (output.size() >= 4 && existing_ids.count(output[0]) == 0 && compatible_node(output[0]) >= 0)
      {
        output_id = output[0];
        break;
      }
    }

    if (output_id < 0)
    {
      PRINT_ERROR("Unable to identify the created topology field output.\n");
      return false;
    }

    PRINT_INFO("Automatically created field output %d for topology displacement results.\n", output_id);
  }

  const int node_index = compatible_node(output_id);

  if (node_index < 0)
  {
    PRINT_ERROR("Invalid topology displacement field output.\n");
    return false;
  }

  const auto& node = outputs.node_data[node_index];
  const bool needs_configuration = node[5] != "YES" || node[2] != "1" || node[key_column] != "U";

  if (needs_configuration)
  {
    if (!command("ccx modify fieldoutput " + std::to_string(output_id) + " node global_yes frequency 1 key_on U"))
    {
      return false;
    }

    PRINT_INFO("Automatically configured field output %d for global displacement results.\n", output_id);
  }

  const int configured_index = compatible_node(output_id);

  if (configured_index < 0 || outputs.node_data[configured_index][5] != "YES" || outputs.node_data[configured_index][2] != "1" || outputs.node_data[configured_index][key_column] != "U")
  {
    PRINT_ERROR("Topology displacement field output configuration failed.\n");
    return false;
  }

  if (assigned_ids.count(output_id) == 0)
  {
    if (!command("ccx step " + std::to_string(step_id) + " add fieldoutput " + std::to_string(output_id)))
    {
      return false;
    }

    const int updated_step = steps.get_steps_data_id_from_step_id(step_id);
    bool assigned = false;

    if (updated_step >= 0 && steps.steps_data[updated_step].size() >= 9)
    {
      for (int index : steps.get_fieldoutput_data_ids_from_fieldoutputs_id(steps.steps_data[updated_step][8]))
      {
        if (index >= 0 && static_cast<std::size_t>(index) < steps.fieldoutputs_data.size() && steps.fieldoutputs_data[index].size() >= 2 && steps.fieldoutputs_data[index][1] == output_id)
        {
          assigned = true;
          break;
        }
      }
    }

    if (!assigned)
    {
      PRINT_ERROR("Topology displacement field output assignment failed.\n");
      return false;
    }

    PRINT_INFO("Automatically assigned field output %d to step %d.\n", output_id, step_id);
  }

  return true;
}
