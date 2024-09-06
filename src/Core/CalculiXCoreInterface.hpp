#ifndef CALCULIXCOREINTERFACE_HPP
#define CALCULIXCOREINTERFACE_HPP

#include <QIcon>

class MeshExportInterface;

class CalculiXCoreInterface
{

public:
  CalculiXCoreInterface();
  ~CalculiXCoreInterface();
	
  bool cmd(std::string cmd); // executes a cubit command with appending to the history
  bool set_use_ccx_autocleanup(bool bool_use);
  bool set_use_ccx_logfile(bool bool_use);
  bool init_pythoninterface();
  std::string print_data();
  bool log_str(std::string str_log);
  bool export_to_csv(std::string path_and_name, std::vector<std::string> header, std::vector<std::vector<double>> data); // exports the data to a .csv file, with the header
  std::vector<int> parser(std::string parse_type, std::string parse_string);
  std::string to_string_scientific(double value, int precision = 6); // converts a double to string with scientific notation, with optional precision
  double string_scientific_to_double(std::string value, int precision = 6); // converts a string with scientific notation to double, with optional precision
  std::vector<std::string> get_ccx_element_types(); // returns all supported ccx element types
  bool set_ccx_element_type(int block_id, std::string ccx_element_type); // sets the ccx element type for a block
  std::string get_ccx_element_type(int block_id); // gets the ccx element type for a block
  std::string get_cubit_element_type_entity(std::string cubit_element_type); // gets the string for a cubit element type
  std::vector<std::vector<int>> get_element_id_type_connectivity(); // gets the element id , type and connectivity all elements
  std::string get_block_name(int block_id); // gets the block name
  std::vector<int> get_block_node_ids(int block_id); // gets the block node ids
  std::vector<int> get_block_element_ids(int block_id); // gets the block global element ids
  std::string get_material_name(int material_id); // gets the material name
  std::string get_nodeset_name(int nodeset_id); // gets the nodeset name
  std::string get_sideset_name(int sideset_id); // gets the sideset name
  std::string get_surfaceinteraction_name(int surfaceinteraction_id); // gets the surfaceinteraction name
  std::vector<std::string> get_contactpair_master_slave(int contactpair_id); // gets the contactpair master and slave name
  std::string get_amplitude_name(int amplitude_id); // gets the amplitude name
  std::string get_orientation_name(int orientation_id); // gets the orientation name
  std::vector<int> get_loadsforces_ids(); // get all load forces ids
  std::vector<int> get_loadspressures_ids(); // get all load pressure ids
  std::vector<int> get_loadsheatfluxes_ids(); // get all load heatflux ids
  std::vector<int> get_loadsgravity_ids(); // get all load gravity ids
  std::vector<int> get_loadscentrifugal_ids(); // get all load centrifugal ids
  std::vector<int> get_bcsdisplacements_ids(); // get all bc displacement ids
  std::vector<int> get_bcstemperatures_ids(); // get all bc temperature ids
  std::vector<int> get_orientations_ids(); // get all orientation ids
  bool check_block_exists(int block_id);
  bool check_nodeset_exists(int nodeset_id);
  bool check_sideset_exists(int sideset_id);
  bool check_vertex_in_nodeset_exists(int vertex_id,int nodeset_id); // checks if the vertex exists in the nodeset
  bool check_orientation_exists(int orientation_id); // check if orientation exists
  bool check_step_exists(int step_id); // check if step exists
  bool core_update(); // lets the core check for updates aka changes from the entities
  bool core_reset(); // reset the whole core to the init level
  std::vector<int> get_blocks(); // gets the block ids from core blocks
  int get_ccx_element_side(int element_type,int side); // gets the ccx element side for the given cubit element side
  bool add_sideset_face(std::string sideset_id, std::string sideset_name, std::string face, std::string element_type); // gets the sideset/elset name and face
  std::vector<std::vector<std::string>> get_sideset_face(int sideset_id); // gets the sideset/elset name and face
  std::string get_material_export_data(); // gets the export data from materials core
  std::string get_section_export_data(); // gets the export data from sections core
  std::string get_constraint_export_data(); // gets the export data from constraints core
  std::string get_surfaceinteraction_export_data(); // gets the export data from surfaceinteractions core
  std::string get_contactpair_export_data(); // gets the export data from contactpairs core
  std::string get_amplitude_export_data(); // gets the export data from amplitudes core
  std::string get_orientation_export_data(); // gets the export data from orientations core
  std::string get_initialcondition_export_data(); // gets the export data from core
  std::string get_hbc_export_data(); // gets the export data from core
  std::string get_step_export_data(); // gets the export data from core
  std::vector<int> get_steps_ids(); //gets the step ids from core
  bool create_section(std::string section_type,int block_id, int material_id, std::vector<std::string> options); // adds a new section
  bool modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a section  
  bool delete_section(int section_id); // delete section
  bool create_constraint(std::string constraint_type, std::vector<std::string> options); // adds a new constraint
  bool modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a constraint  
  bool delete_constraint(int constraint_id); // delete constraint
  bool create_constraint_tie_from_cubitcontactpair(std::string name, std::string position_tolerance); // create constraint tie from cubit contact pairs
  bool create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2); // adds a new surfaceinteraction
  bool modify_surfaceinteraction(std::string modify_type, int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2); // modifies a surfaceinteraction
  bool delete_surfaceinteraction(int surfaceinteraction_id); // delete surface interaction
  bool create_contactpair(std::vector<std::string> options); // adds a new contactpair
  bool modify_contactpair(int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a contactpair  
  bool delete_contactpair(int contactpair_id); // delete contact pair
  bool create_contactpair_from_cubitcontactpair(int surfaceinteraction_id, std::string contactpairtype, std::string adjust, std::string adjust_nodeset); // create contact pairs from cubit contact pairs with given surface interaction
  bool create_amplitude(std::vector<std::string> options, std::vector<std::vector<std::string>> options2); // adds a new amplitude
  bool modify_amplitude(int amplitude_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2); // modify a amplitude
  bool delete_amplitude(int amplitude_id); // delete amplitude
  bool create_orientation(std::vector<std::string> options, std::vector<std::vector<std::string>> options2); // adds a new orientation
  bool modify_orientation(int orientation_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2); // modify a orientation
  bool delete_orientation(int orientation_id); // delete orientation
  bool modify_loadsforces(int force_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a force
  bool modify_loadspressures(int pressure_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a pressure
  bool modify_loadsheatfluxes(int heatflux_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a heatflux
  bool create_loadsgravity(std::vector<std::string> options); // adds a new gravity load
  bool modify_loadsgravity(int gravity_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a gravity
  bool delete_loadsgravity(int gravity_id); // delete gravity load
  bool create_loadscentrifugal(std::vector<std::string> options); // adds a new centrifugal load
  bool modify_loadscentrifugal(int centrifugal_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a centrifugal
  bool delete_loadscentrifugal(int centrifugal_id); // delete centrifugal load
  bool modify_bcsdisplacements(int displacement_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a displacement
  bool modify_bcstemperatures(int displacement_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a temperature
  bool create_historyoutput(std::vector<std::string> options); // adds a new output
  bool modify_historyoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a output
  bool delete_historyoutput(int output_id); // delete output
  std::vector<std::string> get_historyoutput_node_keys();
  std::vector<std::string> get_historyoutput_element_keys();
  std::vector<std::string> get_historyoutput_contact_keys();
  bool create_fieldoutput(std::vector<std::string> options); // adds a new output
  bool modify_fieldoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a output
  bool delete_fieldoutput(int output_id); // delete output
  std::vector<std::string> get_fieldoutput_node_keys();
  std::vector<std::string> get_fieldoutput_element_keys();
  std::vector<std::string> get_fieldoutput_contact_keys();
  bool create_initialcondition(std::vector<std::string> options); // adds a new initialcondition
  bool modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker); // modify a initialcondition
  bool delete_initialcondition(int initialcondition_id); // delete initialcondition
  bool hbc_add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids); // adds bcs to bcs_data
  bool hbc_remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids); // removes bcs from bcs_data
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
  bool create_job(std::vector<std::string> options); // adds a new job
  bool modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a job
  bool delete_job(int job_id); // delete job
  bool run_job(int job_id, int option); // runs job
  bool check_jobs(); // checks for changes of job processes
  bool wait_job(int job_id); // runs job
  bool kill_job(int job_id); // runs job
  bool set_job_conversion(int job_id, int conversion); // sets the paraview conversion value for the job
  bool result_cgx_job(int job_id); // opens the results with cgx
  bool result_paraview_job(int job_id); // opens the results with paraview
  bool result_plot_job_frd(int job_id,int x_node_id, std::string x_block_type, std::string x_block_component, bool x_increment,bool x_time,int y_node_id, std::string y_block_type, std::string y_block_component, bool y_increment, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath); // plots the results
  bool result_plot_job_dat(int job_id); // plots the results
  bool result_csv_job_frd(int job_id,int x_node_id, std::string x_block_type, std::string x_block_component, bool x_increment,bool x_time,int y_node_id, std::string y_block_type, std::string y_block_component, bool y_increment, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath); // plots the results
  std::vector<std::string> get_job_data(int job_id);
  std::vector<std::string> get_job_console_output(int job_id);
  std::vector<std::string> get_job_cvg(int job_id);
  std::vector<std::string> get_job_sta(int job_id);
  int get_job_status(int job_id);
  bool create_result(int job_id); // adds a new result for job
  bool delete_result(int job_id); // delete result for job
  bool load_result(int job_id); // load result for job
  int convert_result(int job_id, std::vector<int> options, std::vector<int> block_ids, std::vector<int> nodeset_ids, std::vector<int> sideset_ids); // convert result for job
  bool project_result(int job_id,int step,int totalincrement,double scale); // project disp result for job for specific step or increment
  double compute_von_mises_stress(std::vector<double> vec); // computes the von mises stress/strain for a vector of values
  double compute_von_mises_strain(std::vector<double> vec); // computes the von mises stress/strain for a vector of values
  std::vector<double> compute_principal_stresses(std::vector<double> vec); // computes the principal stress for a vector/matrix of stresses
  std::vector<double> compute_principal_strains(std::vector<double> vec); // computes the principal stress for a vector/matrix of strains
  bool create_customline(std::vector<std::string> options); // adds a new customline
  bool modify_customline(int customline_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a customline
  bool delete_customline(int customline_id); // delete customline
  std::vector<std::vector<std::string>> get_entities(std::string entity, int id);
  std::vector<std::string> get_customline_data(std::string position,std::string keyword,int keyword_id);
  //draw
  std::vector<std::vector<double>> get_draw_data_for_load_force(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_pressure(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_heatflux(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_gravity(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_load_centrifugal(int id); // returns coord(3) and magnitude(3) std::vector<double>
  std::vector<std::vector<double>> get_draw_data_for_bc_displacement(int id); // returns coord(3) and dof
  std::vector<std::vector<double>> get_draw_data_for_bc_temperature(int id); // returns coord(3) and dof
  std::vector<std::vector<double>> get_draw_data_for_orientation(int id); // returns pairs of 4 for {system_type,local_axis_angle}, coord(3) of section center, a_coord(3) ,b_coord(3)
  bool draw_all(double size); // draw all loads,bcs,orientations
  bool draw_load_force(std::vector<int> force_ids,double size); // draw load force
  bool draw_load_pressure(std::vector<int> pressure_ids,double size); // draw load pressure
  bool draw_load_heatflux(std::vector<int> heatflux_ids,double size); // draw load heatflux
  bool draw_load_gravity(std::vector<int> gravity_ids,double size); // draw load gravity
  bool draw_load_centrifugal(std::vector<int> centrifugal_ids,double size); // draw load centrifugal
  bool draw_bc_displacement(std::vector<int> displacement_ids,double size); // draw bc displacement
  bool draw_bc_temperature(std::vector<int> temperature_ids,double size); // draw bc temperature
  bool draw_orientation(std::vector<int> orientation_ids,double size); // draw orientation
  bool draw_loads(double size); // draw all loads
  bool draw_bcs(double size); // draw all bcs
  bool draw_orientations(double size); // draw all orientations
  bool draw_load_forces(double size); //draw all forces
  bool draw_load_pressures(double size); //draw all pressures
  bool draw_load_heatfluxes(double size); //draw all heatfluxes
  bool draw_load_gravities(double size); //draw all gravities
  bool draw_load_centrifugals(double size); //draw all centrifugals
  bool draw_bc_displacements(double size); //draw all displacements
  bool draw_bc_temperatures(double size); //draw all temperatures


  //QUERY results
  //FRD results
  std::vector<int> frd_get_nodes(int job_id); // returns a list of all nodes
  bool frd_check_node_exists(int job_id,int node_id); // returns if a node in the frd exists
  std::vector<std::string> frd_get_result_block_types(int job_id); // returns a list of all result block types
  std::vector<std::string> frd_get_result_block_components(int job_id, std::string result_block_type); // returns a list of all result block components for a block type
  std::vector<int> frd_get_total_increments(int job_id); // returns a list of the total increments
  double frd_get_time_from_total_increment(int job_id, int total_increment); // returns a the time for a total increment
  std::vector<int> frd_get_node_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value); // returns the global node ids within the values
  std::vector<int> frd_get_node_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global node ids smaller than the value
  std::vector<int> frd_get_node_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global node ids greater than the value
  std::vector<int> frd_get_element_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value); // returns the global element ids within the values
  std::vector<int> frd_get_element_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global element ids smaller than the values
  std::vector<int> frd_get_element_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value); // returns the global element ids greater than the values  
  std::vector<int> frd_get_element_ids_over_limit(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double limit); // returns the global element ids where the largest difference between nodal values exceeds the limit
  double frd_get_node_value(int job_id,int node_id, int total_increment,std::string result_block_type,std::string result_block_component); // returns the queried node_id value or zero if no value exists
  std::vector<double> frd_get_node_values(int job_id,int node_id, int total_increment,std::string result_block_type); // returns the queried node_id values
  //DAT results
  std::vector<std::string> dat_get_result_block_types(int job_id); // returns a list of all result block types
  std::vector<std::string> dat_get_result_block_set(int job_id); // returns a list of all result block set
  std::vector<std::string> dat_get_result_block_components(int job_id, std::string result_block_type); // returns a list of all result block components for a block type and set
  std::vector<double> dat_get_result_block_times(int job_id, std::string result_block_type, std::string result_block_set); // returns a list of all result block components for a block type and set
  std::vector<int> dat_get_node_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value); // returns the global node ids within the values
  std::vector<int> dat_get_node_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global node ids smaller than the value
  std::vector<int> dat_get_node_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global node ids greater than the value
  std::vector<int> dat_get_element_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value); // returns the global element ids within the values
  std::vector<int> dat_get_element_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global element ids smaller than the values
  std::vector<int> dat_get_element_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value); // returns the global element ids greater than the values  
  std::vector<int> dat_get_element_ids_over_limit(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double limit); // returns the global element ids where the largest difference between element values exceeds the limit
  double dat_get_node_value(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component); // returns the queried node_id value or zero if no value exists
  std::vector<double> dat_get_node_values(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set); // returns the queried node_id values
  std::vector<double> dat_get_element_values_for_component(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component); // returns the queried element integration point values for a component
  std::vector<std::vector<double>> dat_get_element_values(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set); // returns the queried element integration point values for all components
  std::vector<std::vector<std::vector<double>>> dat_get_buckle(int job_id); // returns the buckling data for a job

  // GUI
  QIcon* getIcon(std::string name);
  QIcon getIcon2(std::string name);
  std::vector<std::vector<std::string>> get_blocks_tree_data(); // gets the data from core blocks to build the tree
  std::vector<std::vector<std::string>> get_nodeset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_sideset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_material_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_material_group_properties(); // gets the data from core to build the item in the material management
  std::vector<std::vector<std::string>> get_sections_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_constraints_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_surfaceinteractions_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_contactpairs_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_amplitudes_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_orientations_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadsforces_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadspressures_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadsheatfluxes_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadsgravity_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadscentrifugal_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_bcsdisplacements_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_bcstemperatures_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_historyoutputs_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_fieldoutputs_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_initialconditions_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_hbcsdisplacements_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_hbcstemperatures_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadsforces_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadspressures_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadsheatfluxes_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadsgravity_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_loadscentrifugal_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_bcsdisplacements_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_bcstemperatures_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_historyoutputs_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_steps_fieldoutputs_tree_data(int step_id); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_customlines_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_jobs_tree_data(); // gets the data from core to build the tree

  MeshExportInterface *me_iface;
};

#endif // CALCULIXCOREINTERFACE_HPP
