#ifndef COREBLOCKS_HPP
#define COREBLOCKS_HPP

class CoreBlocks
{

public:
  CoreBlocks();
  ~CoreBlocks();

  std::vector<std::vector<int>> blocks_data; // used to store the connection between a block and its possible options
  // blocks_data[0][0] block_id
  // blocks_data[0][1] block_element_type_cubit
  // blocks_data[0][2] block_element_type_ccx

  std::vector<std::string> ccx_element_types; // all possible element types, the index from the vector will also be the element "id"
  std::vector<std::string> cubit_element_types; // all possible element types, the index from the vector will also be the element "id"
  std::vector<std::pair<std::string,std::string>> cubit_ccx_element_types_conversion; // standard element conversion from cubit to ccx, "-1" if not supported
  std::vector<std::pair<std::string,std::string>> cubit_entity_element_types_conversion; // to get the standard entity type to fetch the global id

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the blocks
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool add_block(int block_id, int cubit_element_type, int ccx_element_type); // adds new block to blocks_data
  bool delete_block(int block_id); // deletes block from blocks_data
  bool modify_block_id(int block_id, int new_block_id); // modifies the block_id from given block
  bool modify_block_element_type_cubit(int block_id, int new_cubit_element_type); // modifies the block_element_type from given block
  bool modify_block_element_type_ccx(int block_id, int new_ccx_element_type); // modifies the block_element_type from given block
  int get_blocks_data_id_from_block_id(int block_id); // searches for the block_id in the blocks_data and returns the indices or -1 if it fails
  std::vector<std::string> get_ccx_element_types(); // returns all supported ccx element types ;
  int get_ccx_element_type_id(std::string ccx_element_type); // returns the index for the ccx element type
  int get_cubit_element_type_id(std::string cubit_element_type); // returns the index for the cubit element type
  int get_standard_ccx_element_type_id(std::string cubit_element_type); // returns the index for the standard conversion ccx element type
  std::string get_ccx_element_type_name(int block_id); // returns the name of the ccx element type
  std::string get_cubit_element_type_entity_name(std::string cubit_element_type); // returns the name of the cubit element type
  std::string print_data(); // prints out the blocks_data
};

#endif // COREBLOCKS_HPP
