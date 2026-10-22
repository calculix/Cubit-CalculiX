#include "CalculiXMeshInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::vector<std::string> CalculiXMeshInterface::get_ccx_element_types()
{ 
  return ccx_core.get_ccx_element_types();
}

bool CalculiXMeshInterface::set_ccx_element_type(int block_id, std::string ccx_element_type)
{
  if (ccx_core.set_ccx_element_type(block_id, ccx_element_type))
  {
    return true;
  }else{
    return false;
  }
}

std::string CalculiXMeshInterface::get_ccx_element_type(int block_id)
{
  return ccx_core.get_ccx_element_type(block_id);
}

int CalculiXMeshInterface::get_ccx_element_type_integration_points(int block_id)
{
  return ccx_core.get_ccx_element_type_integration_points(block_id);
}

std::string CalculiXMeshInterface::get_cubit_element_type_entity(std::string cubit_element_type)
{
  return ccx_core.get_cubit_element_type_entity(cubit_element_type);
}

std::vector<std::vector<int>> CalculiXMeshInterface::get_element_id_type_connectivity()
{
  return ccx_core.get_element_id_type_connectivity();
}

std::string CalculiXMeshInterface::get_block_name(int block_id)
{
  return ccx_core.get_block_name(block_id);
}

std::vector<int> CalculiXMeshInterface::get_block_node_ids(int block_id)
{
  return ccx_core.get_block_node_ids(block_id);
}

std::vector<int> CalculiXMeshInterface::get_block_element_ids(int block_id)
{
  return ccx_core.get_block_element_ids(block_id);
}

std::string CalculiXMeshInterface::get_nodeset_name(int nodeset_id)
{
  return ccx_core.get_nodeset_name(nodeset_id);
}

std::string CalculiXMeshInterface::get_sideset_name(int sideset_id)
{
  return ccx_core.get_sideset_name(sideset_id);
}

bool CalculiXMeshInterface::check_block_exists(int block_id)
{
  return ccx_core.check_block_exists(block_id);
}

bool CalculiXMeshInterface::check_global_element_exists(int element_id)
{
  return ccx_core.check_global_element_exists(element_id);
}

bool CalculiXMeshInterface::check_nodeset_exists(int nodeset_id)
{
  return ccx_core.check_nodeset_exists(nodeset_id);
}

bool CalculiXMeshInterface::check_sideset_exists(int sideset_id)
{
  return ccx_core.check_sideset_exists(sideset_id);
}

bool CalculiXMeshInterface::check_vertex_exists(int vertex_id)
{
  return ccx_core.check_vertex_exists(vertex_id);
}

bool CalculiXMeshInterface::check_curve_exists(int curve_id)
{
  return ccx_core.check_curve_exists(curve_id);
}

bool CalculiXMeshInterface::check_surface_exists(int surface_id)
{
  return ccx_core.check_surface_exists(surface_id);
}

bool CalculiXMeshInterface::check_vertex_in_nodeset_exists(int vertex_id,int nodeset_id)
{
  return ccx_core.check_vertex_in_nodeset_exists(vertex_id,nodeset_id);
}

std::vector<int> CalculiXMeshInterface::get_blocks()
{ 
  return ccx_core.get_blocks();
}

int CalculiXMeshInterface::get_ccx_element_side(int element_type,int side)
{
  return ccx_core.get_ccx_element_side(element_type,side);
}

bool CalculiXMeshInterface::add_sideset_face(std::string sideset_id, std::string sideset_name, std::string face, std::string element_type)
{
  return ccx_core.add_sideset_face(sideset_id, sideset_name, face, element_type);
}

std::vector<std::vector<std::string>> CalculiXMeshInterface::get_sideset_face(int sideset_id)
{
  return ccx_core.get_sideset_face(sideset_id);
}

std::vector<std::vector<std::string>> CalculiXMeshInterface::get_entities(std::string entity, int id)
{
  return ccx_core.get_entities(entity,id);
}

std::vector<std::vector<std::string>> CalculiXMeshInterface::get_blocks_tree_data()
{ 
  return ccx_core.get_blocks_tree_data();
}

std::vector<std::vector<std::string>> CalculiXMeshInterface::get_nodeset_tree_data()
{ 
  return ccx_core.get_nodeset_tree_data();
}

std::vector<std::vector<std::string>> CalculiXMeshInterface::get_sideset_tree_data()
{ 
  return ccx_core.get_sideset_tree_data();
}
