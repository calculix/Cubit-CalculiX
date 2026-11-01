#include "CoreInitialConditionsEntities.hpp"
#include "CoreInitialConditions.hpp"
#include "Core/bcs/CoreBCsEntities.hpp"
#include "CubitInterface.hpp"

std::vector<std::vector<std::string>> CoreInitialConditionsEntities::get_entities(
  CoreInitialConditions& conditions, CoreBCsDisplacements& displacements,
  CoreBCsTemperatures& temperatures, int initialcondition_id)
{
  const int index = conditions.get_initialconditions_data_id_from_initialcondition_id(initialcondition_id);

  if (index == -1)
  {
    return {};
  }

  const auto& condition = conditions.initialconditions_data[index];
  const int type = condition[1];
  const int subtype_id = condition[2];

  if (type == 1)
  {
    const int data_index = conditions.get_displacement_data_id_from_displacement_id(subtype_id);

    if (data_index == -1 || conditions.displacement_data[data_index][1].empty())
    {
      return {};
    }

    const int bc_id = std::stoi(conditions.displacement_data[data_index][1]);
    return CoreBCsEntities::get_entities(displacements, bc_id);
  }

  if (type == 2)
  {
    const int data_index = conditions.get_temperature_data_id_from_temperature_id(subtype_id);

    if (data_index == -1 || conditions.temperature_data[data_index][1].empty())
    {
      return {};
    }

    const int bc_id = std::stoi(conditions.temperature_data[data_index][1]);
    return CoreBCsEntities::get_entities(temperatures, bc_id);
  }

  if (type == 3)
  {
    if (conditions.get_stress_data_id_from_stress_id(subtype_id) == -1)
    {
      return {};
    }

    std::vector<std::vector<std::string>> entities;
    const auto block_data_ids = conditions.get_stress_block_data_ids_from_stress_id(subtype_id);

    for (int data_id : block_data_ids)
    {
      const int block_id = static_cast<int>(conditions.stress_block_data[data_id][1]);
      entities.push_back({"block", std::to_string(block_id)});
    }

    const auto element_data_ids = conditions.get_stress_element_data_ids_from_stress_id(subtype_id);

    for (int data_id : element_data_ids)
    {
      const int element_id = static_cast<int>(conditions.stress_element_data[data_id][1]);
      entities.push_back({CubitInterface::get_element_type(element_id), std::to_string(element_id)});
    }

    return entities;
  }

  return {};
}
