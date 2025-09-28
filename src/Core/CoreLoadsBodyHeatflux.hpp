#ifndef CORELOADSBODYHEATFLUX_HPP
#define CORELOADSBODYHEATFLUX_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

class CoreLoadsBodyHeatflux
{

public:
  CoreLoadsBodyHeatflux();
  ~CoreLoadsBodyHeatflux();

  std::vector<std::vector<int>> loads_data; // used to store the connection between a bodyheatflux and its possible options
  // loads_data[0][0] load_id
  // loads_data[0][1] OP mode: 0 for OP=MOD | 1  for OP=NEW
  // loads_data[0][2] amplitude_id
  // loads_data[0][3] time_delay_id
  // loads_data[0][4] element_id
  // loads_data[0][5] element type -> 1 block | 2 element -> either if the ids contain element or blocks
  // loads_data[0][6] magnitude_id
  // loads_data[0][7] name_id


  std::vector<std::vector<std::string>> time_delay_data;
  // time_delay_data[0][0] time_delay_id
  // time_delay_data[0][1] time_delay_value

  std::vector<std::vector<int>> element_data;
  // elements_data[0][0] element_id
  // elements_data[0][1] id 1
  // elements_data[0][2] id 2
  // elements_data[0][3] id 3 and so on

  std::vector<std::vector<double>> magnitude_data;
  // magnitude_data[0][0] magnitude_id
  // magnitude_data[0][1] magnitude

  std::vector<std::vector<std::string>> name_data;
  // name_data[0][0] name_id
  // name_data[0][1] name

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the blocks
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_load(std::vector<std::string> options, std::vector<int> options2); // adds new load
  bool add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id, int block_id, int element_id, int magnitude_id, int name_id); // adds new load to loads_data
  bool modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2); // modify a load
  bool delete_load(int load_id); // deletes load from loads_data
  bool add_time_delay(std::string time_delay_id, std::string time_delay_value); // adds new time delay to time_delay_data
  bool add_element(int element_id, std::vector<int> elements); // adds new direction to direction_data
  bool add_magnitude(double magnitude_id, double magnitude_value); // adds new magnitude to magnitude_data
  bool add_name(std::string name_id, std::string name); // adds new name to name_data 
  int  get_loads_data_id_from_load_id(int load_id); // searches for the load_id in the loads_data and returns the indices or -1 if it fails
  int  get_time_delay_data_id_from_time_delay_id(int time_delay_id); // searches for the time_delay_id in the time_delay_data and returns the indices or -1 if it fails
  int  get_magnitude_data_id_from_magnitude_id(int magnitude_id); // searches for the magnitude_id in the magnitude_data and returns the indices or -1 if it fails
  int  get_element_data_id_from_element_id(int element_id); // searches for the element_data_id in the elements_data and returns the indices or -1 if it fails
  int  get_name_data_id_from_name_id(int name_id); // searches for the name_id in the name_data and returns the indices or -1 if it fails
  std::vector<int>  get_elements_from_element_id(int element_id); // searches for the element_data_id in the elements_data and returns the indices or -1 if it fails
  std::string get_load_export(int load_id); // gets the optional parameters for the export
  std::string print_data(); // prints out the loads_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORELOADSBODYHEATFLUX_HPP