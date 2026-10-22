#ifndef CORELOADSFORCES_HPP
#define CORELOADSFORCES_HPP

#include <vector>
#include <string>
class CoreLoadsForces
{

public:
  CoreLoadsForces();
  ~CoreLoadsForces();

  std::vector<std::vector<int>> loads_data; // used to store the connection between a force and its possible options
  // loads_data[0][0] load_id
  // loads_data[0][1] OP mode: 0 for OP=MOD | 1  for OP=NEW
  // loads_data[0][2] amplitude_id
  // loads_data[0][3] time_delay_id
  // loads_data[0][4] nodeset_id
  // loads_data[0][5] force_id
  // loads_data[0][6] name_id

  std::vector<std::vector<std::string>> time_delay_data;
  // time_delay_data[0][0] time_delay_id
  // time_delay_data[0][1] time_delay_value

  std::vector<std::vector<double>> force_data;
  // force_data[0][0] force_id
  // force_data[0][1..6] force/moment values for dofs 1..6

  std::vector<std::vector<std::string>> name_data;
  // name_data[0][0] name_id
  // name_data[0][1] name

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // plugin-owned loads do not synchronize with Cubit BCs
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_load(std::vector<std::string> options, std::vector<double> options2); // adds new load
  bool add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id, int nodeset_id, int force_id, int name_id); // adds new load to loads_data
  bool modify_load(int load_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker); // modify plugin-owned load data
  bool delete_load(int load_id); // deletes load from loads_data
  bool add_time_delay(std::string time_delay_id, std::string time_delay_value); // adds new time delay to time_delay_data
  bool add_force(double force_id, double dof_1_value, double dof_2_value, double dof_3_value, double dof_4_value, double dof_5_value, double dof_6_value);
  bool add_name(std::string name_id, std::string name);
  int  get_loads_data_id_from_load_id(int load_id); // searches for the load_id in the loads_data and returns the indices or -1 if it fails
  int  get_time_delay_data_id_from_time_delay_id(int time_delay_id); // searches for the time_delay_id in the time_delay_data and returns the indices or -1 if it fails
  int  get_force_data_id_from_force_id(int force_id);
  int  get_name_data_id_from_name_id(int name_id);
  std::string get_load_parameter_export(int load_id); // gets the optional parameters for the export
  std::string get_load_export(int load_id); // gets the complete CLOAD export
  std::string print_data(); // prints out the loads_data

};

#endif // CORELOADSFORCES_HPP
