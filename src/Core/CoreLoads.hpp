#ifndef CORELOADS_HPP
#define CORELOADS_HPP

class CalculiXCoreInterface;

class CoreLoads
{

public:
  CoreLoads();
  ~CoreLoads();

  std::vector<std::vector<int>> loads_data; // used to store the connection between a block and its possible options
  // blocks_data[0][0] block_id
  // blocks_data[0][1] block_element_type_cubit
  // blocks_data[0][2] block_element_type_ccx

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the blocks
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool add_load(int load_id); // adds new load to loads_data
  bool delete_load(int load_id); // deletes load from loads_data
  int  get_loads_data_id_from_load_id(int load_id); // searches for the load_id in the loads_data and returns the indices or -1 if it fails
  std::string print_data(); // prints out the loads_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORELOADS_HPP