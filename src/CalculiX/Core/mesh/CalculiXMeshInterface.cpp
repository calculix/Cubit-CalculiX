#include "CubitInterface.hpp"
#include "MeshExportInterface.hpp"
#include "CoreMeshElementMapping.hpp"
#include "CalculiXMeshInterface.hpp"
#include "CoreMeshQuery.hpp"
#include "CoreBlocks.hpp"
#include "CoreMeshGeometry.hpp"
#include "CoreMeshTreeData.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

bool CalculiXMeshInterface::get_assignment_entity_name(const std::string& type, int id, std::string& name)
{
  name.clear();
  if (id < 1)
  {
    return false;
  }
  if (type == "block" || type == "nodeset" || type == "sideset")
  {
    if (ccx_core.me_iface == nullptr)
    {
      return false;
    }
    auto& mesh = *ccx_core.me_iface;
    if (type == "block")
    {
      BlockHandle handle;
      if (!mesh.get_block_handle(id, handle))
      {
        return false;
      }
      name = mesh.name_from_handle(handle);
    }
    else if (type == "nodeset")
    {
      NodesetHandle handle;
      if (!mesh.get_nodeset_handle(id, handle))
      {
        return false;
      }
      name = mesh.get_nodeset_name(handle);
    }
    else
    {
      SidesetHandle handle;
      if (!mesh.get_sideset_handle(id, handle))
      {
        return false;
      }
      name = mesh.get_sideset_name(handle);
    }
    return true;
  }
  const bool exists = (type == "vertex" && check_vertex_exists(id)) || (type == "curve" && check_curve_exists(id)) || (type == "surface" && check_surface_exists(id));
  if (!exists)
  {
    return false;
  }
  name = CubitInterface::get_entity_name(type, id, true);
  return true;
}

std::vector<std::string> CalculiXMeshInterface::get_valid_block_element_types(int blockId)
{
  if (!check_block_exists(blockId))
  {
    return {};
  }
  return CubitInterface::get_valid_block_element_types(blockId);
}

std::vector<std::string> CalculiXMeshInterface::get_compatible_ccx_element_types(int blockId)
{
  if (ccx_core.cb == nullptr || !check_block_exists(blockId))
  {
    return {};
  }
  return ccx_core.cb->get_compatible_ccx_element_types(CubitInterface::get_block_element_type(blockId));
}

std::vector<std::string> CalculiXMeshInterface::get_ccx_element_types()
{
  return ccx_core.cb->get_ccx_element_types();
}

bool CalculiXMeshInterface::set_ccx_element_type(int block_id, std::string ccx_element_type)
{
  return ccx_core.cb->set_ccx_element_type(block_id, ccx_element_type);
}

std::string CalculiXMeshInterface::get_ccx_element_type(int block_id)
{
  return ccx_core.cb->get_ccx_element_type_name(block_id);
}

int CalculiXMeshInterface::get_ccx_element_type_integration_points(int block_id)
{
  return ccx_core.cb->get_ccx_element_type_integration_points(block_id);
}

std::string CalculiXMeshInterface::get_cubit_element_type_entity(std::string cubit_element_type)
{
  return ccx_core.cb->get_cubit_element_type_entity_name(cubit_element_type);
}

std::vector<std::vector<int>> CalculiXMeshInterface::get_element_id_type_connectivity()
{
  return CoreMeshQuery(*ccx_core.me_iface).get_element_id_type_connectivity(*ccx_core.cb);
}

std::string CalculiXMeshInterface::get_block_name(int block_id)
{
  return CoreMeshQuery(*ccx_core.me_iface).get_block_name(block_id);
}

std::vector<int> CalculiXMeshInterface::get_block_node_ids(int block_id)
{
  return CoreMeshQuery(*ccx_core.me_iface).get_block_node_ids(block_id);
}

std::vector<int> CalculiXMeshInterface::get_block_element_ids(int block_id)
{
  return CoreMeshQuery(*ccx_core.me_iface).get_block_element_ids(*ccx_core.cb, block_id);
}

std::vector<double> CalculiXMeshInterface::get_block_bounding_box_center(int block_id)
{
  return CoreMeshGeometry::get_block_bounding_box_center(block_id);
}

std::vector<std::vector<double>> CalculiXMeshInterface::get_nodeset_entities_coords(int nodeset_id)
{
  return CoreMeshGeometry::get_nodeset_entities_coords(nodeset_id);
}

std::vector<std::vector<double>> CalculiXMeshInterface::get_sideset_entities_coords(int sideset_id)
{
  return CoreMeshGeometry::get_sideset_entities_coords(sideset_id);
}

std::vector<std::vector<double>> CalculiXMeshInterface::get_sideset_entities_coords_normals(int sideset_id)
{
  return CoreMeshGeometry::get_sideset_entities_coords_normals(sideset_id);
}

std::string CalculiXMeshInterface::get_nodeset_name(int nodeset_id)
{
  return CoreMeshQuery(*ccx_core.me_iface).get_nodeset_name(nodeset_id);
}

std::string CalculiXMeshInterface::get_sideset_name(int sideset_id)
{
  return CoreMeshQuery(*ccx_core.me_iface).get_sideset_name(sideset_id);
}

bool CalculiXMeshInterface::check_block_exists(int block_id)
{
  return ccx_core.cb->check_block_exists(block_id);
}

bool CalculiXMeshInterface::check_global_element_exists(int element_id)
{
  return CoreMeshQuery::check_global_element_exists(element_id);
}

bool CalculiXMeshInterface::check_nodeset_exists(int nodeset_id)
{
  return CoreMeshQuery(*ccx_core.me_iface).check_nodeset_exists(nodeset_id);
}

bool CalculiXMeshInterface::check_sideset_exists(int sideset_id)
{
  return CoreMeshQuery(*ccx_core.me_iface).check_sideset_exists(sideset_id);
}

bool CalculiXMeshInterface::check_vertex_exists(int vertex_id)
{
  return CoreMeshQuery::check_vertex_exists(vertex_id);
}

bool CalculiXMeshInterface::check_curve_exists(int curve_id)
{
  return CoreMeshQuery::check_curve_exists(curve_id);
}

bool CalculiXMeshInterface::check_surface_exists(int surface_id)
{
  return CoreMeshQuery::check_surface_exists(surface_id);
}

bool CalculiXMeshInterface::check_vertex_in_nodeset_exists(int vertex_id, int nodeset_id)
{
  return CoreMeshQuery::check_vertex_in_nodeset_exists(vertex_id, nodeset_id);
}

std::vector<int> CalculiXMeshInterface::get_blocks()
{ 
  return ccx_core.cb->get_blocks();
}

int CalculiXMeshInterface::get_ccx_element_side(int element_type, int side)
{
  return CoreMeshElementMapping::get_ccx_element_side(element_type, side);
}

bool CalculiXMeshInterface::add_sideset_face(std::string sideset_id, std::string sideset_name, std::string face, std::string element_type)
{
  return ccx_core.sideset_faces.add_sideset_face(sideset_id, sideset_name, face, element_type);
}

std::vector<std::vector<std::string>> CalculiXMeshInterface::get_sideset_face(int sideset_id)
{
  return ccx_core.sideset_faces.get_sideset_face(sideset_id);
}

std::vector<std::vector<std::string>> CalculiXMeshInterface::get_entities(std::string entity, int id)
{
  return ccx_core.get_entities(entity,id);
}

std::vector<std::vector<std::string>> CalculiXMeshInterface::get_blocks_tree_data()
{ 
  return CoreMeshTreeData::get_blocks_tree_data(*ccx_core.cb, *ccx_core.me_iface);
}

std::vector<std::vector<std::string>> CalculiXMeshInterface::get_nodeset_tree_data()
{ 
  return CoreMeshTreeData::get_nodeset_tree_data(*ccx_core.me_iface);
}

std::vector<std::vector<std::string>> CalculiXMeshInterface::get_sideset_tree_data()
{ 
  return CoreMeshTreeData::get_sideset_tree_data(*ccx_core.me_iface);
}


bool CalculiXMeshInterface::get_block_properties(int id, MeshProperties& properties, std::string& error)
{
  properties = {};
  error.clear();
  if (ccx_core.me_iface == nullptr || ccx_core.cb == nullptr || !check_block_exists(id))
  {
    error = "The selected block is unavailable.";
    return false;
  }
  properties.id = id;
  properties.name = get_block_name(id);
  properties.elementType = CubitInterface::get_block_element_type(id);
  properties.calculixElementType = get_ccx_element_type(id);
  std::vector<int> nodes, spheres, edges, triangles, faces, pyramids, wedges, tetrahedra, hexahedra;
  if (!CubitInterface::get_block_elements_and_nodes(id, nodes, spheres, edges, triangles, faces, pyramids, wedges, tetrahedra, hexahedra))
  {
    error = "Unable to read the selected block mesh.";
    return false;
  }
  properties.nodeCount = nodes.size();
  properties.elementCount = spheres.size() + edges.size() + triangles.size() + faces.size() + pyramids.size() + wedges.size() + tetrahedra.size() + hexahedra.size();
  return true;
}

bool CalculiXMeshInterface::get_nodeset_properties(int id, MeshProperties& properties, std::string& error)
{
  properties = {};
  error.clear();
  if (ccx_core.me_iface == nullptr || !check_nodeset_exists(id))
  {
    error = "The selected nodeset is unavailable.";
    return false;
  }
  properties.id = id;
  properties.name = get_nodeset_name(id);
  properties.nodeCount = CubitInterface::get_nodeset_nodes_inclusive(id).size();
  return true;
}

bool CalculiXMeshInterface::get_sideset_properties(int id, MeshProperties& properties, std::string& error)
{
  properties = {};
  error.clear();
  if (ccx_core.me_iface == nullptr || !check_sideset_exists(id))
  {
    error = "The selected sideset is unavailable.";
    return false;
  }
  SidesetHandle handle;
  if (!ccx_core.me_iface->get_sideset_handle(id, handle))
  {
    error = "Unable to read the selected sideset.";
    return false;
  }
  properties.id = id;
  properties.name = get_sideset_name(id);
  const int batchSize = 512;
  std::vector<ElementHandle> elements(batchSize);
  std::vector<ElementType> types(batchSize);
  std::vector<int> sides(batchSize);
  int offset = 0;
  int count = 0;
  while ((count = ccx_core.me_iface->get_sideset_sides(handle, offset, batchSize, elements, types, sides)) > 0)
  {
    properties.sideCount += static_cast<std::size_t>(count);
    offset += count;
  }
  if (count < 0)
  {
    error = "Unable to read the sideset faces.";
    return false;
  }
  return true;
}
