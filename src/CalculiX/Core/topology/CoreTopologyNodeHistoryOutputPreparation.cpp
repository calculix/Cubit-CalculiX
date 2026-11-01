#include "CoreTopologyNodeHistoryOutputPreparation.hpp"
#include "CoreTopologyHistoryOutputAssignment.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cstddef>
#include <vector>

bool CoreTopologyNodeHistoryOutputPreparation::ensure(CoreHistoryOutputs& outputs, CoreSteps& steps, int topology_id, int step_id, int nodeset_id, const std::string& key, const std::function<bool(const std::string&)>& execute_command)
{
  const auto key_it = std::find(outputs.node_keys.begin(), outputs.node_keys.end(), key);

  if (key_it == outputs.node_keys.end())
  {
    PRINT_ERROR("Unknown node history output key '%s'.\n", key.c_str());
    return false;
  }

  const std::size_t key_column = 7 + static_cast<std::size_t>(key_it - outputs.node_keys.begin());
  std::vector<int> assigned_ids;

  if (!CoreTopologyHistoryOutputAssignment::collect_ids(steps, step_id, assigned_ids))
  {
    return false;
  }

  const auto node_index = [&](int output_id) -> int
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

    const int node = outputs.get_node_data_id_from_node_id(output[3]);

    if (node < 0)
    {
      return -1;
    }

    const auto& data = outputs.node_data[node];

    if (data.size() <= key_column || data[1] != std::to_string(nodeset_id))
    {
      return -1;
    }

    return node;
  };

  // Prefer an output that already provides individual nodal values.
  const auto choose_output = [&](const std::vector<int>& ids) -> int
  {
    int fallback = -1;

    for (int id : ids)
    {
      const int index = node_index(id);

      if (index < 0)
      {
        continue;
      }

      if (fallback < 0)
      {
        fallback = id;
      }

      const auto& data = outputs.node_data[index];

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
    const std::string name = "Topology_" + std::to_string(topology_id) + "_Nodeset_" + std::to_string(nodeset_id) + "_" + key;

    if (!execute_command("ccx create historyoutput name " + name + " node"))
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

    if (!execute_command("ccx modify historyoutput " + std::to_string(output_id) + " node nodeset " + std::to_string(nodeset_id) + " totals_no key_on " + key))
    {
      return false;
    }
  }

  int index = node_index(output_id);

  if (index < 0)
  {
    PRINT_ERROR("Automatic history output configuration failed.\n");
    return false;
  }

  const bool missing_key = outputs.node_data[index][key_column] != key;
  const bool totals_only = outputs.node_data[index][4] == "ONLY";

  if (missing_key || totals_only)
  {
    std::string command = "ccx modify historyoutput " + std::to_string(output_id) + " node";

    if (totals_only)
    {
      command += " totals_yes";
    }

    command += " key_on " + key;

    if (!execute_command(command))
    {
      return false;
    }
  }

  index = node_index(output_id);

  if (index < 0 || outputs.node_data[index][key_column] != key || outputs.node_data[index][4] == "ONLY")
  {
    PRINT_ERROR("History output %d does not provide nodal %s values.\n", output_id, key.c_str());
    return false;
  }

  index = node_index(output_id);

  if (index < 0)
  {
    PRINT_ERROR("Invalid nodal history output configuration.\n");
    return false;
  }

  const bool configure_coordinates = outputs.node_data[index][5] != "YES";
  const bool configure_frequency = outputs.node_data[index][2] != "1";

  if (configure_coordinates || configure_frequency)
  {
    const std::string command = "ccx modify historyoutput " + std::to_string(output_id) + " node global_yes frequency 1";

    if (!execute_command(command))
    {
      return false;
    }

    index = node_index(output_id);

    if (index < 0 || outputs.node_data[index][5] != "YES" || outputs.node_data[index][2] != "1")
    {
      PRINT_ERROR("Unable to configure nodal displacement history output.\n");
      return false;
    }

    PRINT_INFO("Automatically configured history output %d for global nodal values at every increment.\n", output_id);
  }

  bool needs_assignment = false;

  if (!CoreTopologyHistoryOutputAssignment::ensure_assigned(steps, step_id, output_id, assigned_ids, execute_command, needs_assignment))
  {
    return false;
  }

  if (created)
  {
    PRINT_INFO("Automatically created history output %d for nodeset %d with key %s.\n", output_id, nodeset_id, key.c_str());
  }
  else if (missing_key || totals_only)
  {
    PRINT_INFO("Automatically configured history output %d to provide nodal %s values.\n", output_id, key.c_str());
  }

  if (needs_assignment)
  {
    PRINT_INFO("Automatically assigned history output %d to step %d.\n", output_id, step_id);
  }

  return true;
}
