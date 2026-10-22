#ifndef COREINITIALCONDITIONS_HPP
#define COREINITIALCONDITIONS_HPP

#include <vector>
#include <string>
class CoreInitialConditions
{

public:
  CoreInitialConditions();
  ~CoreInitialConditions();

  // make 1 data storage for each type
  // Displacement         type 1
  // Temperature          type 2
  // Stress               type 3

  std::vector<std::vector<int>> initialconditions_data; // used to store the connection between a initial condition id and type id
  // initialconditions_data[0][0] initialcondition_id
  // initialconditions_data[0][1] initialcondition_type           
  // initialconditions_data[0][2] initialcondition_type_id     

  std::vector<std::vector<std::string>> displacement_data;
  // displacement_data[0][0] displacement_id
  // displacement_data[0][1] bc_displacement_id

  std::vector<std::vector<std::string>> temperature_data;
  // temperature_data[0][0] temperature_id
  // temperature_data[0][1] bc_temperature_id

  std::vector<int> stress_data;
  // stress_data[0] stress_id

  std::vector<std::vector<double>> stress_element_data;
  // stress_element_data[0][0] stress_id
  // stress_element_data[0][1] element_id
  // stress_element_data[0][2] integration point
  // stress_element_data[0][3] sxx
  // stress_element_data[0][4] syy
  // stress_element_data[0][5] szz
  // stress_element_data[0][6] sxy
  // stress_element_data[0][7] sxz
  // stress_element_data[0][8] syz

  std::vector<std::vector<double>> stress_block_data;
  // stress_block_data[0][0] stress_id
  // stress_block_data[0][1] block_id
  // stress_block_data[0][2] sxx
  // stress_block_data[0][3] syy
  // stress_block_data[0][4] szz
  // stress_block_data[0][5] sxy
  // stress_block_data[0][6] sxz
  // stress_block_data[0][7] syz

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the amplitude
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_initialcondition(std::vector<std::string> options); // adds new initialcondition
  bool modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a initialcondition
  bool add_initialcondition_stress(int initialcondition_id, int modify_type, std::vector<double> options); // adds stress values to a initialcondition stress
  bool delete_initialcondition(int initialcondition_id); // deletes initialcondition from initialconditions_data
  bool add_initialcondition(int initialcondition_id, int initialcondition_type, int initialcondition_type_id); // adds new initialcondition to initialconditions_data
  bool add_displacement(std::string displacement_id); // adds new displacement to displacement_data
  bool add_temperature(std::string temperature_id); // adds new temperature to temperature_data
  bool add_stress(int stress_id); // adds new stress to stress_data
  bool add_stress_element(double stress_id, double element_id, double ip, double sxx, double syy, double szz, double sxy, double sxz, double syz); // adds new stress to stress_element_data
  bool add_stress_block(double stress_id, double block_id, double sxx, double syy, double szz, double sxy, double sxz, double syz); // adds new stress to stress_block_data
  int  get_initialconditions_data_id_from_initialcondition_id(int initialcondition_id); // searches for the initialcondition_id in the initialconditions_data and returns the indices or -1 if it fails
  int  get_displacement_data_id_from_displacement_id(int displacement_id); // searches for the displacement_id in the displacement_data and returns the indices or -1 if it fails
  int  get_temperature_data_id_from_temperature_id(int temperature_id); // searches for the temperature_id in the temperature_data and returns the indices or -1 if it fails
  int  get_stress_data_id_from_stress_id(int stress_id); // searches for the stress_id in the stress_data and returns the indices or -1 if it fails
  std::vector<int>  get_stress_element_data_ids_from_stress_id(int stress_id); // searches for the stress_id in the stress_element_data and returns the indices or empty if it fails
  std::vector<int>  get_stress_block_data_ids_from_stress_id(int stress_id); // searches for the stress_id in the stress_block_data and returns the indices or empty if it fails
  std::string print_data(); // prints out the blocks_data

};

#endif // COREINITIALCONDITIONS_HPP