#include "CoreInitialConditionsTreeData.hpp"
#include "CoreInitialConditions.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "CubitMessage.hpp"

namespace
{
  template<typename BCType>
  std::string get_bc_label(
    BCType& bcs, const std::string& bc_id,
    const std::string& type, const std::string& unset_message)
  {
    std::string name;

    if (bc_id.empty())
    {
      name = unset_message;
    }
    else
    {
      int bcs_data_id = bcs.get_bcs_data_id_from_bc_id(std::stoi(bc_id));

      if (bcs_data_id == -1)
      {
        name = type + " ID " + bc_id + " doesn't exist!";
      }
      else
      {
        int name_data_id = bcs.get_name_data_id_from_name_id(bcs.bcs_data[bcs_data_id][6]);

        if (name_data_id != -1)
        {
          name = bcs.name_data[name_data_id][1];
        }
      }
    }

    if (name.empty())
    {
      name = type + "_" + bc_id;
    }

    return name + " (" + type + ")";
  }
}

std::vector<std::vector<std::string>> CoreInitialConditionsTreeData::get_initialconditions_tree_data(
  CoreInitialConditions& conditions,
  CoreBCsDisplacements& displacements,
  CoreBCsTemperatures& temperatures)
{
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& condition : conditions.initialconditions_data)
  {
    std::string name;

    if (condition[1] == 1)
    {
      const int index = conditions.get_displacement_data_id_from_displacement_id(condition[2]);

      if (index < 0)
      {
        PRINT_ERROR("Displacement data missing for Initial Condition ID %d.\n", condition[0]);
        continue;
      }

      name = get_bc_label(displacements, conditions.displacement_data[index][1], "Displacement", "Displacement ID not set!");
    }
    else if (condition[1] == 2)
    {
      const int index = conditions.get_temperature_data_id_from_temperature_id(condition[2]);

      if (index < 0)
      {
        PRINT_ERROR("Temperature data missing for Initial Condition ID %d.\n", condition[0]);
        continue;
      }

      name = get_bc_label(temperatures, conditions.temperature_data[index][1], "Temperature", "Temperature ID not set");
    }
    else if (condition[1] == 3)
    {
      if (conditions.get_stress_data_id_from_stress_id(condition[2]) == -1)
      {
        PRINT_ERROR("Stress data missing for Initial Condition ID %d.\n", condition[0]);
        continue;
      }

      name = "Stress_" + std::to_string(condition[2]);
    }

    tree_data.push_back({std::to_string(condition[0]), name});
  }

  return tree_data;
}
