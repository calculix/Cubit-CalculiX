#include "CoreBCsTreeData.hpp"
#include "CoreBCsDisplacements.hpp"
#include "CoreBCsTemperatures.hpp"
#include "CoreHBCs.hpp"
#include "CubitMessage.hpp"

namespace
{
  template<typename BCType>
  std::string get_bc_name(BCType& bcs, int name_id, int bc_id, const std::string& prefix)
  {
    int name_data_id = bcs.get_name_data_id_from_name_id(name_id);

    if (name_data_id != -1 && !bcs.name_data[name_data_id][1].empty())
    {
      return bcs.name_data[name_data_id][1];
    }

    return prefix + std::to_string(bc_id);
  }

  template<typename BCType>
  std::vector<std::vector<std::string>> get_bc_tree_data(BCType& bcs, const std::string& prefix)
  {
    std::vector<std::vector<std::string>> tree_data;

    for (const auto& bc : bcs.bcs_data)
    {
      std::string name = get_bc_name(bcs, bc[6], bc[0], prefix);
      tree_data.push_back({std::to_string(bc[0]), name});
    }

    return tree_data;
  }

  template<typename BCType>
  std::vector<std::vector<std::string>> get_hbc_tree_data(
    CoreHBCs& hbcs, BCType& bcs, int bc_type, const std::string& prefix)
  {
    std::vector<std::vector<std::string>> tree_data;
    std::vector<int> data_ids = hbcs.get_bc_data_ids_from_bcs_id(0);

    for (int data_id : data_ids)
    {
      const auto& reference = hbcs.bcs_data[data_id];

      if (reference[1] == bc_type)
      {
        const int bc_id = reference[2];
        const int bcs_data_id = bcs.get_bcs_data_id_from_bc_id(bc_id);

        if (bcs_data_id < 0)
        {
          PRINT_ERROR("HBC references missing BC ID %d (type %d).\n", bc_id, bc_type);
          continue;
        }

        const std::string name = get_bc_name(bcs, bcs.bcs_data[bcs_data_id][6], bc_id, prefix);
        tree_data.push_back({std::to_string(bc_id), name});
      }
    }

    return tree_data;
  }
}

std::vector<std::vector<std::string>> CoreBCsTreeData::get_bcsdisplacements_tree_data(CoreBCsDisplacements& bcs)
{
  return get_bc_tree_data(bcs, "Displacement_");
}

std::vector<std::vector<std::string>> CoreBCsTreeData::get_bcstemperatures_tree_data(CoreBCsTemperatures& bcs)
{
  return get_bc_tree_data(bcs, "Temperature_");
}

std::vector<std::vector<std::string>> CoreBCsTreeData::get_hbcsdisplacements_tree_data(
  CoreHBCs& hbcs, CoreBCsDisplacements& bcs)
{
  return get_hbc_tree_data(hbcs, bcs, 1, "Displacement_");
}

std::vector<std::vector<std::string>> CoreBCsTreeData::get_hbcstemperatures_tree_data(
  CoreHBCs& hbcs, CoreBCsTemperatures& bcs)
{
  return get_hbc_tree_data(hbcs, bcs, 2, "Temperature_");
}
