#ifndef CALCULIXCORE_HPP
#define CALCULIXCORE_HPP


class CoreBlocks;
class CoreMaterials;
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
  std::vector<std::string> get_ccx_element_types(); // returns all supported ccx element types;
  std::string get_ccx_element_type(int block_id); // gets the ccx element type for a block
  std::string get_block_name(int block_id); // gets the block name
  bool set_ccx_element_type(int block_id, std::string ccx_element_type); // sets the ccx element type for a block
  std::vector<int> get_blocks(); // gets the block ids from core blocks
  std::vector<std::vector<std::string>> get_blocks_tree_data(); // gets the data from core blocks to build the tree
  std::vector<std::vector<std::string>> get_nodeset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_sideset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_material_tree_data(); // gets the data from core to build the tree

  CoreBlocks *cb;
  CoreMaterials *mat;
  MeshExportInterface *me_iface;
  MaterialInterface *mat_iface;
};

#endif // CALCULIXCore_HPP
