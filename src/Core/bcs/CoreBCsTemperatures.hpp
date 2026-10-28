#ifndef COREBCSTEMPERATURES_HPP
#define COREBCSTEMPERATURES_HPP

#include <vector>
#include <string>
class CoreBCsTemperatures
{

public:
  CoreBCsTemperatures();
  ~CoreBCsTemperatures();

  std::vector<std::vector<int>> bcs_data; // used to store the connection between a temperature and its possible options
  // bcs_data[0][0] bc_id
  // bcs_data[0][1] OP mode: 0 for OP=MOD | 1  for OP=NEW
  // bcs_data[0][2] amplitude_id
  // bcs_data[0][3] time_delay_id
  // bcs_data[0][4] nodeset_id
  // bcs_data[0][5] magnitude_id
  // bcs_data[0][6] name_id
  // bcs_data[0][7] keyword_type: 0 BOUNDARY, 1 TEMPERATURE

  std::vector<std::vector<std::string>> time_delay_data;
  // time_delay_data[0][0] time_delay_id
  // time_delay_data[0][1] time_delay_value

  std::vector<std::vector<double>> magnitude_data;
  // magnitude_data[0][0] magnitude_id
  // magnitude_data[0][1] magnitude_value

  std::vector<std::vector<std::string>> name_data;
  // name_data[0][0] name_id
  // name_data[0][1] name

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // plugin-owned BCs do not synchronize with Cubit BCs
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_bc(std::vector<std::string> options, std::vector<double> options2); // adds new BC
  bool add_bc(int bc_id, int op_mode, int amplitude_id, int time_delay_id, int nodeset_id, int magnitude_id, int name_id, int keyword_type); // adds new BC to bcs_data
  bool modify_bc(int bc_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker); // modify plugin-owned BC data
  bool delete_bc(int bc_id); // deletes BC from bcs_data
  bool add_time_delay(std::string time_delay_id, std::string time_delay_value); // adds new time delay to time_delay_data
  bool add_magnitude(double magnitude_id, double magnitude_value);
  bool add_name(std::string name_id, std::string name);
  int  get_bcs_data_id_from_bc_id(int bc_id); // searches for the bc_id in the bcs_data and returns the indices or -1 if it fails
  int  get_time_delay_data_id_from_time_delay_id(int time_delay_id); // searches for the time_delay_id in the time_delay_data and returns the indices or -1 if it fails
  int  get_magnitude_data_id_from_magnitude_id(int magnitude_id);
  int  get_name_data_id_from_name_id(int name_id);
  std::string get_bc_parameter_export(int bc_id); // gets the optional parameters for the export
  int get_bc_keyword_type(int bc_id);
  std::string get_bc_export(int bc_id);
  std::string get_hbc_export(int bc_id);
  std::string get_initial_temperature_export(int bc_id);
  std::string print_data(); // prints out the bcs_data

};

#endif // COREBCSTEMPERATURES_HPP