#include "Core/steps/CoreInitialConditionsCleanup.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"

#include <cstddef>
#include <vector>

bool CoreInitialConditionsCleanup::cleanup(CoreInitialConditions& conditions, CoreBCsDisplacements& displacements, CoreBCsTemperatures& temperatures, std::string& log)
{
  bool changed = false;

  for (std::size_t index = conditions.initialconditions_data.size(); index > 0; index--)
  {
    const auto& condition = conditions.initialconditions_data[index - 1];
    const int condition_id = condition[0];

    const auto clear_missing_reference = [&](std::vector<std::string>& row, auto& bcs, const std::string& label)
    {
      if (row[1].empty())
      {
        return;
      }

      if (bcs.get_bcs_data_id_from_bc_id(std::stoi(row[1])) != -1)
      {
        return;
      }

      log.append("BC " + label + " ID " + row[1] + " doesn't exist.\n");
      log.append("BC " + label + " Reference from Initial Condition ID " + std::to_string(condition_id) + " will be deleted.\n");
      row[1].clear();
      changed = true;
    };

    if (condition[1] == 1)
    {
      const int data_index = conditions.get_displacement_data_id_from_displacement_id(condition[2]);

      if (data_index < 0)
      {
        log.append("Displacement data for Initial Condition ID " + std::to_string(condition_id) + " doesn't exist.\n");
        log.append("Initial Condition ID " + std::to_string(condition_id) + " will be deleted.\n");
        changed = conditions.delete_initialcondition(condition_id) || changed;
        continue;
      }

      clear_missing_reference(conditions.displacement_data[data_index], displacements, "Displacement");
    }

    if (condition[1] == 2)
    {
      const int data_index = conditions.get_temperature_data_id_from_temperature_id(condition[2]);

      if (data_index < 0)
      {
        log.append("Temperature data for Initial Condition ID " + std::to_string(condition_id) + " doesn't exist.\n");
        log.append("Initial Condition ID " + std::to_string(condition_id) + " will be deleted.\n");
        changed = conditions.delete_initialcondition(condition_id) || changed;
        continue;
      }

      clear_missing_reference(conditions.temperature_data[data_index], temperatures, "Temperature");
    }

    if (condition[1] == 3)
    {
      if (conditions.get_stress_data_id_from_stress_id(condition[2]) == -1)
      {
        log.append("Stress data for Initial Condition ID " + std::to_string(condition_id) + " doesn't exist.\n");
        log.append("Initial Condition ID " + std::to_string(condition_id) + " will be deleted.\n");
        changed = conditions.delete_initialcondition(condition_id) || changed;
        continue;
      }
    }
  }

  return changed;
}
