#include "CoreMeshQuery.hpp"
#include "MeshExportInterface.hpp"
#include "CoreBlocks.hpp"
#include "CubitInterface.hpp"

CoreMeshQuery::CoreMeshQuery(MeshExportInterface& mesh)
  : mesh(mesh)
{
}

std::string CoreMeshQuery::get_block_name(int block_id)
{
  BlockHandle block;

  if (!mesh.get_block_handle(block_id, block))
  {
    return "BLOCK " + std::to_string(block_id) + " doesn't exist!";
  }

  std::string name = mesh.name_from_handle(block);

  if (name.empty())
  {
    name = "Block_" + std::to_string(block_id);
  }

  return name;
}

std::string CoreMeshQuery::get_nodeset_name(int nodeset_id)
{
  NodesetHandle nodeset;

  if (!mesh.get_nodeset_handle(nodeset_id, nodeset))
  {
    return "NODESET " + std::to_string(nodeset_id) + " doesn't exist!";
  }

  std::string name = mesh.get_nodeset_name(nodeset);

  if (name.empty())
  {
    name = "Nodeset_" + std::to_string(nodeset_id);
  }

  return name;
}

std::string CoreMeshQuery::get_sideset_name(int sideset_id)
{
  SidesetHandle sideset;

  if (!mesh.get_sideset_handle(sideset_id, sideset))
  {
    return "SIDESET " + std::to_string(sideset_id) + " doesn't exist!";
  }

  std::string name = mesh.get_sideset_name(sideset);

  if (name.empty())
  {
    name = "Surface_" + std::to_string(sideset_id);
  }

  return name;
}

bool CoreMeshQuery::check_nodeset_exists(int nodeset_id)
{
  NodesetHandle nodeset;
  return mesh.get_nodeset_handle(nodeset_id, nodeset);
}

bool CoreMeshQuery::check_sideset_exists(int sideset_id)
{
  SidesetHandle sideset;
  return mesh.get_sideset_handle(sideset_id, sideset);
}

std::vector<int> CoreMeshQuery::get_block_node_ids(int block_id)
{
  std::vector<int> nodes, spheres, edges, triangles, faces, pyramids, wedges, tetrahedra, hexahedra;
  CubitInterface::get_block_elements_and_nodes(block_id, nodes, spheres, edges, triangles, faces, pyramids, wedges, tetrahedra, hexahedra);
  return nodes;
}

std::vector<int> CoreMeshQuery::get_block_element_ids(CoreBlocks& blocks, int block_id)
{
  std::vector<int> element_ids;
  const std::string block_element_type = CubitInterface::get_block_element_type(block_id);

  if (block_element_type == "SPHERE")
  {
    return element_ids;
  }

  const int buffer_size = 100;
  std::vector<ElementType> element_types(buffer_size);
  std::vector<ElementHandle> handles(buffer_size);
  BlockHandle block;
  mesh.get_block_handle(block_id, block);

  int start_index = 0;
  int element_count;

  while ((element_count = mesh.get_block_elements(start_index, buffer_size, block, element_types, handles)) > 0)
  {
    std::vector<int> ids(element_count);
    mesh.get_element_ids(element_count, handles, ids);
    const std::string element_type = mesh.get_element_type_name(element_types[0]);

    if (element_types[0] != 0 && block_element_type == element_type)
    {
      const std::string entity = blocks.get_cubit_element_type_entity_name(element_type);

      for (int id : ids)
      {
        element_ids.push_back(CubitInterface::get_global_element_id(entity, id));
      }
    }

    start_index += element_count;
  }

  return element_ids;
}

std::vector<std::vector<int>> CoreMeshQuery::get_element_id_type_connectivity(CoreBlocks& blocks)
{
  std::vector<std::vector<int>> connectivity;
  const std::vector<int> block_ids = blocks.get_blocks();

  const int buffer_size = 100;
  std::vector<ElementType> element_types(buffer_size);
  std::vector<ElementHandle> handles(buffer_size);

  for (int block_id : block_ids)
  {
    BlockHandle block;
    mesh.get_block_handle(block_id, block);

    int start_index = 0;
    int element_count;

    while ((element_count = mesh.get_block_elements(start_index, buffer_size, block, element_types, handles)) > 0)
    {
      std::vector<int> ids(element_count);
      mesh.get_element_ids(element_count, handles, ids);

      const std::string element_type = mesh.get_element_type_name(element_types[0]);
      const std::string entity = blocks.get_cubit_element_type_entity_name(element_type);

      for (int i = 0; i < element_count; i++)
      {
        const int global_id = CubitInterface::get_global_element_id(entity, ids[i]);
        std::vector<int> row = {global_id, element_types[i]};

        int node_count;
        mesh.get_nodes_per_element(element_types[i], node_count);

        std::vector<int> nodes(node_count);
        mesh.get_connectivity(handles[i], nodes);
        row.insert(row.end(), nodes.begin(), nodes.end());
        connectivity.push_back(row);
      }

      start_index += element_count;
    }
  }

  return connectivity;
}

bool CoreMeshQuery::check_global_element_exists(int element_id)
{
  return CubitInterface::get_element_exists(element_id);
}

bool CoreMeshQuery::check_vertex_exists(int vertex_id)
{
  return CubitInterface::parse_cubit_list("vertex", std::to_string(vertex_id)).size() == 1;
}

bool CoreMeshQuery::check_curve_exists(int curve_id)
{
  return CubitInterface::parse_cubit_list("curve", std::to_string(curve_id)).size() == 1;
}

bool CoreMeshQuery::check_surface_exists(int surface_id)
{
  return CubitInterface::parse_cubit_list("surface", std::to_string(surface_id)).size() == 1;
}

bool CoreMeshQuery::check_vertex_in_nodeset_exists(int vertex_id, int nodeset_id)
{
  const std::vector<int> vertices = CubitInterface::get_nodeset_vertices(nodeset_id);

  for (int vertex : vertices)
  {
    if (vertex == vertex_id)
    {
      return true;
    }
  }

  return false;
}
