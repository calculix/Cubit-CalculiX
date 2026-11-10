#ifndef CALCULIXCORE_HPP
#define CALCULIXCORE_HPP

#include <vector>
#include <cstddef>
#include <memory>
#include "Core/results/dat/CoreResultsDatStructs.hpp"
#include <string>
#include <set>
#include <map>
#include <array>
#include <QIcon>
#include <QString>

#include "Core/jobs/CoreJobExportContext.hpp"
#include "Core/mesh/CoreSidesetFaces.hpp"
#include "Core/steps/CoreStepsStructs.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include "Framework/Core/CubitSimVersion.hpp"

class CoreTopologyLevelSetGeometryCache;
class CoreBlocks;
class CoreMaterials;
class CoreMaterialsLibrary;
class CoreSections;
class CoreConstraints;
class CoreSurfaceInteractions;
class CoreContactPairs;
class CoreAmplitudes;
class CoreOrientations;
class CoreDamping;
class CorePhysicalConstants;
class CoreLoadsForces;
class CoreLoadsPressures;
class CoreLoadsHeatfluxes;
class CoreLoadsGravity;
class CoreLoadsCentrifugal;
class CoreLoadsTrajectory;
class CoreLoadsFilm;
class CoreLoadsRadiation;
class CoreLoadsSurfaceTraction;
class CoreLoadsBodyHeatflux;
struct CoreLoadsQueryData;
class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreHistoryOutputs;
class CoreFieldOutputs;
class CoreInitialConditions;
class CoreHBCs;
class CoreSteps;
class CoreJobs;
class CoreResults;
class CoreTimer;
class CoreTopology;
class CoreTopologyOptimization;
class CoreCustomLines;
class CoreDraw;

class MeshExportInterface;
class MaterialInterface;

class CalculiXCore
{
  
public:
  CalculiXCore();
  ~CalculiXCore();

  CalculiXCore(const CalculiXCore&) = delete;
  CalculiXCore& operator=(const CalculiXCore&) = delete;
  CalculiXCore(CalculiXCore&&) = delete;
  CalculiXCore& operator=(CalculiXCore&&) = delete;

  bool bool_init = false;
  bool bool_init2 = false;
  bool bool_init_pythoninterface = false;
  bool bool_init_materiallibrary = false;
  bool bool_gui_updated = false;
  bool bool_block_core_update = false;
  bool bool_block_gui_update = false;
  bool use_ccx_logfile = false;
  bool use_ccx_autocleanup = true;
  std::string version = CubitSim::Version;
  CoreSidesetFaces sideset_faces;
  bool checking_jobs = false;
  unsigned int silent_cmd_depth = 0;
  using JobExportContext = CoreJobExportContext;

  JobExportContext job_export_context;

  // Initialization
  bool init();
  bool init2();
  bool init_pythoninterface();
  bool init_materiallibrary();
  bool init_completed();

  // Maintenance
  bool update();
  bool reset();
  std::string autocleanup();
  std::string print_data();

  // Persistence
  bool read_cub(std::string filename);
  bool save_cub(std::string filename);

  // Application state
  std::string get_version();
  bool is_checking_jobs() const;
  bool gui_updated();
  bool set_gui_updated(bool status);
  bool block_core_update();
  bool set_block_core_update(bool status);
  bool block_gui_update();
  bool set_block_gui_update(bool status);

  // Commands and logging
  bool cmd(std::string cmd); // Executes a Cubit command and records it in history.
  bool silent_cmd(std::string cmd); // Executes a Cubit command within the silent-command scope.
  bool is_silent_cmd() const;
  bool print_to_log(std::string str_log);
  bool print_to_cl(std::string str_log);

  // Selection
  std::vector<int> parser(std::string parse_type, std::string parse_string);
  std::vector<int> extractIntegers(std::string str);

  // Shared entity queries
  bool check_bc_exists(int bc_id, int BCType); // Checks a boundary-condition or load ID using the shared type mapping.
  std::vector<std::vector<std::string>> get_entities(std::string entity, int id); // Returns referenced entities for highlighting.

  // Utilities
  bool export_to_csv(std::string path_and_name, std::vector<std::string> header, std::vector<std::vector<double>> data, bool overwrite);
  std::string to_string_scientific(double value, int precision = 6);
  double string_scientific_to_double(std::string value, int precision = 6);

  // Mesh: element types and connectivity
  std::vector<std::string> get_ccx_element_types();
  bool set_ccx_element_type(int block_id, std::string ccx_element_type);
  std::string get_ccx_element_type(int block_id);
  int get_ccx_element_type_integration_points(int block_id);
  std::string get_cubit_element_type_entity(std::string cubit_element_type);
  int get_ccx_element_side(int element_type, int side);
  std::vector<std::vector<int>> get_element_id_type_connectivity();

  // Mesh: blocks
  std::vector<int> get_blocks();
  std::string get_block_name(int block_id);
  std::vector<int> get_block_node_ids(int block_id);
  std::vector<int> get_block_element_ids(int block_id);
  std::vector<double> get_block_bounding_box_center(int block_id);

  // Mesh: nodesets and sidesets
  std::string get_nodeset_name(int nodeset_id);
  std::vector<std::vector<double>> get_nodeset_entities_coords(int nodeset_id);
  std::string get_sideset_name(int sideset_id);
  std::vector<std::vector<double>> get_sideset_entities_coords(int sideset_id);
  std::vector<std::vector<double>> get_sideset_entities_coords_normals(int sideset_id);
  bool add_sideset_face(std::string sideset_id, std::string sideset_name, std::string face, std::string element_type);
  std::vector<std::vector<std::string>> get_sideset_face(int sideset_id);

  // Mesh: existence checks
  bool check_block_exists(int block_id);
  bool check_global_element_exists(int element_id);
  bool check_nodeset_exists(int nodeset_id);
  bool check_sideset_exists(int sideset_id);
  bool check_vertex_exists(int vertex_id);
  bool check_curve_exists(int curve_id);
  bool check_surface_exists(int surface_id);
  bool check_vertex_in_nodeset_exists(int vertex_id, int nodeset_id);

  // Mesh: tree data
  std::vector<std::vector<std::string>> get_blocks_tree_data();
  std::vector<std::vector<std::string>> get_nodeset_tree_data();
  std::vector<std::vector<std::string>> get_sideset_tree_data();

  // Materials
  std::string get_material_name(int material_id);
  std::string get_material_prefix(std::string material_name);
  int get_group_property_size(std::string group_property);
  bool check_material_exists(int material_id);
  std::vector<std::vector<std::string>> get_material_tree_data();
  std::vector<std::vector<std::string>> get_material_group_properties();
  std::string get_material_export_data(int topology_id = -1, int iteration_id = -1);

  // Material library groups
  bool create_materiallibrary_group(std::string name);
  bool modify_materiallibrary_group(std::string name, std::string data, int mode);
  bool delete_materiallibrary_group(std::string name);

  // Material library materials
  bool create_materiallibrary_material(std::string name, std::string groupname);
  bool modify_materiallibrary_material(std::string name, std::string groupname, std::string data, int mode, std::vector<double> value_data);
  bool delete_materiallibrary_material(std::string name, std::string groupname);
  bool export_materiallibrary_material(std::string name, std::string groupname, std::string cubit_name);
  bool import_materiallibrary_material(std::string name, std::string groupname, std::string cubit_name);

  // Material library queries
  std::vector<std::vector<std::string>> get_materiallibrary_tree_data();
  std::vector<std::vector<std::string>> get_materiallibrary_material_properties(std::string name, std::string group);
  std::vector<std::vector<double>> get_materiallibrary_material_values(std::string name, std::string group, std::string property);

  // Sections
  bool create_section(std::string section_type, int block_id, int material_id, std::vector<std::string> options);
  bool modify_section(std::string section_type, int section_id, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_section(int section_id);
  std::vector<std::vector<std::string>> get_sections_tree_data();
  std::string get_section_export_data(int topology_id = -1, int iteration_id = -1);

  // Orientations
  std::string get_orientation_name(int orientation_id);
  std::vector<int> get_orientations_ids();
  bool check_orientation_exists(int orientation_id);
  bool create_orientation(std::vector<std::string> options, std::vector<std::vector<std::string>> options2);
  bool modify_orientation(int orientation_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2);
  bool delete_orientation(int orientation_id);
  std::vector<std::vector<std::string>> get_orientations_tree_data();
  std::string get_orientation_export_data();

  // Constraints
  std::vector<int> get_equation_ids();
  std::vector<int> get_equationgroup_ids();
  bool create_constraint(std::string constraint_type, std::vector<std::string> options, std::vector<std::vector<double>> options2);
  bool modify_constraint(std::string constraint_type, int constraint_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<double>> options2);
  bool delete_constraint(int constraint_id);
  bool create_constraint_tie_from_cubitcontactpair(std::string name, std::string position_tolerance);
  bool create_constraint_equation_from_coincident_nodes(std::string name, int group_id, double tolerance, bool dof_1, bool dof_2, bool dof_3);
  std::vector<std::vector<std::string>> get_constraints_tree_data();
  std::string get_constraint_export_data();

  // Surface interactions
  std::string get_surfaceinteraction_name(int surfaceinteraction_id);
  bool check_surfaceinteraction_exists(int surfaceinteraction_id);
  bool create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2);
  bool modify_surfaceinteraction(std::string modify_type, int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2);
  bool delete_surfaceinteraction(int surfaceinteraction_id);
  std::vector<std::vector<std::string>> get_surfaceinteractions_tree_data();
  std::string get_surfaceinteraction_export_data();

  // Contact pairs
  std::vector<std::string> get_contactpair_master_slave(int contactpair_id);
  bool check_contactpair_exists(int contactpair_id);
  bool create_contactpair(std::vector<std::string> options);
  bool modify_contactpair(int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_contactpair(int contactpair_id);
  bool create_contactpair_from_cubitcontactpair(int surfaceinteraction_id, std::string contactpairtype, std::string adjust, std::string adjust_nodeset);
  std::vector<std::vector<std::string>> get_contactpairs_tree_data();
  std::string get_contactpair_export_data();

  // Amplitudes
  std::string get_amplitude_name(int amplitude_id);
  bool check_amplitude_exists(int amplitude_id);
  bool create_amplitude(std::vector<std::string> options, std::vector<std::vector<std::string>> options2);
  bool modify_amplitude(int amplitude_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2);
  bool delete_amplitude(int amplitude_id);
  bool delete_amplitudes(std::vector<int> amplitude_ids);
  std::vector<std::vector<std::string>> get_amplitudes_tree_data();
  std::string get_amplitude_export_data();

  // Damping
  bool modify_damping(std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_damping(bool delete_alpha, bool delete_beta);
  std::vector<std::vector<std::string>> get_damping_tree_data();
  std::string get_damping_export_data();

  // Physical constants
  bool modify_physicalconstants(std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_physicalconstants(bool delete_absolute_zero, bool delete_stefan_boltzmann, bool delete_newton_gravity);
  std::vector<std::vector<std::string>> get_physicalconstants_tree_data();
  std::string get_physicalconstants_export_data();

  // Custom lines
  std::vector<int> get_customline_ids();
  bool create_customline(std::vector<std::string> options);
  bool modify_customline(int customline_id, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_customline(int customline_id);
  std::vector<std::string> get_customline_data(std::string position, std::string keyword, int keyword_id);
  std::vector<std::vector<std::string>> get_customlines_tree_data();

  // Displacement boundary conditions
  std::vector<int> get_bcsdisplacements_ids();
  bool create_bcsdisplacements(std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3);
  bool modify_bcsdisplacements(int displacement_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3, std::vector<int> options_marker);
  bool delete_bcsdisplacements(int displacement_id);
  std::vector<std::vector<std::string>> get_bcsdisplacements_tree_data();

  // Temperature boundary conditions
  std::vector<int> get_bcstemperatures_ids();
  bool create_bcstemperatures(std::vector<std::string> options, std::vector<double> options2);
  bool modify_bcstemperatures(int temperature_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker);
  bool delete_bcstemperatures(int temperature_id);
  std::vector<std::vector<std::string>> get_bcstemperatures_tree_data();

  // HBC assignments
  bool hbc_add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids);
  bool hbc_remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids);
  std::vector<std::vector<std::string>> get_hbcsdisplacements_tree_data();
  std::vector<std::vector<std::string>> get_hbcstemperatures_tree_data();
  std::string get_hbc_export_data();

  // Initial conditions
  bool create_initialcondition(std::vector<std::string> options);
  bool modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker);
  bool add_initialcondition_stress(int initialcondition_id, int modify_type, std::vector<double> options);
  bool delete_initialcondition(int initialcondition_id);
  std::vector<std::vector<std::string>> get_initialconditions_tree_data();
  std::string get_initialcondition_export_data();

  // History outputs
  bool check_historyoutput_exists(int historyoutput_id);
  bool create_historyoutput(std::vector<std::string> options);
  bool modify_historyoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_historyoutput(int output_id);
  std::vector<std::string> get_historyoutput_node_keys();
  std::vector<std::string> get_historyoutput_element_keys();
  std::vector<std::string> get_historyoutput_contact_keys();
  std::vector<std::string> get_historyoutput_section_keys();
  std::vector<std::vector<std::string>> get_historyoutputs_tree_data();

  // Field outputs
  bool check_fieldoutput_exists(int fieldoutput_id);
  bool create_fieldoutput(std::vector<std::string> options);
  bool modify_fieldoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_fieldoutput(int output_id);
  std::vector<std::string> get_fieldoutput_node_keys();
  std::vector<std::string> get_fieldoutput_element_keys();
  std::vector<std::string> get_fieldoutput_contact_keys();
  std::vector<std::vector<std::string>> get_fieldoutputs_tree_data();

  // Steps
  std::vector<int> get_steps_ids();
  bool check_step_exists(int step_id);
  bool create_step(std::vector<std::string> options);
  bool modify_step(int step_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_step(int step_id);

  // Step assignments
  bool step_add_loads(int step_id, int load_type, std::vector<int> load_ids);
  bool step_remove_loads(int step_id, int load_type, std::vector<int> load_ids);
  bool step_add_bcs(int step_id, int bc_type, std::vector<int> bc_ids);
  bool step_remove_bcs(int step_id, int bc_type, std::vector<int> bc_ids);
  bool step_add_historyoutputs(int step_id, std::vector<int> historyoutput_ids);
  bool step_remove_historyoutputs(int step_id, std::vector<int> historyoutput_ids);
  bool step_add_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids);
  bool step_remove_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids);

  // Step utilities
  bool step_utility_modelchangeelement_dummystep(int step_id, std::vector<int> trajectory_ids);
  StepSplitResult step_utility_split_step(int step_id, std::vector<std::vector<double>> times);
  bool step_utility_save_backup();
  bool step_utility_load_backup();

  // Step export
  std::string get_step_export_data();

  // Step tree data
  std::vector<std::vector<std::string>> get_steps_tree_data();
  std::vector<std::vector<std::string>> get_steps_loadsforces_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_loadspressures_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_loadsheatfluxes_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_loadsgravity_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_loadscentrifugal_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_loadstrajectory_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_loadsfilm_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_loadsradiation_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_loadssurfacetraction_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_loadsbodyheatflux_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_bcsdisplacements_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_bcstemperatures_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_historyoutputs_tree_data(int step_id);
  std::vector<std::vector<std::string>> get_steps_fieldoutputs_tree_data(int step_id);

  // Force loads
  std::vector<int> get_loadsforces_ids();
  bool create_loadsforces(std::vector<std::string> options, std::vector<double> options2);
  bool modify_loadsforces(int force_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker);
  bool delete_loadsforces(int force_id);
  std::vector<std::vector<std::string>> get_loadsforces_tree_data();

  // Pressure loads
  std::vector<int> get_loadspressures_ids();
  bool create_loadspressures(std::vector<std::string> options, std::vector<double> options2);
  bool modify_loadspressures(int pressure_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker);
  bool delete_loadspressures(int pressure_id);
  std::vector<std::vector<std::string>> get_loadspressures_tree_data();

  // Gravity loads
  std::vector<int> get_loadsgravity_ids();
  bool create_loadsgravity(std::vector<std::string> options);
  bool modify_loadsgravity(int gravity_id, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_loadsgravity(int gravity_id);
  std::vector<std::vector<std::string>> get_loadsgravity_tree_data();

  // Centrifugal loads
  std::vector<int> get_loadscentrifugal_ids();
  bool create_loadscentrifugal(std::vector<std::string> options);
  bool modify_loadscentrifugal(int centrifugal_id, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_loadscentrifugal(int centrifugal_id);
  std::vector<std::vector<std::string>> get_loadscentrifugal_tree_data();

  // Surface traction loads
  std::vector<int> get_loadssurfacetraction_ids();
  bool create_loadssurfacetraction(std::vector<std::string> options, std::vector<double> options2);
  bool modify_loadssurfacetraction(int surfacetraction_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker);
  bool delete_loadssurfacetraction(int surfacetraction_id);
  std::vector<std::vector<std::string>> get_loadssurfacetraction_tree_data();

  // Heat flux loads
  std::vector<int> get_loadsheatfluxes_ids();
  bool create_loadsheatfluxes(std::vector<std::string> options, std::vector<double> options2);
  bool modify_loadsheatfluxes(int heatflux_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker);
  bool delete_loadsheatfluxes(int heatflux_id);
  std::vector<std::vector<std::string>> get_loadsheatfluxes_tree_data();

  // Body heat flux loads
  std::vector<int> get_loadsbodyheatflux_ids();
  bool create_loadsbodyheatflux(std::vector<std::string> options, std::vector<int> options2);
  bool modify_loadsbodyheatflux(int bodyheatflux_id, std::vector<std::string> options, std::vector<int> options2, std::vector<int> options_marker);
  bool delete_loadsbodyheatflux(int bodyheatflux_id);
  std::vector<std::vector<std::string>> get_loadsbodyheatflux_tree_data();

  // Film loads
  std::vector<int> get_loadsfilm_ids();
  bool create_loadsfilm(std::vector<std::string> options);
  bool modify_loadsfilm(int film_id, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_loadsfilm(int film_id);
  std::vector<std::vector<std::string>> get_loadsfilm_tree_data();

  // Radiation loads
  std::vector<int> get_loadsradiation_ids();
  bool create_loadsradiation(std::vector<std::string> options);
  bool modify_loadsradiation(int radiation_id, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_loadsradiation(int radiation_id);
  std::vector<std::vector<std::string>> get_loadsradiation_tree_data();

  // Trajectory loads
  std::vector<int> get_loadstrajectory_ids();
  bool create_loadstrajectory(std::string load_type, std::vector<std::string> options, std::vector<int> options2, std::vector<std::vector<double>> options3);
  bool modify_loadstrajectory(int trajectory_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2, std::vector<std::vector<double>> options3);
  bool delete_loadstrajectory(int trajectory_id);
  std::string loadstrajectory_get_load_type(int trajectory_id);
  int loadstrajectory_get_subload_id(int trajectory_id); // Returns -1 if the lookup fails.
  std::vector<std::vector<std::string>> get_loadstrajectory_tree_data();

  // Trajectory export preparation
  bool prepare_export();
  bool clean_export();

  // Heat flux trajectory queries
  std::vector<int> loadstrajectory_heatflux_get_node_ids(int trajectory_id);
  std::vector<int> loadstrajectory_heatflux_get_edge_ids(int trajectory_id);
  std::vector<std::vector<double>> loadstrajectory_heatflux_get_hit_coordinates(int trajectory_id);
  std::vector<std::vector<std::vector<int>>> loadstrajectory_heatflux_get_face_ids(int trajectory_id);
  std::vector<std::vector<std::vector<int>>> loadstrajectory_heatflux_get_draw_face_ids(int trajectory_id);
  std::vector<std::vector<double>> loadstrajectory_heatflux_get_times(int trajectory_id);
  std::vector<std::vector<double>> loadstrajectory_heatflux_get_radius(int trajectory_id);
  std::vector<std::vector<double>> loadstrajectory_heatflux_get_magnitude(int trajectory_id);

  // Spherical body heat flux trajectory queries
  std::vector<int> loadstrajectory_bodyheatfluxsphere_get_node_ids(int trajectory_id);
  std::vector<int> loadstrajectory_bodyheatfluxsphere_get_edge_ids(int trajectory_id);
  std::vector<std::vector<double>> loadstrajectory_bodyheatfluxsphere_get_hit_coordinates(int trajectory_id);
  std::vector<std::vector<std::vector<int>>> loadstrajectory_bodyheatfluxsphere_get_element_ids(int trajectory_id);
  std::vector<std::vector<std::vector<int>>> loadstrajectory_bodyheatfluxsphere_get_draw_element_ids(int trajectory_id);
  std::vector<std::vector<double>> loadstrajectory_bodyheatfluxsphere_get_times(int trajectory_id);
  std::vector<std::vector<double>> loadstrajectory_bodyheatfluxsphere_get_radius(int trajectory_id);
  std::vector<std::vector<double>> loadstrajectory_bodyheatfluxsphere_get_depth(int trajectory_id);
  std::vector<std::vector<double>> loadstrajectory_bodyheatfluxsphere_get_magnitude(int trajectory_id);

  // Job management
  bool create_job(std::vector<std::string> options);
  bool modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker);
  bool delete_job(int job_id);
  bool set_job_conversion(int job_id, int conversion);

  // Job execution
  bool prepare_job_input(int job_id, const std::string& filename);
  bool is_topology_adjoint_export() const;
  bool run_job(int job_id, int option);
  bool check_jobs();
  bool wait_job(int job_id);
  bool kill_job(int job_id);

  // Job queries
  std::vector<std::string> get_job_data(int job_id);
  std::vector<std::string> get_job_console_output(int job_id);
  std::vector<std::string> get_job_cvg(int job_id);
  std::vector<std::string> get_job_sta(int job_id);
  int get_job_status(int job_id);
  std::vector<std::vector<std::string>> get_jobs_tree_data();

  // Job result viewers

  // Result management
  bool create_result(int job_id);
  bool delete_result(int job_id);
  bool load_result(int job_id);
  // options contains [frd_only, dat_only]; incomplete data is always retained.
  int convert_result(int job_id, std::vector<int> options, std::vector<int> block_ids, std::vector<int> nodeset_ids, std::vector<int> sideset_ids);
  bool project_result(int job_id, int step, int totalincrement, double scale);
  bool project_topology(int topology_id, int iteration_id = -1, double threshold = 0.5, bool preview = false);

  // Result calculations
  double compute_von_mises_stress(std::vector<double> vec);
  double compute_von_mises_strain(std::vector<double> vec);
  std::vector<double> compute_principal_stresses(std::vector<double> vec);
  std::vector<double> compute_principal_strains(std::vector<double> vec);

  // Result plotting
  bool result_plot_job_frd(int job_id, int x_node_id, std::string x_block_type, std::string x_block_component, bool x_increment, bool x_time, int y_node_id, std::string y_block_type, std::string y_block_component, bool y_increment, bool y_time, QString title, QString x_axis, QString y_axis, bool save, QString save_filepath);
  bool result_plot_job_dat(int job_id, int x_node_id, int x_element_id, int x_element_ip, std::string x_block_set, std::string x_block_type, std::string x_block_component, bool x_time, int y_node_id, int y_element_id, int y_element_ip, std::string y_block_set, std::string y_block_type, std::string y_block_component, bool y_time, QString title, QString x_axis, QString y_axis, bool save, QString save_filepath);
  bool result_plot_job_dat_section(int job_id, std::string x_section_set, std::string x_section_component, bool x_time, std::string y_section_set, std::string y_section_component, bool y_time, QString title, QString x_axis, QString y_axis, bool save, QString save_filepath);

  // Result CSV export
  bool result_csv_job_frd(int job_id, std::string block_type, std::string block_component, std::string increment, int node_id, int block_id, int nodeset_id, int sideset_id, bool overwrite, std::string save_filepath);
  bool result_csv_job_dat(int job_id, std::string block_type, std::string block_set, std::string block_component, std::string time, int node_id, int element_id, bool overwrite, std::string save_filepath);
  bool result_csv_job_dat_section(int job_id, std::string result_set, std::string time, bool overwrite, std::string save_filepath);

  // FRD: nodes and result metadata
  std::vector<int> frd_get_nodes(int job_id);
  bool frd_check_node_exists(int job_id, int node_id);
  std::vector<std::string> frd_get_result_block_types(int job_id);
  std::vector<std::string> frd_get_result_block_components(int job_id, std::string result_block_type);
  std::vector<int> frd_get_total_increments(int job_id);
  double frd_get_time_from_total_increment(int job_id, int total_increment);

  // FRD: selection by result values
  std::vector<int> frd_get_node_ids_between_values(int job_id, int total_increment, std::string result_block_type, std::string result_block_component, double lower_value, double upper_value);
  std::vector<int> frd_get_node_ids_smaller_value(int job_id, int total_increment, std::string result_block_type, std::string result_block_component, double value);
  std::vector<int> frd_get_node_ids_greater_value(int job_id, int total_increment, std::string result_block_type, std::string result_block_component, double value);
  std::vector<int> frd_get_element_ids_between_values(int job_id, int total_increment, std::string result_block_type, std::string result_block_component, double lower_value, double upper_value);
  std::vector<int> frd_get_element_ids_smaller_value(int job_id, int total_increment, std::string result_block_type, std::string result_block_component, double value);
  std::vector<int> frd_get_element_ids_greater_value(int job_id, int total_increment, std::string result_block_type, std::string result_block_component, double value);
  std::vector<int> frd_get_element_ids_over_limit(int job_id, int total_increment, std::string result_block_type, std::string result_block_component, double limit);

  // FRD: nodal values
  double frd_get_node_value(int job_id, int node_id, int total_increment, std::string result_block_type, std::string result_block_component);
  std::vector<double> frd_get_node_values(int job_id, int node_id, int total_increment, std::string result_block_type);

  // DAT: result metadata
  std::vector<std::string> dat_get_result_block_types(int job_id);
  std::vector<std::string> dat_get_result_block_set(int job_id);
  std::vector<std::string> dat_get_result_block_components(int job_id, std::string result_block_type);
  std::vector<double> dat_get_result_block_times(int job_id, std::string result_block_type, std::string result_block_set);
  std::vector<int> dat_get_result_block_nodes(int job_id, double time, std::string result_block_type, std::string result_block_set);
  std::vector<int> dat_get_result_block_elements(int job_id, double time, std::string result_block_type, std::string result_block_set);

  // DAT: selection by result values
  std::vector<int> dat_get_node_ids_between_values(int job_id, double time, std::string result_block_type, std::string result_block_set, std::string result_block_component, double lower_value, double upper_value);
  std::vector<int> dat_get_node_ids_smaller_value(int job_id, double time, std::string result_block_type, std::string result_block_set, std::string result_block_component, double value);
  std::vector<int> dat_get_node_ids_greater_value(int job_id, double time, std::string result_block_type, std::string result_block_set, std::string result_block_component, double value);
  std::vector<int> dat_get_element_ids_between_values(int job_id, double time, std::string result_block_type, std::string result_block_set, std::string result_block_component, double lower_value, double upper_value);
  std::vector<int> dat_get_element_ids_smaller_value(int job_id, double time, std::string result_block_type, std::string result_block_set, std::string result_block_component, double value);
  std::vector<int> dat_get_element_ids_greater_value(int job_id, double time, std::string result_block_type, std::string result_block_set, std::string result_block_component, double value);
  std::vector<int> dat_get_element_ids_over_limit(int job_id, double time, std::string result_block_type, std::string result_block_set, std::string result_block_component, double limit);

  // DAT: nodal and element values
  double dat_get_node_value(int job_id, int node_id, double time, std::string result_block_type, std::string result_block_set, std::string result_block_component);
  std::vector<double> dat_get_node_values(int job_id, int node_id, double time, std::string result_block_type, std::string result_block_set);
  std::vector<double> dat_get_element_values_for_component(int job_id, int element_id, double time, std::string result_block_type, std::string result_block_set, std::string result_block_component);
  std::vector<std::vector<double>> dat_get_element_values(int job_id, int element_id, double time, std::string result_block_type, std::string result_block_set);

  // DAT: buckling and section results
  std::vector<std::vector<std::vector<double>>> dat_get_buckle(int job_id);
  std::vector<std::string> dat_get_section_set(int job_id);
  std::vector<std::vector<double>> dat_get_section_data(int job_id, std::string section_set);
  std::vector<std::string> dat_get_section_label(int job_id);

  // Metadata excludes rows. Read table values in bounded pages through this API.
  std::vector<DatResultTable> dat_get_table_metadata(int job_id);
  std::size_t dat_get_table_row_count(int job_id, int table_id);
  std::vector<std::vector<double>> dat_get_table_rows(int job_id, int table_id, std::size_t offset, std::size_t count);


  // Load drawing data
  std::vector<std::vector<double>> get_draw_data_for_load_force(int id);
  std::vector<std::vector<double>> get_draw_data_for_load_pressure(int id);
  std::vector<std::vector<double>> get_draw_data_for_load_heatflux(int id);
  std::vector<std::vector<double>> get_draw_data_for_load_gravity(int id);
  std::vector<std::vector<double>> get_draw_data_for_load_centrifugal(int id);
  std::vector<std::vector<double>> get_draw_data_for_load_film(int id);
  std::vector<std::vector<double>> get_draw_data_for_load_radiation(int id);
  std::vector<std::vector<double>> get_draw_data_for_load_surface_traction(int id);
  std::vector<std::vector<int>> get_draw_data_for_load_bodyheatflux(int id); // Returns block IDs in row 0 and element IDs in row 1.

  // Boundary-condition drawing data
  std::vector<std::vector<double>> get_draw_data_for_bc_displacement(int id);
  std::vector<std::vector<double>> get_draw_data_for_bc_temperature(int id);

  // Orientation and constraint drawing data
  std::vector<std::vector<double>> get_draw_data_for_orientation(int id);
  std::vector<std::vector<double>> get_draw_data_for_equation(int id);
  std::vector<std::vector<double>> get_draw_data_for_equationgroup(int id);
  std::vector<int> get_rigidbody_vertex_list();

  // Drawing: complete categories
  bool draw_all(double size);
  bool draw_loads(double size);
  bool draw_bcs(double size);
  bool draw_orientations(double size);
  bool draw_equations(double size);
  bool draw_equationgroups(double size);

  // Drawing: all entities of a type
  bool draw_load_forces(double size);
  bool draw_load_pressures(double size);
  bool draw_load_heatfluxes(double size);
  bool draw_load_gravities(double size);
  bool draw_load_centrifugals(double size);
  bool draw_load_trajectories(double size);
  bool draw_load_films(double size);
  bool draw_load_radiations(double size);
  bool draw_load_surface_tractions(double size);
  bool draw_load_bodyheatfluxes(double size);
  bool draw_bc_displacements(double size);
  bool draw_bc_temperatures(double size);

  // Drawing: selected loads
  bool draw_load_force(std::vector<int> force_ids, double size);
  bool draw_load_pressure(std::vector<int> pressure_ids, double size);
  bool draw_load_heatflux(std::vector<int> heatflux_ids, double size);
  bool draw_load_gravity(std::vector<int> gravity_ids, double size);
  bool draw_load_centrifugal(std::vector<int> centrifugal_ids, double size);
  bool draw_load_trajectory(std::vector<int> trajectory_ids, double size);
  bool draw_load_film(std::vector<int> film_ids, double size);
  bool draw_load_radiation(std::vector<int> radiation_ids, double size);
  bool draw_load_surface_traction(std::vector<int> surface_traction_ids, double size);
  bool draw_load_bodyheatflux(std::vector<int> bodyheatflux_ids, double size);

  // Drawing: selected boundary conditions, orientations, and equations
  bool draw_bc_displacement(std::vector<int> displacement_ids, double size);
  bool draw_bc_temperature(std::vector<int> temperature_ids, double size);
  bool draw_orientation(std::vector<int> orientation_ids, double size);
  bool draw_equation(std::vector<int> equation_ids, double size);
  bool draw_equationgroup(std::vector<int> equationgroup_ids, double size);

  // Topology drawing and animation
  bool draw_topology(int topology_id, int iteration_id = -1, double threshold = 0.5);
  bool animate_topology(int topology_id, double threshold = 0.5, int fps = 5, std::string save_filepath = "", bool overwrite = false);
  bool topology_animation_active() const;
  void cancel_topology_animation();

  // GUI resources
  QIcon* getIcon(std::string name);
  QIcon getIcon2(std::string name);

  // Topology settings
  bool create_topology(int topology_id, const TopologyOptions& options);
  bool modify_topology(int topology_id, const TopologyOptions& options);
  std::vector<double> get_topology_parameters(int topology_id);
  bool get_topology_settings(int topology_id, TopologyOptions& options);

  // Topology queries
  std::vector<std::vector<std::string>> get_topology_tree_data();
  std::vector<std::vector<std::string>> get_topology_iteration_data(int topology_id);
  bool get_topology_iteration_level_set_settings(int topology_id, int iteration_id, TopologyLevelSetSettings& settings);
  bool get_topology_iteration_method(int topology_id, int iteration_id, TopologyMethod& method);
  std::vector<std::vector<double>> get_topology_iteration_element_data(int topology_id, int iteration_id = -1);
  /// Completed density iteration: [element ID, dJ/ds, unfiltered dJ/drho].
  /// Grouped density gradients are SIMP surrogates. Returns empty on failure.
  /// Historical [node ID, phi, fixed flag] rows, sorted by global node ID.
  std::vector<std::vector<double>> get_topology_iteration_level_set_field_data(int topology_id, int iteration_id = -1);
  bool is_topology_level_set_run_enabled();
  std::vector<std::vector<double>> get_topology_iteration_sensitivity_data(int topology_id, int iteration_id = -1);
  std::string get_topology_run_state(int topology_id);
  bool get_topology_base_run_info(int topology_id, TopologyRunInfo& information);

  // Topology job queries
  bool get_job_topology_context(int job_id, int& topology_id, int& iteration_id);
  int get_topology_active_iteration_job(int topology_id, int iteration_id);
  int get_topology_adjoint_job_source(int topology_id, int iteration_id);
  int get_topology_iteration_job_id(int topology_id, int iteration_id); // Returns -1 if no associated job is available.
  int get_topology_iteration_adjoint_job_id(int topology_id, int iteration_id);

  // Topology iteration updates
  bool set_topology_iteration_displacement(int topology_id, int iteration_id, double displacement, int node_id);
  bool mark_topology_iteration_completed(int topology_id, int iteration_id);
  bool set_topology_iteration_factors(int topology_id, int iteration_id, const std::vector<int>& element_ids, const std::vector<double>& stiffness_factors);
  bool select_topology_iteration(int topology_id, int iteration_id);

  // Topology run control
  bool begin_topology_run(int topology_id);
  bool prepare_topology_run(int topology_id);
  bool set_topology_run_state(int topology_id, TopologyRunState state);
  bool run_topology(int topology_id, bool wait = false);
  bool wait_topology(int topology_id);
  bool kill_topology(int topology_id);
  bool cancel_topology_runs();
  bool get_topology_run_info(int topology_id, TopologyRunInfo& information);

  // Topology iteration execution
  int start_topology_iteration(int topology_id, int iteration_id);
  int run_topology_iteration(int topology_id, int iteration_id); // Returns the completed job ID, or -1 on failure.
  bool finish_topology_iteration(int topology_id, int iteration_id);
  bool create_topology_adjoint_job(int topology_id, int iteration_id, int& job_id, bool& skipped);
  bool build_topology_adjoint_load(int topology_id, int iteration_id, double& objective_value, int& node_id, std::array<double, 3>& load);

  // Topology job management
  bool reset_topology_run(int topology_id);
  bool delete_topology(int topology_id);
  int create_topology_iteration_job(int topology_id, int iteration_id, const std::string& job_name);
  bool register_topology_adjoint_job(int topology_id, int iteration_id, int& job_id);

  // Topology model preparation
  bool collect_topology_run_sections(int topology_id, std::vector<int>& section_ids, std::set<int>& block_ids);
  bool initialize_topology_run_design(int topology_id, const std::vector<int>& section_ids);
  bool initialize_topology(int topology_id, const std::vector<int>& section_ids);

  // Topology output preparation
  bool prepare_topology_history_outputs(int topology_id, const std::set<int>& block_ids);
  bool ensure_topology_element_history_output(int topology_id, int step_id, int block_id, const std::string& key);
  bool ensure_topology_node_history_output(int topology_id, int step_id, int nodeset_id, const std::string& key);
  bool ensure_topology_displacement_field_output(int topology_id, int step_id);

  // Topology geometry
  bool collect_topology_element_volumes(int topology_id, std::map<int, double>& element_volumes, std::map<int, std::array<double, 3>>* element_centers = nullptr);
  bool collect_topology_integration_weights(int topology_id, std::map<int, std::map<int, double>>& weights);

  // Topology result reading
  bool read_topology_job_displacements(int job_id, std::map<int, std::array<double, 3>>& displacements);
  bool read_topology_displacement(int topology_id, int iteration_id, double& value, int& node_id, std::array<double, 3>* critical_displacement = nullptr);
  bool read_topology_job_element_tensors(int job_id, const std::string& key, std::map<int, std::map<int, std::array<double, 6>>>& tensors);
  bool read_topology_element_energies(int topology_id, int iteration_id, std::map<int, double>& element_energies);
  bool check_topology_adjoint_results(int job_id);

  // Topology evaluation
  bool evaluate_topology_compliance(int topology_id, int iteration_id, const std::map<int, double>& energies);
  bool evaluate_topology_displacement(int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals);
  bool evaluate_topology_compliance_oc_residual(int topology_id, int iteration_id);
  bool evaluate_topology_displacement_projected_residual(int topology_id, int iteration_id);

  // Topology density updates
  bool set_topology_iteration_densities(int topology_id, int iteration_id, const std::map<int, double>& densities, int source_iteration = -1);
  bool initialize_topology_level_set_design(int topology_id);
  bool evaluate_topology_level_set_compliance(int topology_id, int iteration_id, const std::map<int, double>& energies, CoreTopologyLevelSetGeometryCache* cache = nullptr);
  bool evaluate_topology_level_set_proposal(int topology_id, int iteration_id, TopologyLevelSetProposal& proposal, std::shared_ptr<CoreTopologyLevelSetGeometryCache> source_cache = {});
  bool prepare_topology_level_set_iteration(int topology_id, int source_iteration, int next_iteration, const TopologyLevelSetProposal& proposal, double initial_step_scale = 1.0);
  bool check_topology_level_set_convergence(int topology_id, int previous_iteration, int current_iteration, bool& converged);
  bool prepare_topology_density_iteration(int topology_id, int source_iteration, int next_iteration);
  bool filter_topology_density_sensitivities(int topology_id, int iteration_id, const std::map<int, double>& element_volumes, const std::map<int, std::array<double, 3>>& element_centers);
  bool build_topology_density_update(int topology_id, int source_iteration, std::map<int, double>& densities);
  bool check_topology_density_convergence(int topology_id, int previous_iteration, int current_iteration, bool& converged);

  // Topology validation
  bool validate_topology_compliance_model(bool require_zero_displacements = true);
  bool check_topology_displacement_selection(int topology_id, int iteration_id);
  bool check_topology_export(int topology_id, int iteration_id);

  // Topology preservation
  bool get_topology_preserved_sets(int topology_id, std::vector<TopologyPreservedSetInfo>& sets, std::set<int>& direct_elements);
  bool reset_preserved_sets(int topology_id);
  bool add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids);
  bool remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids);
  std::vector<std::vector<int>> get_topology_element_preservation(int topology_id);

  /**
   * @brief Query participating volume elements selected by an existing set.
   * @param topology_id Case with a prepared topology material/analysis collection.
   * @param set_type Preservation kind: 1=block, 2=nodeset, 3=sideset.
   * @param set_id Existing positive set ID of that kind.
   * @param element_ids Sorted unique global IDs; cleared on entry and empty on failure.
   * @return True for a successful query, including an empty intersection; false on error.
   * @note Nodesets use any-node intersection with expanded connectivity, including
   * midside nodes. Sidesets select owners. Results are restricted to the analysis.
   */
  bool get_topology_set_element_ids(int topology_id, int set_type, int set_id, std::vector<int>& element_ids);

  /**
   * @brief Collect automatic preservation rules from current analysis definitions.
   * @param default_sets Automatic IDs keyed by kind: 1=block, 2=nodeset, 3=sideset.
   * @param default_elements Direct global-element selections, separate from set rules.
   * @return True after publishing both collections; false with both outputs empty.
   * @note Outputs are cleared on entry. Constraints/contact definitions are model
   * selections; only loads and BCs referenced by existing steps contribute.
   * Direct selections can include elements outside a particular topology domain;
   * the resolver applies that domain restriction. No case flags are modified here.
   */
  bool collect_topology_default_sets(std::map<int, std::set<int>>& default_sets, std::set<int>& default_elements);

  /**
   * @brief Resolve automatic and case-specific preservation against prepared elements.
   * @param topology_id Existing case with a prepared topology material collection.
   * @param default_sets Automatic IDs keyed by kind: 1=block, 2=nodeset, 3=sideset.
   * @param default_elements Direct automatic selections not removed by set exclusions.
   * @return True when CoreTopology commits the flags; false on resolution failure.
   * @note Uses the prepared analysis global IDs and delegates rule application
   * through the owned topology core; collection and resolution remain separate.
   */
  bool resolve_topology_preservation(int topology_id, const std::map<int, std::set<int>>& default_sets, const std::set<int>& default_elements);

  // Topology material queries
  bool check_topology_material_assignment(int topology_id, int iteration_id, int element_id);
  std::string get_topology_material_name(int topology_id, int element_id);
  bool get_topology_group_material_name(int topology_id, int iteration_id, int element_id, std::string& material_name);

  CoreBlocks* cb = nullptr;
  CoreMaterials* mat = nullptr;
  CoreMaterialsLibrary* mat_library = nullptr;
  CoreSections* sections = nullptr;
  CoreConstraints* constraints = nullptr;
  CoreSurfaceInteractions* surfaceinteractions = nullptr;
  CoreContactPairs* contactpairs = nullptr;
  CoreAmplitudes* amplitudes = nullptr;
  CoreOrientations* orientations = nullptr;
  CoreDamping* damping = nullptr;
  CorePhysicalConstants* physicalconstants = nullptr;
  CoreLoadsForces* loadsforces = nullptr;
  CoreLoadsPressures* loadspressures = nullptr;
  CoreLoadsHeatfluxes* loadsheatfluxes = nullptr;
  CoreLoadsGravity* loadsgravity = nullptr;
  CoreLoadsCentrifugal* loadscentrifugal = nullptr;
  CoreLoadsTrajectory* loadstrajectory = nullptr;
  CoreLoadsFilm* loadsfilm = nullptr;
  CoreLoadsRadiation* loadsradiation = nullptr;
  CoreLoadsSurfaceTraction* loadssurfacetraction = nullptr;
  CoreLoadsBodyHeatflux* loadsbodyheatflux = nullptr;
  CoreBCsDisplacements* bcsdisplacements = nullptr;
  CoreBCsTemperatures* bcstemperatures = nullptr;
  CoreHistoryOutputs* historyoutputs = nullptr;
  CoreFieldOutputs* fieldoutputs = nullptr;
  CoreInitialConditions* initialconditions = nullptr;
  CoreHBCs* hbcs = nullptr;
  CoreSteps* steps = nullptr;
  CoreJobs* jobs = nullptr;
  CoreTopology* topology = nullptr;
  CoreTopologyOptimization* topologyoptimization = nullptr;
  CoreResults* results = nullptr;
  CoreTimer* timer = nullptr;
  CoreCustomLines* customlines = nullptr;
  CoreDraw* draw = nullptr;
  MeshExportInterface* me_iface = nullptr;
  MaterialInterface* mat_iface = nullptr;

private:
  CoreLoadsQueryData get_load_query_data() const;

};

#endif // CALCULIXCORE_HPP
