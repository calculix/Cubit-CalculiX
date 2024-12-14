#ifndef CORELOADSRADIATION_HPP
#define CORELOADSRADIATION_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

class CoreLoadsRadiation
{

public:
  CoreLoadsRadiation();
  ~CoreLoadsRadiation();

  std::vector<std::vector<int>> loads_data; // used to store the connection between a gravity and its possible options
  // loads_data[0][0] load_id
  // loads_data[0][1] OP mode: 0 for OP=MOD | 1  for OP=NEW
  // loads_data[0][2] amplitude_id
  // loads_data[0][3] time_delay_id
  // loads_data[0][4] sideset_id
  // loads_data[0][5] temperature_id
  // loads_data[0][6] emissivity_id
  // loads_data[0][7] radiation_amplitude_id
  // loads_data[0][8] radiation_time_delay_id
  // loads_data[0][9] name_id
  // loads_data[0][10] cavity_id
  // loads_data[0][11] CavityRadiation 0 is default 1 use it

  std::vector<std::vector<std::string>> time_delay_data;
  // time_delay_data[0][0] time_delay_id
  // time_delay_data[0][1] time_delay_value

  std::vector<std::vector<std::string>> radiation_time_delay_data;
  // radiation_time_delay_data[0][0] radiation_time_delay_id
  // radiation_time_delay_data[0][1] radiation_time_delay_value

  std::vector<std::vector<std::string>> temperature_data;
  // temperature_data[0][0] temperature_id
  // temperature_data[0][1] value

  std::vector<std::vector<std::string>> emissivity_data;
  // emissivity_data[0][0] emissivity_id
  // emissivity_data[0][1] value

  std::vector<std::vector<std::string>> name_data;
  // name_data[0][0] name_id
  // name_data[0][1] name

  std::vector<std::vector<std::string>> cavity_data;
  // cavity_data[0][0] cavity_id
  // cavity_data[0][1] cavity

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the blocks
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_load(std::vector<std::string> options); // adds new load
  bool add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id, int sideset_id, int temperature_id, int emissivity_id,int radiation_amplitude_id, int radiation_time_delay_id, int name_id, int cavity_id, int cavityradiation); // adds new load to loads_data
  bool modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a load
  bool delete_load(int load_id); // deletes load from loads_data
  bool add_time_delay(std::string time_delay_id, std::string time_delay_value); // adds new time delay to time_delay_data
  bool add_radiation_time_delay(std::string time_delay_id, std::string time_delay_value); // adds new time delay to time_delay_data
  bool add_temperature(std::string temperature_id, std::string temperature_value); // adds new temperature to temperature_data
  bool add_emissivity(std::string emissivity_id, std::string emissivity_value); // adds new emissivity to emissivity_data
  bool add_name(std::string name_id, std::string name); // adds new name to name_data
  bool add_cavity(std::string cavity_id, std::string cavity); // adds new cavity to cavity_data
  int  get_loads_data_id_from_load_id(int load_id); // searches for the load_id in the loads_data and returns the indices or -1 if it fails
  int  get_time_delay_data_id_from_time_delay_id(int time_delay_id); // searches for the time_delay_id in the time_delay_data and returns the indices or -1 if it fails
  int  get_radiation_time_delay_data_id_from_radiation_time_delay_id(int radiation_time_delay_id); // searches for the radiation_time_delay_id in the radiation_time_delay_data and returns the indices or -1 if it fails
  int  get_emissivity_data_id_from_emissivity_id(int emissivity_id); // searches for the emissivity_id in the emissivity_data and returns the indices or -1 if it fails
  int  get_temperature_data_id_from_temperature_id(int temperature_id); // searches for the temperature_id in the temperature_data and returns the indices or -1 if it fails
  int  get_name_data_id_from_name_id(int name_id); // searches for the name_id in the name_data and returns the indices or -1 if it fails
  int  get_cavity_data_id_from_cavity_id(int cavity_id); // searches for the cavity_id in the cavity_data and returns the indices or -1 if it fails
  std::string get_load_export(int load_id); // gets the optional parameters for the export
  std::string print_data(); // prints out the loads_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORELOADSRADIATION_HPP