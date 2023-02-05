#ifndef CORESURFACEINTERACTIONS_HPP
#define CORESURFACEINTERACTIONS_HPP

class CalculiXCoreInterface;

class CoreSurfaceInteractions
{

public:
  CoreSurfaceInteractions();
  ~CoreSurfaceInteractions();

  // make 1 data storage for each surface interaction / behavior type , gap conductance, friction
  // surface behavior type 1 - exponential
  // surface behavior type 2 - linear
  // surface behavior type 3 - tabular
  // surface behavior type 4 - tied
  // surface behavior type 5 - hard NO STORAGE NEEDED!
  // gap conductance
  // gap heat generation

  std::vector<std::vector<int>> surfaceinteractions_data; // used to store the connection between a surfaceinteractions id and its other keyword id
  // value -1 means no data connected to the surface interaction! not every option is needed after all
  // surfaceinteractions_data[0][0] surfaceinteraction_id
  // surfaceinteractions_data[0][1] surfaceinteraction_name_id
  // surfaceinteractions_data[0][2] surfacebehavior_type
  // surfaceinteractions_data[0][3] surfacebehavior_type_id
  // surfaceinteractions_data[0][4] gap_conductance_id
  // surfaceinteractions_data[0][5] gap_heat_generation_id
  // surfaceinteractions_data[0][6] friction_id


  std::vector<std::string> surfaceinteraction_name_data; // name
  // surfaceinteraction_name_data[0] name for id 0

  std::vector<std::vector<std::string>> exponential_surfacebehavior_data; // type 1
  // exponential_surfacebehavior_data[0][0] exponential_surfacebehavior_id
  // exponential_surfacebehavior_data[0][1] c0
  // exponential_surfacebehavior_data[0][2] p0
  
  std::vector<std::vector<std::string>> linear_surfacebehavior_data; // type 2
  // linear_surfacebehavior_data[0][0] linear_surfacebehavior_id
  // linear_surfacebehavior_data[0][1] slope K
  // linear_surfacebehavior_data[0][2] sigma_inf
  // linear_surfacebehavior_data[0][3] c0

  std::vector<std::vector<std::string>> tabular_surfacebehavior_data; // type 3 - same id means one entry for 1 line
  // tabular_surfacebehavior_data[0][0] tabular_surfacebehavior_id
  // tabular_surfacebehavior_data[0][1] pressure
  // tabular_surfacebehavior_data[0][2] overclosure

  std::vector<std::vector<std::string>> tied_surfacebehavior_data; // type 4
  // tied_surfacebehavior_data[0][0] tied_surfacebehavior_id
  // tied_surfacebehavior_data[0][1] slope K

  std::vector<std::vector<std::string>> gap_conductance_data; // same id means one entry for 1 line
  // gap_conductance_data[0][0] gap_conductance_id
  // gap_conductance_data[0][1] Conductance
  // gap_conductance_data[0][2] Contact Pressure
  // gap_conductance_data[0][3] Temperature

  std::vector<std::vector<std::string>> gap_heat_generation_data; 
  // gap_heat_generation_data[0][0] gap_heat_generation_id
  // gap_heat_generation_data[0][1] heat conversion factor
  // gap_heat_generation_data[0][2] surface weighing factor
  // gap_heat_generation_data[0][3] differential tangential velocity

  std::vector<std::vector<std::string>> friction_data; 
  // friction_data[0][0] friction_id
  // friction_data[0][1] mü
  // friction_data[0][2] lambda

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the constraints
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2); // adds new surfaceinteraction
  bool modify_surfaceinteraction(std::string modify_type, int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2); // modify a surfaceinteraction
  bool delete_surfaceinteraction(int surfaceinteraction_id); // deletes surfaceinteraction from surfaceinteractions_data
  bool add_surfaceinteraction(int surfaceinteraction_id, std::string surfaceinteraction_name, int surfacebehavior_type, int surfacebehavior_type_id, int gap_conductance_id, int gap_heat_generation_id, int friction_id); // adds new surfaceinteraction to surfaceinteractions_data
  
  bool add_exponential_surfacebehavior(std::string exponential_surfacebehavior_id, std::string constant_1, std::string constant_2); // adds new exponential_surfacebehavior to exponential_surfacebehavior_data
  bool add_linear_surfacebehavior(std::string linear_surfacebehavior_id, std::string constant_1, std::string constant_2, std::string constant_3); // adds new linear_surfacebehavior to linear_surfacebehavior_data
  bool add_tabular_surfacebehavior(std::string tabular_surfacebehavior_id, std::string constant_1, std::string constant_2); // adds new tabular_surfacebehavior to tabular_surfacebehavior_data
  bool add_tied_surfacebehavior(std::string tied_surfacebehavior_id, std::string constant_1); // adds new tied_surfacebehavior to tied_surfacebehavior_data
  bool add_gap_conductance(std::string gap_conductance_id, std::string constant_1, std::string constant_2, std::string constant_3); // adds new gap_conductance to gap_conductance_data
  bool add_gap_heat_generation(std::string gap_heat_generation_id, std::string constant_1, std::string constant_2, std::string constant_3); // adds new gap_heat_generation to gap_heat_generation_data
  bool add_friction(std::string friction_id, std::string constant_1, std::string constant_2); // adds new friction to friction_data
  
  int  get_surfaceinteractions_data_id_from_surfaceinteraction_id(int surfaceinteraction_id); // searches for the surfaceinteraction_id in the surfaceinteractions_data and returns the indices or -1 if it fails
  int  get_exponential_surfacebehavior_data_id_from_exponential_surfacebehavior_id(int exponential_surfacebehavior_id); // searches for the exponential_surfacebehavior_id in the exponential_surfacebehavior_data and returns the indices or -1 if it fails
  int  get_linear_surfacebehavior_data_id_from_linear_surfacebehavior_id(int linear_surfacebehavior_id); // searches for the linear_surfacebehavior_id in the linear_surfacebehavior_data and returns the indices or -1 if it fails
  std::vector<int> get_tabular_surfacebehavior_data_ids_from_tabular_surfacebehavior_id(int tabular_surfacebehavior_id); // searches for the tabular_surfacebehavior_id in the tabular_surfacebehavior_data and returns the indices or -1 if it fails
  int  get_tied_surfacebehavior_data_id_from_tied_surfacebehavior_id(int tied_surfacebehavior_id); // searches for the tied_surfacebehavior_id in the tied_surfacebehavior_data and returns the indices or -1 if it fails
  std::vector<int> get_gap_conductance_data_ids_from_gap_conductance_id(int gap_conductance_id); // searches for the gap_conductance_id in the gap_conductance_data and returns the indices or -1 if it fails
  int  get_gap_heat_generation_data_id_from_gap_heat_generation_id(int gap_heat_generation_id); // searches for the gap_heat_generation_id in the gap_heat_generation_data and returns the indices or -1 if it fails
  int  get_friction_data_id_from_friction_id(int friction_id); // searches for the friction_id in the friction_data and returns the indices or -1 if it fails

  std::string get_surfaceinteractions_export(); // get CalculiX surfaceinteractions exports
  std::string print_data(); // prints out the xxx_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORESURFACEINTERACTIONS_HPP
