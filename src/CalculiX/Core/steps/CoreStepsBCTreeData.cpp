#include "CoreStepsBCTreeData.hpp"
#include "CoreSteps.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "CubitMessage.hpp"

namespace
{
  template<typename BCType>
  std::vector<std::vector<std::string>> get_step_bc_tree_data(
    CoreSteps& steps, BCType& bcs, int step_id, int bc_type, const std::string& prefix)
  {
    std::vector<std::vector<std::string>> tree_data;
    int step_data_id = steps.get_steps_data_id_from_step_id(step_id);

    if (step_data_id == -1)
    {
      return tree_data;
    }

    std::vector<int> data_ids = steps.get_bc_data_ids_from_bcs_id(steps.steps_data[step_data_id][6]);

    for (int data_id : data_ids)
    {
      const auto& reference = steps.bcs_data[data_id];

      if (reference[1] != bc_type)
      {
        continue;
      }

      int bc_id = reference[2];
      int bcs_data_id = bcs.get_bcs_data_id_from_bc_id(bc_id);

      if (bcs_data_id < 0)
      {
        PRINT_ERROR("Step ID %d references missing BC ID %d (type %d).\n", step_id, bc_id, bc_type);
        continue;
      }

      int name_data_id = bcs.get_name_data_id_from_name_id(bcs.bcs_data[bcs_data_id][6]);
      std::string name;

      if (name_data_id != -1 && !bcs.name_data[name_data_id][1].empty())
      {
        name = bcs.name_data[name_data_id][1];
      }
      else
      {
        name = prefix + std::to_string(bc_id);
      }

      tree_data.push_back({std::to_string(bc_id), name});
    }

    return tree_data;
  }
}

std::vector<std::vector<std::string>> CoreStepsBCTreeData::get_steps_bcsdisplacements_tree_data(
  CoreSteps& steps, CoreBCsDisplacements& bcs, int step_id)
{
  return get_step_bc_tree_data(steps, bcs, step_id, 1, "Displacement_");
}

std::vector<std::vector<std::string>> CoreStepsBCTreeData::get_steps_bcstemperatures_tree_data(
  CoreSteps& steps, CoreBCsTemperatures& bcs, int step_id)
{
  return get_step_bc_tree_data(steps, bcs, step_id, 2, "Temperature_");
}
