#ifndef CALCULIXCORE_HPP
#define CALCULIXCORE_HPP


class CoreBlocks;
class MeshExportInterface;

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
  bool set_ccx_element_type(int block_id, std::string ccx_element_type); // sets the ccx element type for a block
  std::vector<std::vector<std::string>> get_blocks_tree_data(); // gets the data from core blocks to build the tree

  CoreBlocks *cb;
  MeshExportInterface *me_iface;
};

#endif // CALCULIXCore_HPP
