#include "CoreTopologyPreservation.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "MeshExportInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include <set>
#include <string>

bool CoreTopologyPreservation::get_set_element_ids(const CoreMaterials& materials, CoreBlocks& blocks, MeshExportInterface& mesh, int topology_id, int set_type, int set_id, std::vector<int>& element_ids)
{
  element_ids.clear();

  const auto collection = materials.topology_material_data.find(topology_id);

  if (collection == materials.topology_material_data.end())
  {
    PRINT_ERROR("Material collection for topology %d not found.\n", topology_id);
    return false;
  }

  const auto& analysis_elements = collection->second.element_base_material;
  CoreMeshQuery mesh_query(mesh);

  bool exists = false;

  if (set_id > 0)
  {
    if (set_type == 1)
    {
      exists = blocks.check_block_exists(set_id);
    }
    else if (set_type == 2)
    {
      exists = mesh_query.check_nodeset_exists(set_id);
    }
    else if (set_type == 3)
    {
      exists = mesh_query.check_sideset_exists(set_id);
    }
  }

  if (!exists)
  {
    PRINT_ERROR("Invalid topology set type %d, ID %d.\n", set_type, set_id);
    return false;
  }

  std::set<int> resolved;

  if (set_type == 1)
  {
    for (int id : mesh_query.get_block_element_ids(blocks, set_id))
    {
      if (analysis_elements.count(id) != 0)
      {
        resolved.insert(id);
      }
    }
  }
  else if (set_type == 2)
  {
    const auto node_ids = CubitInterface::get_nodeset_nodes_inclusive(set_id);
    const std::set<int> selected_nodes(node_ids.begin(), node_ids.end());

    for (const auto& assignment : analysis_elements)
    {
      const int global_id = assignment.first;

      if (!CubitInterface::get_element_exists(global_id))
      {
        PRINT_ERROR("Topology element %d does not exist.\n", global_id);
        return false;
      }

      std::string entity = CubitInterface::get_element_type(global_id);

      if (entity != "hex" && entity != "tet" && entity != "wedge")
      {
        entity = blocks.get_cubit_element_type_entity_name(entity);
      }

      const int local_id = CubitInterface::get_element_type_id(global_id);

      if ((entity != "hex" && entity != "tet" && entity != "wedge") || local_id <= 0)
      {
        PRINT_ERROR("Invalid topology volume element %d.\n", global_id);
        return false;
      }

      // Include midside nodes for quadratic elements.
      const auto connectivity = CubitInterface::get_expanded_connectivity(entity, local_id);

      if (connectivity.empty())
      {
        PRINT_ERROR("Missing connectivity for topology element %d.\n", global_id);
        return false;
      }

      for (const int node_id : connectivity)
      {
        if (selected_nodes.count(node_id) != 0)
        {
          resolved.insert(global_id);
          break;
        }
      }
    }

    PRINT_INFO(
        "Topology %d: nodeset %d contains %zu nodes "
        "and selects %zu preserved elements.\n",
        topology_id, set_id, selected_nodes.size(), resolved.size());
  }
  else
  {
    SidesetHandle sideset;

    if (!mesh.get_sideset_handle(set_id, sideset))
    {
      PRINT_ERROR("Unable to query sideset %d.\n", set_id);
      return false;
    }

    std::vector<ElementHandle> handles;
    std::vector<ElementType> types;
    std::vector<int> side_ids;

    const int batch_size = 100;
    int start = 0;
    int count = 0;

    while ((count = mesh.get_sideset_sides(sideset, start, batch_size, handles, types, side_ids)) > 0)
    {
      for (int i = 0; i < count; ++i)
      {
        int local_id = -1;
        mesh.get_element_id(handles[i], local_id);

        const std::string entity = blocks.get_cubit_element_type_entity_name(mesh.get_element_type_name(types[i]));
        const int global_id = CubitInterface::get_global_element_id(entity, local_id);

        if (analysis_elements.count(global_id) != 0)
        {
          resolved.insert(global_id);
        }
      }

      start += count;
    }

    if (count < 0)
    {
      PRINT_ERROR("Failed to read sideset %d.\n", set_id);
      return false;
    }
  }

  element_ids.assign(resolved.begin(), resolved.end());
  return true;
}

bool CoreTopologyPreservation::collect_step_references(CoreSteps& steps, std::map<int, std::set<int>>& used_loads, std::map<int, std::set<int>>& used_bcs)
{
  used_loads.clear();
  used_bcs.clear();

  std::map<int, std::set<int>> collected_loads;
  std::map<int, std::set<int>> collected_bcs;

  for (const auto& step : steps.steps_data)
  {
    if (step.size() < 7)
    {
      PRINT_ERROR("Invalid step data.\n");
      return false;
    }

    const auto load_indices = steps.get_load_data_ids_from_loads_id(step[5]);

    for (int index : load_indices)
    {
      const auto& load = steps.loads_data[index];

      if (load.size() < 3 || load[1] < 1 || load[1] > 10 || load[2] <= 0)
      {
        PRINT_ERROR("Invalid step load reference.\n");
        return false;
      }

      collected_loads[load[1]].insert(load[2]);
    }

    const auto bc_indices = steps.get_bc_data_ids_from_bcs_id(step[6]);

    for (int index : bc_indices)
    {
      const auto& bc = steps.bcs_data[index];

      if (bc.size() < 3 || (bc[1] != 1 && bc[1] != 2) || bc[2] <= 0)
      {
        PRINT_ERROR("Invalid step boundary condition reference.\n");
        return false;
      }

      collected_bcs[bc[1]].insert(bc[2]);
    }
  }

  used_loads.swap(collected_loads);
  used_bcs.swap(collected_bcs);
  return true;
}

bool CoreTopologyPreservation::build_set_rows(const TopologyCase& configuration, const std::map<int, std::set<int>>& defaults, const std::set<int>& default_elements, std::vector<TopologyPreservedSetInfo>& sets, std::set<int>& direct_elements)
{
  sets.clear();
  direct_elements.clear();

  auto all_sets = defaults;

  const auto merge = [&](const std::map<int, std::set<int>>& groups)
  {
    for (const auto& group : groups)
    {
      all_sets[group.first].insert(group.second.begin(), group.second.end());
    }
  };

  merge(configuration.added_preserved_sets);
  merge(configuration.removed_preserved_sets);

  const auto contains = [](const std::map<int, std::set<int>>& groups, int set_type, int set_id) -> bool
  {
    const auto group = groups.find(set_type);
    return group != groups.end() && group->second.count(set_id) != 0;
  };

  std::vector<TopologyPreservedSetInfo> rows;

  for (const auto& group : all_sets)
  {
    if (group.first < 1 || group.first > 3)
    {
      PRINT_ERROR("Invalid topology preserved set type %d.\n", group.first);
      return false;
    }

    for (int set_id : group.second)
    {
      if (set_id <= 0)
      {
        PRINT_ERROR("Invalid topology preserved set ID %d.\n", set_id);
        return false;
      }

      TopologyPreservedSetInfo row;
      row.set_type = group.first;
      row.set_id = set_id;
      row.automatic = contains(defaults, row.set_type, set_id);
      row.user_added = contains(configuration.added_preserved_sets, row.set_type, set_id);
      row.excluded = contains(configuration.removed_preserved_sets, row.set_type, set_id);
      row.effective = (row.automatic || row.user_added) && !row.excluded;
      rows.push_back(row);
    }
  }

  sets.swap(rows);
  direct_elements = default_elements;
  return true;
}
