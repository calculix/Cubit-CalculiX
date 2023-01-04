#ifndef CALCULIXCORE_HPP
#define CALCULIXCORE_HPP


class CoreBlocks;
class CoreMaterials;
class CoreSections;
class CoreConstraints;
class CoreReferencePoints;

class MeshExportInterface;
class MaterialInterface;

class CalculiXCore
{
  
public:
  CalculiXCore();
  ~CalculiXCore();

  bool print_to_log(std::string str_log);
  bool init();
  bool update(); // updates everything to catch changes in the entities
  bool reset(); // delete all data and init new
  std::string print_data();
  std::vector<int> parser(std::string parse_type, std::string parse_string);
  std::vector<int> extractIntegers(std::string str);
  std::vector<std::string> get_ccx_element_types(); // returns all supported ccx element types;
  std::string get_ccx_element_type(int block_id); // gets the ccx element type for a block
  std::string get_block_name(int block_id); // gets the block name
  std::string get_nodeset_name(int nodeset_id); // gets the nodeset name
  std::string get_sideset_name(int sideset_id); // gets the sideset name
  bool set_ccx_element_type(int block_id, std::string ccx_element_type); // sets the ccx element type for a block
  std::vector<int> get_blocks(); // gets the block ids from core blocks
  std::string get_material_export_data(); // gets the export data from materials core
  std::string get_section_export_data(); // gets the export data from sections core
  std::string get_constraint_export_data(); // gets the export data from constraints core
  bool create_section(std::string section_type,int block_id, std::string material_name, std::vector<std::string> options); // adds a new section
  bool modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a section
  bool delete_section(int section_id); // adds a new section
  bool create_constraint(std::string constraint_type, std::vector<std::string> options); // adds a new constraint
  bool modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a constraint
  bool delete_constraint(int constraint_id); // adds a new constraint
  std::vector<int> get_rigidbody_vertex_list(); // get list of rigid body vertices
  bool referencepoints_update_on_export(int max_node_id); // update reference points
  bool referencepoints_reset_on_export(); // reset reference points
  std::string get_referencepoints_export(); // export reference points
  std::string get_referencepoints_export_nodesets(); // export reference points nodesets
  int  referencepoints_get_ref_from_vertex_id(int vertex_id); // get ref_node for the vertex_id  and returns -1 if it fails
  int  referencepoints_get_rot_from_vertex_id(int vertex_id); // get ref_node for the vertex_id  and returns -1 if it fails
  std::vector<std::vector<std::string>> get_blocks_tree_data(); // gets the data from core blocks to build the tree
  std::vector<std::vector<std::string>> get_nodeset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_sideset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_material_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_sections_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_constraints_tree_data(); // gets the data from core to build the tree

  CoreBlocks *cb;
  CoreMaterials *mat;
  CoreSections *sections;
  CoreConstraints *constraints;
  CoreReferencePoints *referencepoints;
  MeshExportInterface *me_iface;
  MaterialInterface *mat_iface;
};

#endif // CALCULIXCore_HPP
