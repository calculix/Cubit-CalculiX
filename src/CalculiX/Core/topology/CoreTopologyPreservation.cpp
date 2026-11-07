#include "CoreTopologyPreservation.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "MeshExportInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
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
      const auto ids = CubitInterface::get_sideset_id_list();
      exists = std::find(ids.begin(), ids.end(), set_id) != ids.end();
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
    const auto explicit_quads = CubitInterface::get_sideset_quads(set_id);
    const auto explicit_tris = CubitInterface::get_sideset_tris(set_id);

    std::set<int> quad_ids(explicit_quads.begin(), explicit_quads.end());
    std::set<int> tri_ids(explicit_tris.begin(), explicit_tris.end());

    // Geometry membership and explicit mesh membership are both supported.
    for (int surface_id : CubitInterface::get_sideset_surfaces(set_id))
    {
      const auto quads = CubitInterface::get_surface_quads(surface_id);
      const auto tris = CubitInterface::get_surface_tris(surface_id);

      quad_ids.insert(quads.begin(), quads.end());
      tri_ids.insert(tris.begin(), tris.end());
    }

    std::set<std::vector<int>> selected_faces;

    const auto append_face = [&](const std::string& entity, int id) -> bool
    {
      auto nodes = CubitInterface::get_connectivity(entity, id);
      const std::size_t expected = entity == "quad" ? 4 : 3;

      if (nodes.size() != expected)
      {
        PRINT_ERROR("Invalid %s connectivity in sideset %d.\n", entity.c_str(), set_id);
        return false;
      }

      std::sort(nodes.begin(), nodes.end());

      if (nodes.front() <= 0 ||
          std::adjacent_find(nodes.begin(), nodes.end()) != nodes.end())
      {
        PRINT_ERROR("Invalid face nodes in sideset %d.\n", set_id);
        return false;
      }

      selected_faces.insert(nodes);
      return true;
    };

    for (int id : quad_ids)
    {
      if (!append_face("quad", id))
      {
        return false;
      }
    }

    for (int id : tri_ids)
    {
      if (!append_face("tri", id))
      {
        return false;
      }
    }

    // Curve/edge preservation needs a separate membership contract.
    if (!CubitInterface::get_sideset_curves(set_id).empty() ||
        !CubitInterface::get_sideset_edges(set_id).empty())
    {
      PRINT_ERROR("Topology preservation requires surface, quad or tri members in sideset %d; curve/edge members are unsupported.\n", set_id);
      return false;
    }

    // Zero-based corner positions in Cubit/Exodus connectivity.
    // Midside-node permutations do not change these corner positions.
    // A selected internal face preserves both adjacent analysis elements.
    static const std::map<std::string, std::vector<std::vector<std::size_t>>> face_corners = {
      {"hex", {
        {0, 1, 2, 3}, {4, 5, 6, 7},
        {0, 1, 5, 4}, {1, 2, 6, 5},
        {2, 3, 7, 6}, {3, 0, 4, 7}
      }},
      {"tet", {
        {0, 1, 2}, {0, 3, 1},
        {1, 3, 2}, {2, 3, 0}
      }},
      {"wedge", {
        {0, 1, 2}, {3, 4, 5},
        {0, 1, 4, 3}, {1, 2, 5, 4}, {2, 0, 3, 5}
      }}
    };

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
      const auto layout = face_corners.find(entity);

      if (local_id <= 0 || layout == face_corners.end())
      {
        PRINT_ERROR("Invalid topology volume element %d.\n", global_id);
        return false;
      }

      const auto connectivity = CubitInterface::get_connectivity(entity, local_id);
      const std::size_t expected = entity == "hex" ? 8 : entity == "tet" ? 4 : 6;

      if (connectivity.size() != expected)
      {
        PRINT_ERROR("Invalid corner connectivity for topology element %d.\n", global_id);
        return false;
      }

      for (const auto& positions : layout->second)
      {
        std::vector<int> face;
        face.reserve(positions.size());

        for (std::size_t position : positions)
        {
          face.push_back(connectivity[position]);
        }

        std::sort(face.begin(), face.end());

        if (selected_faces.count(face) != 0)
        {
          resolved.insert(global_id);
          break;
        }
      }
    }

    PRINT_INFO("Topology %d: sideset %d contains %zu mesh faces and selects %zu preserved volume elements.\n", topology_id, set_id, selected_faces.size(), resolved.size());
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
