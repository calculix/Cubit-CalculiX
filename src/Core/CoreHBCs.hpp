#ifndef COREHBCS_HPP
#define COREHBCS_HPP

class CalculiXCoreInterface;

class CoreHBCs
{

public:
  CoreHBCs();
  ~CoreHBCs();

  std::vector<std::vector<int>> bcs_data;
  // bcs_data[0][0] bcs_id        // always 0
  // bcs_data[0][1] bc_type       // 1: displacement | 2: temperature
  // bcs_data[0][2] bc_id

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  
  bool add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids); // adds bcs to bcs_data
  bool remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids); // removes bcs from bcs_data
  bool add_bc(int bcs_id, int bc_type, int bc_id); // adds new bc to bcs_data
  int  get_bc_data_id(int bcs_id, int bc_type, int bc_id); // searches for the bc_id in the bc_data and returns the indices or -1 if it fails
  std::vector<int> get_bc_data_ids_from_bcs_id(int bcs_id); // searches for the bcs_id in the bcs_data and returns the indices or -1 if it fails
  std::string get_hbc_export(); // get hbcs data export
  std::string print_data(); // prints out the data

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREHBCS_HPP