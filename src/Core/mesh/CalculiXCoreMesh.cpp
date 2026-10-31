#include "Core/CalculiXCore.hpp"
#include "CoreBlocks.hpp"
#include "CoreMeshQuery.hpp"
#include "CoreMeshGeometry.hpp"
#include "CoreMeshElementMapping.hpp"
#include "CoreMeshTreeData.hpp"

std::vector<std::string> CalculiXCore::get_ccx_element_types()
{
  return cb->get_ccx_element_types();
}

bool CalculiXCore::set_ccx_element_type(int block_id, std::string ccx_element_type)
{
  return cb->set_ccx_element_type(block_id, ccx_element_type);
}

std::string CalculiXCore::get_ccx_element_type(int block_id)
{
  return cb->get_ccx_element_type_name(block_id);
}

int CalculiXCore::get_ccx_element_type_integration_points(int block_id)
{
  return cb->get_ccx_element_type_integration_points(block_id);
}

std::string CalculiXCore::get_cubit_element_type_entity(std::string cubit_element_type)
{
  return cb->get_cubit_element_type_entity_name(cubit_element_type);
}

std::vector<std::vector<int>> CalculiXCore::get_element_id_type_connectivity()
{
  return CoreMeshQuery(*me_iface).get_element_id_type_connectivity(*cb);
}

std::string CalculiXCore::get_block_name(int block_id)
{
  return CoreMeshQuery(*me_iface).get_block_name(block_id);
}

std::vector<int> CalculiXCore::get_block_node_ids(int block_id)
{
  return CoreMeshQuery(*me_iface).get_block_node_ids(block_id);
}

std::vector<int> CalculiXCore::get_block_element_ids(int block_id)
{
  return CoreMeshQuery(*me_iface).get_block_element_ids(*cb, block_id);
}

std::vector<double> CalculiXCore::get_block_bounding_box_center(int block_id)
{
  return CoreMeshGeometry::get_block_bounding_box_center(block_id);
}

bool CalculiXCore::check_block_exists(int block_id)
{
  return cb->check_block_exists(block_id);
}

std::vector<int> CalculiXCore::get_blocks()
{
  return cb->get_blocks();
}

std::vector<std::vector<std::string>> CalculiXCore::get_blocks_tree_data()
{
  return CoreMeshTreeData::get_blocks_tree_data(*cb, *me_iface);
}

std::string CalculiXCore::get_nodeset_name(int nodeset_id)
{
  return CoreMeshQuery(*me_iface).get_nodeset_name(nodeset_id);
}

std::vector<std::vector<double>> CalculiXCore::get_nodeset_entities_coords(int nodeset_id)
{
  return CoreMeshGeometry::get_nodeset_entities_coords(nodeset_id);
}

std::string CalculiXCore::get_sideset_name(int sideset_id)
{
  return CoreMeshQuery(*me_iface).get_sideset_name(sideset_id);
}

std::vector<std::vector<double>> CalculiXCore::get_sideset_entities_coords(int sideset_id)
{
  return CoreMeshGeometry::get_sideset_entities_coords(sideset_id);
}

std::vector<std::vector<double>> CalculiXCore::get_sideset_entities_coords_normals(int sideset_id)
{
  return CoreMeshGeometry::get_sideset_entities_coords_normals(sideset_id);
}

bool CalculiXCore::check_global_element_exists(int element_id)
{
  return CoreMeshQuery::check_global_element_exists(element_id);
}

bool CalculiXCore::check_nodeset_exists(int nodeset_id)
{
  return CoreMeshQuery(*me_iface).check_nodeset_exists(nodeset_id);
}

bool CalculiXCore::check_sideset_exists(int sideset_id)
{
  return CoreMeshQuery(*me_iface).check_sideset_exists(sideset_id);
}

bool CalculiXCore::check_vertex_exists(int vertex_id)
{
  return CoreMeshQuery::check_vertex_exists(vertex_id);
}

bool CalculiXCore::check_curve_exists(int curve_id)
{
  return CoreMeshQuery::check_curve_exists(curve_id);
}

bool CalculiXCore::check_surface_exists(int surface_id)
{
  return CoreMeshQuery::check_surface_exists(surface_id);
}

bool CalculiXCore::check_vertex_in_nodeset_exists(int vertex_id, int nodeset_id)
{
  return CoreMeshQuery::check_vertex_in_nodeset_exists(vertex_id, nodeset_id);
}

int CalculiXCore::get_ccx_element_side(int element_type, int side)
{
  return CoreMeshElementMapping::get_ccx_element_side(element_type, side);
}

bool CalculiXCore::add_sideset_face(std::string sideset_id, std::string sideset_name, std::string face, std::string element_type)
{
  return sideset_faces.add_sideset_face(sideset_id, sideset_name, face, element_type);
}

std::vector<std::vector<std::string>> CalculiXCore::get_sideset_face(int sideset_id)
{
  return sideset_faces.get_sideset_face(sideset_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_nodeset_tree_data()
{
  return CoreMeshTreeData::get_nodeset_tree_data(*me_iface);
}

std::vector<std::vector<std::string>> CalculiXCore::get_sideset_tree_data()
{
  return CoreMeshTreeData::get_sideset_tree_data(*me_iface);
}
