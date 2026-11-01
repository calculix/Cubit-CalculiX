#include "CoreTopologyGeometry.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "MeshExportInterface.hpp"
#include <algorithm>
#include <cstddef>
#include "CoreTopology.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ElementIntegration.hpp"
#include <cmath>
#include <set>
#include <string>
#include <vector>

bool CoreTopologyGeometry::collect_element_volumes(const CoreMaterials& materials, CoreBlocks& blocks, int topology_id, std::map<int, double>& element_volumes, std::map<int, std::array<double, 3>>* element_centers)
{
  element_volumes.clear();

  if (element_centers)
  {
    element_centers->clear();
  }

  const auto collection = materials.topology_material_data.find(topology_id);

  if (collection == materials.topology_material_data.end() || collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Topology %d has no prepared elements.\n", topology_id);
    return false;
  }

  std::map<int, double> volumes;
  std::map<int, std::array<double, 3>> centers;

  PRINT_INFO("Topology %d: querying prepared elements directly.\n", topology_id);

  for (const auto& assignment : collection->second.element_base_material)
  {
    const int global_id = assignment.first;
    const std::string type = CubitInterface::get_element_type(global_id);

    std::string entity;

    if (type == "hex" || type == "tet" || type == "wedge")
    {
      entity = type;
    }
    else
    {
      entity = blocks.get_cubit_element_type_entity_name(type);
    }

    const int local_id = CubitInterface::get_element_type_id(global_id);

    if ((entity != "hex" && entity != "tet" && entity != "wedge") || local_id <= 0)
    {
      PRINT_ERROR("Invalid topology element %d: type '%s', local ID %d.\n", global_id, type.c_str(), local_id);
      return false;
    }

    const bool first = volumes.empty();
    const double volume = CubitInterface::get_meshed_volume_or_area(entity, {local_id});

    if (!std::isfinite(volume) || volume <= 0.0)
    {
      PRINT_ERROR("Invalid volume for topology element %d.\n", global_id);
      return false;
    }

    volumes.emplace(global_id, volume);

    if (element_centers)
    {
      const auto center = CubitInterface::get_center_point(entity, local_id);

      for (double coordinate : center)
      {
        if (!std::isfinite(coordinate))
        {
          PRINT_ERROR("Invalid center for topology element %d.\n", global_id);
          return false;
        }
      }

      centers.emplace(global_id, center);

      if (first)
      {
        PRINT_INFO("First element center queried successfully.\n");
      }
    }
  }

  element_volumes.swap(volumes);

  if (element_centers)
  {
    element_centers->swap(centers);
  }

  PRINT_INFO("Topology %d: element geometry queries completed.\n", topology_id);
  return true;
}

bool CoreTopologyGeometry::collect_integration_weights(const CoreTopology& topology, const CoreMaterials& materials, CoreBlocks& blocks, int topology_id, std::map<int, std::map<int, double>>& weights)
{
  weights.clear();

  if (topology.topology_cases.find(topology_id) == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const auto collection = materials.topology_material_data.find(topology_id);

  if (collection == materials.topology_material_data.end() || collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Topology %d has no prepared elements.\n", topology_id);
    return false;
  }

  std::map<int, std::map<int, double>> collected;

  for (const auto& assignment : collection->second.element_base_material)
  {
    const int element_id = assignment.first;

    if (!CubitInterface::get_element_exists(element_id))
    {
      PRINT_ERROR("Topology element %d no longer exists.\n", element_id);
      return false;
    }

    std::string entity = CubitInterface::get_element_type(element_id);

    if (entity != "hex" && entity != "tet" && entity != "wedge")
    {
      entity = blocks.get_cubit_element_type_entity_name(entity);
    }

    const int local_id = CubitInterface::get_element_type_id(element_id);

    if ((entity != "hex" && entity != "tet" && entity != "wedge") || local_id <= 0)
    {
      PRINT_ERROR("Invalid topology volume element %d.\n", element_id);
      return false;
    }

    const int block_id = CubitInterface::get_element_block(element_id);

    if (block_id <= 0 || blocks.get_blocks_data_id_from_block_id(block_id) < 0)
    {
      PRINT_ERROR("Missing CalculiX block assignment for topology element %d.\n", element_id);
      return false;
    }

    const std::string calculix_type = blocks.get_ccx_element_type_name(block_id);
    const auto connectivity = CubitInterface::get_expanded_connectivity(entity, local_id);

    int cubit_type = -1;

    if (entity == "hex")
    {
      if (connectivity.size() == 8)
      {
        cubit_type = 42; // HEX8
      }
      else if (connectivity.size() == 20)
      {
        cubit_type = 44; // HEX20
      }
    }
    else if (entity == "tet")
    {
      if (connectivity.size() == 4)
      {
        cubit_type = 31; // TETRA4
      }
      else if (connectivity.size() == 10)
      {
        cubit_type = 33; // TETRA10
      }
    }
    else if (entity == "wedge")
    {
      if (connectivity.size() == 6)
      {
        cubit_type = 50; // WEDGE6
      }
      else if (connectivity.size() == 15)
      {
        cubit_type = 52; // WEDGE15
      }
    }

    if (cubit_type < 0)
    {
      PRINT_ERROR("Unsupported connectivity for topology element %d: %s with %d nodes.\n", element_id, entity.c_str(), static_cast<int>(connectivity.size()));
      return false;
    }

    std::vector<ElementIntegration::Coordinates> coordinates;
    coordinates.reserve(connectivity.size());

    std::set<int> unique_nodes;

    for (int node_id : connectivity)
    {
      if (node_id <= 0 || !CubitInterface::get_node_exists(node_id))
      {
        PRINT_ERROR("Invalid node %d in topology element %d.\n", node_id, element_id);
        return false;
      }

      if (!unique_nodes.insert(node_id).second)
      {
        PRINT_ERROR("Duplicate node %d in topology element %d.\n", node_id, element_id);
        return false;
      }

      coordinates.push_back(CubitInterface::get_nodal_coordinates(node_id));
    }

    std::map<int, double> element_weights;

    if (!ElementIntegration::calculate_weights(calculix_type, cubit_type, coordinates, element_weights))
    {
      PRINT_ERROR("Unable to calculate integration weights for topology %d element %d in block %d.\n", topology_id, element_id, block_id);
      return false;
    }

    collected.emplace(element_id, element_weights);
  }

  weights.swap(collected);
  return true;
}

bool CoreTopologyGeometry::collect_face_owners(CoreBlocks& blocks, MeshExportInterface& mesh, std::map<std::vector<int>, std::set<int>>& face_owners)
{
  face_owners.clear();

  CoreMeshQuery mesh_query(mesh);
  const auto connectivity = mesh_query.get_element_id_type_connectivity(blocks);
  std::map<std::vector<int>, std::set<int>> collected;

  for (const auto& element : connectivity)
  {
    if (element.size() < 3 || element[0] <= 0)
    {
      PRINT_ERROR("Invalid topology connectivity row.\n");
      return false;
    }

    const std::string entity = blocks.get_cubit_element_type_entity_name(mesh.get_element_type_name(static_cast<ElementType>(element[1])));

    // Corner-node indices in Exodus connectivity order.
    std::vector<std::vector<int>> faces;
    std::size_t corner_count = 0;

    if (entity == "hex")
    {
      corner_count = 8;
      faces = {
          {0, 1, 2, 3},
          {4, 5, 6, 7},
          {0, 1, 5, 4},
          {1, 2, 6, 5},
          {2, 3, 7, 6},
          {3, 0, 4, 7}
      };
    }
    else if (entity == "tet")
    {
      corner_count = 4;
      faces = {
          {0, 1, 2},
          {0, 1, 3},
          {1, 2, 3},
          {2, 0, 3}
      };
    }
    else if (entity == "wedge")
    {
      corner_count = 6;
      faces = {
          {0, 1, 2},
          {3, 4, 5},
          {0, 1, 4, 3},
          {1, 2, 5, 4},
          {2, 0, 3, 5}
      };
    }
    else
    {
      continue;
    }

    if (element.size() < corner_count + 2)
    {
      PRINT_ERROR("Incomplete connectivity for element %d.\n", element[0]);
      return false;
    }

    for (const auto& face : faces)
    {
      std::vector<int> key;

      for (int corner : face)
      {
        key.push_back(element[2 + corner]);
      }

      std::sort(key.begin(), key.end());
      collected[key].insert(element[0]);
    }
  }

  face_owners.swap(collected);
  return true;
}

bool CoreTopologyGeometry::append_node_elements(CoreBlocks& blocks, MeshExportInterface& mesh, const std::set<int>& nodes, std::set<int>& elements)
{
  if (nodes.empty())
  {
    return true;
  }

  CoreMeshQuery mesh_query(mesh);
  const auto connectivity = mesh_query.get_element_id_type_connectivity(blocks);

  for (const auto& element : connectivity)
  {
    // Row: global element ID, element type, node IDs.
    if (element.size() < 3 || element[0] <= 0)
    {
      PRINT_ERROR("Invalid topology connectivity row.\n");
      return false;
    }

    for (std::size_t i = 2; i < element.size(); ++i)
    {
      if (nodes.count(element[i]) != 0)
      {
        elements.insert(element[0]);
        break;
      }
    }
  }

  return true;
}

bool CoreTopologyGeometry::collect_element_nodes(CoreBlocks& blocks, const std::map<int, int>& element_materials, std::set<int>& nodes)
{
  nodes.clear();
  std::set<int> collected;

  for (const auto& assignment : element_materials)
  {
    const int element_id = assignment.first;

    if (!CubitInterface::get_element_exists(element_id))
    {
      PRINT_ERROR("Topology element %d no longer exists.\n", element_id);
      return false;
    }

    std::string entity = CubitInterface::get_element_type(element_id);

    if (entity != "hex" && entity != "tet" && entity != "wedge")
    {
      entity = blocks.get_cubit_element_type_entity_name(entity);
    }

    const int local_id = CubitInterface::get_element_type_id(element_id);

    if ((entity != "hex" && entity != "tet" && entity != "wedge") || local_id <= 0)
    {
      PRINT_ERROR("Invalid topology volume element %d.\n", element_id);
      return false;
    }

    const auto connectivity = CubitInterface::get_expanded_connectivity(entity, local_id);

    if (connectivity.empty())
    {
      PRINT_ERROR("Missing connectivity for topology element %d.\n", element_id);
      return false;
    }

    for (int node_id : connectivity)
    {
      if (node_id <= 0)
      {
        PRINT_ERROR("Invalid connectivity for topology element %d.\n", element_id);
        return false;
      }

      collected.insert(node_id);
    }
  }

  nodes.swap(collected);
  return true;
}
