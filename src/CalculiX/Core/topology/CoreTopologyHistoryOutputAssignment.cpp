#include "CoreTopologyHistoryOutputAssignment.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cstddef>

bool CoreTopologyHistoryOutputAssignment::collect_ids(CoreSteps& steps, int step_id, std::vector<int>& assigned_ids)
{
  assigned_ids.clear();

  const int step_index = steps.get_steps_data_id_from_step_id(step_id);

  if (step_index < 0 || steps.steps_data[step_index].size() < 8)
  {
    PRINT_ERROR("Missing step %d.\n", step_id);
    return false;
  }

  std::vector<int> collected;

  for (int index : steps.get_historyoutput_data_ids_from_historyoutputs_id(steps.steps_data[step_index][7]))
  {
    if (index < 0 || static_cast<std::size_t>(index) >= steps.historyoutputs_data.size())
    {
      PRINT_ERROR("Invalid step history output assignment.\n");
      return false;
    }

    const auto& assignment = steps.historyoutputs_data[index];

    if (assignment.size() < 2)
    {
      PRINT_ERROR("Invalid step history output assignment.\n");
      return false;
    }

    collected.push_back(assignment[1]);
  }

  assigned_ids.swap(collected);
  return true;
}

bool CoreTopologyHistoryOutputAssignment::ensure_assigned(CoreSteps& steps, int step_id, int output_id, const std::vector<int>& assigned_ids, const std::function<bool(const std::string&)>& command, bool& needs_assignment)
{
  needs_assignment = std::find(assigned_ids.begin(), assigned_ids.end(), output_id) == assigned_ids.end();

  if (needs_assignment)
  {
    if (!command("ccx step " + std::to_string(step_id) + " add historyoutput " + std::to_string(output_id)))
    {
      return false;
    }
  }

  // Re-query after the command.
  const int updated_step = steps.get_steps_data_id_from_step_id(step_id);
  bool assigned = false;

  if (updated_step >= 0 && steps.steps_data[updated_step].size() >= 8)
  {
    for (int index : steps.get_historyoutput_data_ids_from_historyoutputs_id(steps.steps_data[updated_step][7]))
    {
      if (index < 0 || static_cast<std::size_t>(index) >= steps.historyoutputs_data.size())
      {
        PRINT_ERROR("Invalid step history output assignment.\n");
        return false;
      }

      const auto& data = steps.historyoutputs_data[index];

      if (data.size() >= 2 && data[1] == output_id)
      {
        assigned = true;
      }
    }
  }

  if (!assigned)
  {
    PRINT_ERROR("History output %d was not assigned to step %d.\n", output_id, step_id);
    return false;
  }

  return true;
}
