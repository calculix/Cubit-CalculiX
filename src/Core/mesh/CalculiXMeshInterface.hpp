#ifndef CALCULIXMESHINTERFACE_HPP
#define CALCULIXMESHINTERFACE_HPP

#include <string>
#include <vector>

class CalculiXMeshInterface
{
public:
  std::vector<std::string> get_ccx_element_types(); // returns all supported ccx element types
  bool set_ccx_element_type(int block_id, std::string ccx_element_type); // sets the ccx element type for a block
  std::string get_ccx_element_type(int block_id); // gets the ccx element type for a block
  int get_ccx_element_type_integration_points(int block_id); // gets the number of integration points for the ccx element type for a block
  std::string get_cubit_element_type_entity(std::string cubit_element_type); // gets the string for a cubit element type
  std::vector<std::vector<int>> get_element_id_type_connectivity(); // gets the element id , type and connectivity all elements
  std::string get_block_name(int block_id); // gets the block name
  std::vector<int> get_block_node_ids(int block_id); // gets the block node ids
  std::vector<int> get_block_element_ids(int block_id); // gets the block global element ids
  std::string get_nodeset_name(int nodeset_id); // gets the nodeset name
  std::string get_sideset_name(int sideset_id); // gets the sideset name
  bool check_block_exists(int block_id);
  bool check_global_element_exists(int element_id);
  bool check_nodeset_exists(int nodeset_id);
  bool check_sideset_exists(int sideset_id);
  bool check_vertex_exists(int vertex_id);
  bool check_curve_exists(int curve_id);
  bool check_surface_exists(int surface_id);
  bool check_vertex_in_nodeset_exists(int vertex_id,int nodeset_id); // checks if the vertex exists in the nodeset
  std::vector<int> get_blocks(); // gets the block ids from core blocks
  int get_ccx_element_side(int element_type,int side); // gets the ccx element side for the given cubit element side
  bool add_sideset_face(std::string sideset_id, std::string sideset_name, std::string face, std::string element_type); // gets the sideset/elset name and face
  std::vector<std::vector<std::string>> get_sideset_face(int sideset_id); // gets the sideset/elset name and face
  std::vector<std::vector<std::string>> get_entities(std::string entity, int id);
  std::vector<std::vector<std::string>> get_blocks_tree_data(); // gets the data from core blocks to build the tree
  std::vector<std::vector<std::string>> get_nodeset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_sideset_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXMESHINTERFACE_HPP
