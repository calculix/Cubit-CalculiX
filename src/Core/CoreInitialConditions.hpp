#ifndef COREINITIALCONDITIONS_HPP
#define COREINITIALCONDITIONS_HPP

class CalculiXCoreInterface;

class CoreInitialConditions
{

public:
  CoreInitialConditions();
  ~CoreInitialConditions();

  // make 1 data storage for each type
  // Displacement       type 1
  // Temperature        type 2

  std::vector<std::vector<int>> initialconditions_data; // used to store the connection between a amplitude id and amplitude parameter id
  // initialconditions_data[0][0] initialcondition_id
  // initialconditions_data[0][1] initialcondition_type           
  // initialconditions_data[0][2] initialcondition_type_id     

  std::vector<std::vector<std::string>> displacement_data;
  // displacement_data[0][0] displacement_id
  // displacement_data[0][1] bc_displacement_id

  std::vector<std::vector<std::string>> temperature_data;
  // temperature_data[0][0] temperature_id
  // temperature_data[0][1] bc_temperature_id

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the amplitude
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_initialcondition(std::vector<std::string> options); // adds new initialcondition
  bool modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a initialcondition
  bool delete_initialcondition(int initialcondition_id); // deletes initialcondition from initialconditions_data
  bool add_initialcondition(int initialcondition_id, int initialcondition_type, int initialcondition_type_id); // adds new initialcondition to initialconditions_data
  bool add_displacement(std::string displacement_id); // adds new displacement to displacement_data
  bool add_temperature(std::string temperature_id); // adds new temperature to temperature_data
  int  get_initialconditions_data_id_from_initialcondition_id(int initialcondition_id); // searches for the initialcondition_id in the initialconditions_data and returns the indices or -1 if it fails
  int  get_displacement_data_id_from_displacement_id(int displacement_id); // searches for the displacement_id in the displacement_data and returns the indices or -1 if it fails
  int  get_temperature_data_id_from_temperature_id(int temperature_id); // searches for the temperature_id in the temperature_data and returns the indices or -1 if it fails
  std::string get_initialcondition_export(); // get CalculiX initialcondition exports
  std::string print_data(); // prints out the blocks_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREINITIALCONDITIONS_HPP