#ifndef CALCULIXCORE_HPP
#define CALCULIXCORE_HPP


class CoreBlocks;
class CoreMaterials;
class CoreSections;
class CoreConstraints;
class CoreReferencePoints;
class CoreSurfaceInteractions;
class CoreContactPairs;
class CoreAmplitudes;
class CoreLoadsForces;
class CoreLoadsPressures;
class CoreBCsDisplacements;
class CoreBCsTemperatures;

class MeshExportInterface;
class MaterialInterface;

class CalculiXCore
{
  
public:
  CalculiXCore();
  ~CalculiXCore();

  bool use_ccx_logfile = false;

  bool print_to_log(std::string str_log);
  bool init();
  bool update(); // updates everything to catch changes in the entities
  bool reset(); // delete all data and init new
  std::string print_data();
  std::vector<int> parser(std::string parse_type, std::string parse_string);
  std::vector<int> extractIntegers(std::string str);
  std::vector<std::string> get_ccx_element_types(); // returns all supported ccx element types;
  std::string get_ccx_element_type(int block_id); // gets the ccx element type for a block
  std::string get_block_name(int block_id); // gets the block name
  std::string get_nodeset_name(int nodeset_id); // gets the nodeset name
  std::string get_sideset_name(int sideset_id); // gets the sideset name
  std::string get_surfaceinteraction_name(int surfaceinteraction_id); // gets the surfaceinteraction name
  bool set_ccx_element_type(int block_id, std::string ccx_element_type); // sets the ccx element type for a block
  std::vector<int> get_blocks(); // gets the block ids from core blocks
  std::string get_material_export_data(); // gets the export data from materials core
  std::string get_section_export_data(); // gets the export data from sections core
  std::string get_constraint_export_data(); // gets the export data from constraints core
  std::string get_surfaceinteraction_export_data(); // gets the export data from surfaceinteractions core
  std::string get_contactpair_export_data(); // gets the export data from contactpairs core
  std::string get_amplitude_export_data(); // gets the export data from amplitudes core
  bool create_section(std::string section_type,int block_id, std::string material_name, std::vector<std::string> options); // adds a new section
  bool modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a section
  bool delete_section(int section_id); // adds a new section
  bool create_constraint(std::string constraint_type, std::vector<std::string> options); // adds a new constraint
  bool modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a constraint
  bool delete_constraint(int constraint_id); // adds a new constraint
  bool create_constraint_tie_from_cubitcontactpair(std::string name, std::string position_tolerance); // create constraint tie from cubit contact pairs
  std::vector<int> get_rigidbody_vertex_list(); // get list of rigid body vertices
  bool referencepoints_update_on_export(int max_node_id); // update reference points
  bool referencepoints_reset_on_export(); // reset reference points
  std::string get_referencepoints_export(); // export reference points
  std::string get_referencepoints_export_nodesets(); // export reference points nodesets
  int  referencepoints_get_ref_from_vertex_id(int vertex_id); // get ref_node for the vertex_id  and returns -1 if it fails
  int  referencepoints_get_rot_from_vertex_id(int vertex_id); // get ref_node for the vertex_id  and returns -1 if it fails
  bool create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2); // adds a new surfaceinteraction
  bool modify_surfaceinteraction(std::string modify_type, int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2); // modifies a surfaceinteraction
  bool delete_surfaceinteraction(int surfaceinteraction_id); // delete surface interaction
  bool create_contactpair(std::vector<std::string> options); // adds a new contactpair
  bool modify_contactpair(int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a contactpair  
  bool delete_contactpair(int contactpair_id); // delete contact pair
  bool create_contactpair_from_cubitcontactpair(int surfaceinteraction_id, std::string contactpairtype, std::string adjust); // create contact pairs from cubit contact pairs with given surface interaction
  bool create_amplitude(std::vector<std::string> options, std::vector<std::vector<std::string>> options2); // adds a new amplitude
  bool modify_amplitude(int amplitude_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2); // modify a amplitude
  bool delete_amplitude(int amplitude_id); // delete amplitude
  bool modify_loadsforces(int force_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a force
  bool modify_loadspressures(int pressure_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a pressure
  bool modify_bcsdisplacements(int displacement_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a displacement
  bool modify_bcstemperatures(int temperature_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a temperature
  std::vector<std::vector<std::string>> get_blocks_tree_data(); // gets the data from core blocks to build the tree
  std::vector<std::vector<std::string>> get_nodeset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_sideset_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_material_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_sections_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_constraints_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_surfaceinteractions_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_contactpairs_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_amplitudes_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadsforces_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_loadspressures_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_bcsdisplacements_tree_data(); // gets the data from core to build the tree
  std::vector<std::vector<std::string>> get_bcstemperatures_tree_data(); // gets the data from core to build the tree

  CoreBlocks *cb;
  CoreMaterials *mat;
  CoreSections *sections;
  CoreConstraints *constraints;
  CoreReferencePoints *referencepoints;
  CoreSurfaceInteractions *surfaceinteractions;
  CoreContactPairs *contactpairs;
  CoreAmplitudes *amplitudes;
  CoreLoadsForces *loadsforces;
  CoreLoadsPressures *loadspressures;
  CoreBCsDisplacements *bcsdisplacements;
  CoreBCsTemperatures *bcstemperatures;
  MeshExportInterface *me_iface;
  MaterialInterface *mat_iface;
};

#endif // CALCULIXCore_HPP
