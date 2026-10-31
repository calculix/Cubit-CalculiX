#include "CoreTopologyElementHistoryOutputPreparation.hpp"
#include "CoreTopologyHistoryOutputAssignment.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cstddef>
#include <vector>

bool CoreTopologyElementHistoryOutputPreparation::ensure(CoreHistoryOutputs& outputs, CoreSteps& steps, int topology_id, int step_id, int block_id, const std::string& key, const std::function<bool(const std::string&)>& execute_command)
{
  const auto key_it = std::find(outputs.element_keys.begin(), outputs.element_keys.end(), key);

  if (key_it == outputs.element_keys.end())
  {
    PRINT_ERROR("Unknown element history output key '%s'.\n", key.c_str());
    return false;
  }

  const std::size_t key_column = 7 + static_cast<std::size_t>(key_it - outputs.element_keys.begin());
  const bool tensor_output = key == "S" || key == "ME";
  std::vector<int> assigned_ids;

  if (!CoreTopologyHistoryOutputAssignment::collect_ids(steps, step_id, assigned_ids))
  {
    return false;
  }

  const auto element_index = [&](int output_id) -> int
  {
    const int index = outputs.get_outputs_data_id_from_output_id(output_id);

    if (index < 0)
    {
      return -1;
    }

    const auto& output = outputs.outputs_data[index];

    if (output.size() < 4 || output[2] != 2)
    {
      return -1;
    }

    const int element = outputs.get_element_data_id_from_element_id(output[3]);

    if (element < 0)
    {
      return -1;
    }

    const auto& data = outputs.element_data[element];

    if (data.size() <= key_column || data[1] != std::to_string(block_id))
    {
      return -1;
    }

    return element;
  };

  // Prefer an output that already provides individual element values.
  const auto choose_output = [&](const std::vector<int>& ids) -> int
  {
    int fallback = -1;

    for (int id : ids)
    {
      const int index = element_index(id);

      if (index < 0)
      {
        continue;
      }

      if (fallback < 0)
      {
        fallback = id;
      }

      const auto& data = outputs.element_data[index];

      if (data[key_column] == key && data[4] != "ONLY")
      {
        return id;
      }
    }

    return fallback;
  };

  int output_id = choose_output(assigned_ids);
  std::vector<int> existing_ids;

  for (const auto& output : outputs.outputs_data)
  {
    if (output.size() < 4)
    {
      PRINT_ERROR("Invalid history output data.\n");
      return false;
    }

    existing_ids.push_back(output[0]);
  }

  if (output_id < 0)
  {
    output_id = choose_output(existing_ids);
  }

  bool created = false;

  if (output_id < 0)
  {
    const std::string name = "Topology_" + std::to_string(topology_id) + "_Block_" + std::to_string(block_id) + "_" + key;

    if (!execute_command("ccx create historyoutput name " + name + " element"))
    {
      return false;
    }

    // Discover the created ID and verify creation actually happened.
    for (const auto& output : outputs.outputs_data)
    {
      if (output.empty() || std::find(existing_ids.begin(), existing_ids.end(), output[0]) != existing_ids.end())
      {
        continue;
      }

      if (output_id >= 0)
      {
        PRINT_ERROR("Unexpected history output creation result.\n");
        return false;
      }

      output_id = output[0];
    }

    if (output_id < 0)
    {
      PRINT_ERROR("Automatic history output creation failed.\n");
      return false;
    }

    created = true;

    std::string command = "ccx modify historyoutput " + std::to_string(output_id) + " element block " + std::to_string(block_id) + " totals_no key_on " + key;

    if (tensor_output)
    {
      command += " global_yes frequency 1";
    }

    if (!execute_command(command))
    {
      return false;
    }
  }

  int index = element_index(output_id);

  if (index < 0)
  {
    PRINT_ERROR("Automatic history output configuration failed.\n");
    return false;
  }

  const bool missing_key = outputs.element_data[index][key_column] != key;
  const bool totals_only = outputs.element_data[index][4] == "ONLY";
  const bool needs_tensor_settings = tensor_output && (outputs.element_data[index][5] != "YES" || outputs.element_data[index][2] != "1");

  if (missing_key || totals_only || needs_tensor_settings)
  {
    std::string command = "ccx modify historyoutput " + std::to_string(output_id) + " element";

    if (totals_only)
    {
      command += " totals_yes";
    }

    command += " key_on " + key;

    if (tensor_output)
    {
      command += " global_yes frequency 1";
    }

    if (!execute_command(command))
    {
      return false;
    }
  }

  index = element_index(output_id);

  if (index < 0 || outputs.element_data[index][key_column] != key || outputs.element_data[index][4] == "ONLY")
  {
    PRINT_ERROR("History output %d does not provide element %s values.\n", output_id, key.c_str());
    return false;
  }

  if (tensor_output && (outputs.element_data[index][5] != "YES" || outputs.element_data[index][2] != "1"))
  {
    PRINT_ERROR("History output %d does not provide global element %s values at every increment.\n", output_id, key.c_str());
    return false;
  }

  bool needs_assignment = false;

  if (!CoreTopologyHistoryOutputAssignment::ensure_assigned(steps, step_id, output_id, assigned_ids, execute_command, needs_assignment))
  {
    return false;
  }

  if (created)
  {
    PRINT_INFO("Automatically created history output %d for block %d with key %s.\n", output_id, block_id, key.c_str());
  }
  else if (missing_key || totals_only || needs_tensor_settings)
  {
    PRINT_INFO("Automatically configured history output %d to provide element %s values.\n", output_id, key.c_str());
  }

  if (needs_assignment)
  {
    PRINT_INFO("Automatically assigned history output %d to step %d.\n", output_id, step_id);
  }

  return true;
}
