#include "Core/steps/CoreStepsCleanup.hpp"
#include "Core/steps/CoreSteps.hpp"

#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreFieldOutputs.hpp"

#include <array>
#include <cstddef>

bool CoreStepsCleanup::cleanup_loads(CoreSteps& steps, int step_id, int loads_id, const std::function<bool(int, int)>& load_exists, std::string& log)
{
  static const std::array<std::string, 10> labels = {
    "Force", "Pressure", "Heatflux", "Gravity", "Centrifugal",
    "Trajectory", "Film", "Radiation", "Surface Traction", "BodyHeatflux"
  };

  bool changed = false;

  for (std::size_t type_index = 0; type_index < labels.size(); type_index++)
  {
    const int load_type = static_cast<int>(type_index) + 1;
    const auto indices = steps.get_load_data_ids_from_loads_id(loads_id);

    for (std::size_t index = indices.size(); index > 0; index--)
    {
      const auto& reference = steps.loads_data[indices[index - 1]];

      if (reference[1] == load_type)
      {
        const int load_id = reference[2];

        if (!load_exists(load_type, load_id))
        {
          const std::string& label = labels[type_index];
          log.append("Load " + label + " ID " + std::to_string(load_id) + " doesn't exist.\n");
          log.append("Load " + label + " Reference from Step ID " + std::to_string(step_id) + " will be deleted.\n");

          steps.remove_loads(step_id, load_type, {load_id});
          changed = true;
        }
      }
    }
  }

  return changed;
}

bool CoreStepsCleanup::cleanup_bcs(CoreSteps& steps, int step_id, int bcs_id, CoreBCsDisplacements& displacements, CoreBCsTemperatures& temperatures, std::string& log)
{
  static const std::array<std::string, 2> labels = {"Displacement", "Temperature"};
  bool changed = false;

  for (std::size_t type_index = 0; type_index < labels.size(); type_index++)
  {
    const int bc_type = static_cast<int>(type_index) + 1;
    const auto indices = steps.get_bc_data_ids_from_bcs_id(bcs_id);

    for (std::size_t index = indices.size(); index > 0; index--)
    {
      const auto& reference = steps.bcs_data[indices[index - 1]];

      if (reference[1] == bc_type)
      {
        const int bc_id = reference[2];
        const bool exists = bc_type == 1 ? displacements.get_bcs_data_id_from_bc_id(bc_id) != -1 : temperatures.get_bcs_data_id_from_bc_id(bc_id) != -1;

        if (!exists)
        {
          const std::string& label = labels[type_index];
          log.append("BC " + label + " ID " + std::to_string(bc_id) + " doesn't exist.\n");
          log.append("BC " + label + " Reference from Step ID " + std::to_string(step_id) + " will be deleted.\n");

          steps.remove_bcs(step_id, bc_type, {bc_id});
          changed = true;
        }
      }
    }
  }

  return changed;
}

bool CoreStepsCleanup::cleanup_outputs(CoreSteps& steps, int step_id, int history_id, int field_id, CoreHistoryOutputs& history, CoreFieldOutputs& field, std::string& log)
{
  bool changed = false;
  const auto history_indices = steps.get_historyoutput_data_ids_from_historyoutputs_id(history_id);

  for (std::size_t index = history_indices.size(); index > 0; index--)
  {
    const int output_id = steps.historyoutputs_data[history_indices[index - 1]][1];

    if (!history.check_historyoutput_exists(output_id))
    {
      log.append("History Output ID " + std::to_string(output_id) + " doesn't exist.\n");
      log.append("History Output Reference from Step ID " + std::to_string(step_id) + " will be deleted.\n");

      steps.remove_historyoutputs(step_id, {output_id});
      changed = true;
    }
  }

  const auto field_indices = steps.get_fieldoutput_data_ids_from_fieldoutputs_id(field_id);

  for (std::size_t index = field_indices.size(); index > 0; index--)
  {
    const int output_id = steps.fieldoutputs_data[field_indices[index - 1]][1];

    if (!field.check_fieldoutput_exists(output_id))
    {
      log.append("Field Output ID " + std::to_string(output_id) + " doesn't exist.\n");
      log.append("Field Output Reference from Step ID " + std::to_string(step_id) + " will be deleted.\n");

      steps.remove_fieldoutputs(step_id, {output_id});
      changed = true;
    }
  }

  return changed;
}

bool CoreStepsCleanup::cleanup(CoreSteps& steps, CoreBCsDisplacements& displacements, CoreBCsTemperatures& temperatures, CoreHistoryOutputs& history, CoreFieldOutputs& field, const std::function<bool(int, int)>& load_exists, std::string& log)
{
  bool changed = false;

  for (std::size_t index = steps.steps_data.size(); index > 0; index--)
  {
    const auto& step = steps.steps_data[index - 1];
    const int step_id = step[0];

    if (cleanup_loads(steps, step_id, step[5], load_exists, log))
    {
      changed = true;
    }

    if (cleanup_bcs(steps, step_id, step[6], displacements, temperatures, log))
    {
      changed = true;
    }

    if (cleanup_outputs(steps, step_id, step[7], step[8], history, field, log))
    {
      changed = true;
    }
  }

  return changed;
}
