#ifndef CALCULIXSTEPSINTERFACE_HPP
#define CALCULIXSTEPSINTERFACE_HPP

#include <string>
#include <vector>

struct StepSplitResult;

class CalculiXStepsInterface
{
public:
  bool check_step_exists(int step_id); // check if step exists
  std::string get_damping_export_data(); // gets the export data from damping core
  std::string get_physicalconstants_export_data(); // gets the export data from physicalconstants core
  std::string get_initialcondition_export_data(); // gets the export data from core
  std::string get_step_export_data(); // gets the export data from core
  std::vector<int> get_steps_ids(); //gets the step ids from core
  bool modify_damping(std::vector<std::string> options, std::vector<int> options_marker); // modify damping
  bool delete_damping(bool delete_alpha, bool delete_beta); // deletes the values
  bool modify_physicalconstants(std::vector<std::string> options, std::vector<int> options_marker); // modify physical constants
  bool delete_physicalconstants(bool delete_absolute_zero, bool delete_stefan_boltzmann, bool delete_newton_gravity); // deletes the values
  bool create_historyoutput(std::vector<std::string> options); // adds a new output
  bool modify_historyoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a output
  bool delete_historyoutput(int output_id); // delete output
  std::vector<std::string> get_historyoutput_node_keys();
  std::vector<std::string> get_historyoutput_element_keys();
  std::vector<std::string> get_historyoutput_contact_keys();
  std::vector<std::string> get_historyoutput_section_keys();
  bool create_fieldoutput(std::vector<std::string> options); // adds a new output
  bool modify_fieldoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a output
  bool delete_fieldoutput(int output_id); // delete output
  std::vector<std::string> get_fieldoutput_node_keys();
  std::vector<std::string> get_fieldoutput_element_keys();
  std::vector<std::string> get_fieldoutput_contact_keys();
  bool create_initialcondition(std::vector<std::string> options); // adds a new initialcondition
  bool modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a initialcondition
  bool add_initialcondition_stress(int initialcondition_id, int modify_type, std::vector<double> options); // adds stress values to a initialcondition stress
  bool delete_initialcondition(int initialcondition_id); // delete initialcondition
  bool create_step(std::vector<std::string> options); // adds a new step
  bool modify_step(int step_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a step
  bool delete_step(int step_id); // delete step
  bool step_add_loads(int step_id, int load_type, std::vector<int> load_ids); // adds loads to loads_data
  bool step_add_bcs(int step_id, int bc_type, std::vector<int> bc_ids); // adds bcs to bcs_data
  bool step_add_historyoutputs(int step_id, std::vector<int> historyoutput_ids); // adds historyoutputs to historyoutputs_data
  bool step_add_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids); // adds fieldoutputs to fieldoutputs_data
  bool step_remove_loads(int step_id, int load_type, std::vector<int> load_ids); // removes loads from loads_data
  bool step_remove_bcs(int step_id, int bc_type, std::vector<int> bc_ids); // removes bcs from bcs_data
  bool step_remove_historyoutputs(int step_id, std::vector<int> historyoutput_ids); // removes historyoutputs to historyoutputs_data
  bool step_remove_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids); // removes fieldoutputs to fieldoutputs_data
  bool step_utility_modelchangeelement_dummystep(int step_id, std::vector<int> trajectory_ids); // inserts dummysteps
  StepSplitResult step_utility_split_step(int step_id, std::vector<std::vector<double>> times); // split step, returns step ids
  bool step_utility_save_backup(); // saves the current steps data
  bool step_utility_load_backup(); // loads the backup steps data
  std::vector<std::vector<std::string>> get_damping_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_physicalconstants_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_historyoutputs_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_fieldoutputs_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_initialconditions_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadsforces_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadspressures_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadsheatfluxes_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadsgravity_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadscentrifugal_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadstrajectory_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadsfilm_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadsradiation_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadssurfacetraction_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadsbodyheatflux_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_bcsdisplacements_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_bcstemperatures_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_historyoutputs_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_fieldoutputs_tree_data(int step_id); // gets the data from core to build the tree
};

#endif // CALCULIXSTEPSINTERFACE_HPP
