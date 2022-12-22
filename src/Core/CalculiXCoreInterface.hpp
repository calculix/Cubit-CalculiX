#ifndef CALCULIXCOREINTERFACE_HPP
#define CALCULIXCOREINTERFACE_HPP

class CalculiXCoreInterface
{

public:
  CalculiXCoreInterface();
  ~CalculiXCoreInterface();
	
  std::string print_data();
  bool log_str(std::string str_log);
  std::vector<std::string> get_ccx_element_types(); // returns all supported ccx element types
  bool set_ccx_element_type(int block_id, std::string ccx_element_type); // sets the ccx element type for a block
  std::string get_ccx_element_type(int block_id); // gets the ccx element type for a block
  std::string get_block_name(int block_id); // gets the block name
  bool core_update(); // lets the core check for updates aka changes from the entities
  bool core_reset(); // reset the whole core to the init level
  std::vector<int> get_blocks(); // gets the block ids from core blocks
  std::string get_material_export_data(); // gets the export data from materials core
  bool create_section(std::string section_type,int block_id, std::string material_name, std::vector<std::string> options); // adds a new section
  std::vector<std::vector<std::string>> get_blocks_tree_data(); // gets the data from core blocks to build the tree
  std::vector<std::vector<std::string>> get_nodeset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_sideset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_material_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXCOREINTERFACE_HPP
