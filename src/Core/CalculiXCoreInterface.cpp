#include "CalculiXCoreInterface.hpp"
#include "loadCalculiXCore.hpp"

CalculiXCoreInterface::CalculiXCoreInterface()
{}

CalculiXCoreInterface::~CalculiXCoreInterface()
{}

bool CalculiXCoreInterface::set_use_ccx_logfile(bool bool_use)
{
  ccx_core.use_ccx_logfile = bool_use;
  return true;
}

std::string CalculiXCoreInterface::print_data()
{
  return ccx_core.print_data();
}

bool CalculiXCoreInterface::log_str(std::string str_log)
{
  ccx_core.print_to_log(str_log);
  return true;
}

std::vector<int> CalculiXCoreInterface::parser(std::string parse_type, std::string parse_string)
{
  return ccx_core.parser(parse_type, parse_string);
}

std::vector<std::string> CalculiXCoreInterface::get_ccx_element_types()
{ 
  return ccx_core.get_ccx_element_types();
}

bool CalculiXCoreInterface::set_ccx_element_type(int block_id, std::string ccx_element_type)
{
  if (ccx_core.set_ccx_element_type(block_id, ccx_element_type))
  {
    return true;
  }else{
    return false;
  }
}

std::string CalculiXCoreInterface::get_ccx_element_type(int block_id)
{
  return ccx_core.get_ccx_element_type(block_id);
}

std::string CalculiXCoreInterface::get_block_name(int block_id)
{
  return ccx_core.get_block_name(block_id);
}

std::string CalculiXCoreInterface::get_nodeset_name(int nodeset_id)
{
  return ccx_core.get_nodeset_name(nodeset_id);
}

std::string CalculiXCoreInterface::get_sideset_name(int sideset_id)
{
  return ccx_core.get_sideset_name(sideset_id);
}

std::string CalculiXCoreInterface::get_surfaceinteraction_name(int surfaceinteraction_id)
{
  return ccx_core.get_surfaceinteraction_name(surfaceinteraction_id);
}

bool CalculiXCoreInterface::core_update()
{
  if (ccx_core.update())
  {
    return true;
  }else{
    return false;
  }
}

bool CalculiXCoreInterface::core_reset()
{
  if (ccx_core.reset())
  {
    return true;
  }else{
    return false;
  }
}

std::vector<int> CalculiXCoreInterface::get_blocks()
{ 
  return ccx_core.get_blocks();
}

bool CalculiXCoreInterface::create_section(std::string section_type,int block_id, std::string material_name, std::vector<std::string> options)
{
  return ccx_core.create_section(section_type,block_id,material_name,options);
}

bool CalculiXCoreInterface::modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_section(section_type,section_id,options, options_marker);
}

bool CalculiXCoreInterface::delete_section(int section_id)
{
  return ccx_core.delete_section(section_id);
}

bool CalculiXCoreInterface::create_constraint(std::string constraint_type, std::vector<std::string> options)
{
  return ccx_core.create_constraint(constraint_type,options);
}

bool CalculiXCoreInterface::modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_constraint(constraint_type,constraint_id,options, options_marker);
}

bool CalculiXCoreInterface::delete_constraint(int constraint_id)
{
  return ccx_core.delete_constraint(constraint_id);
}

bool CalculiXCoreInterface::create_constraint_tie_from_cubitcontactpair(std::string name, std::string position_tolerance)
{
  return ccx_core.create_constraint_tie_from_cubitcontactpair(name, position_tolerance);
}

std::vector<int> CalculiXCoreInterface::get_rigidbody_vertex_list()
{
  return ccx_core.get_rigidbody_vertex_list();
}

bool CalculiXCoreInterface::referencepoints_update_on_export(int max_node_id)
{
  return ccx_core.referencepoints_update_on_export(max_node_id);
}

bool CalculiXCoreInterface::referencepoints_reset_on_export()
{
  return ccx_core.referencepoints_reset_on_export();
}

std::string CalculiXCoreInterface::get_referencepoints_export()
{
  return ccx_core.get_referencepoints_export();
}

std::string CalculiXCoreInterface::get_referencepoints_export_nodesets()
{
  return ccx_core.get_referencepoints_export_nodesets();
}

int CalculiXCoreInterface::referencepoints_get_ref_from_vertex_id(int vertex_id)
{
  return ccx_core.referencepoints_get_ref_from_vertex_id(vertex_id);
}

int CalculiXCoreInterface::referencepoints_get_rot_from_vertex_id(int vertex_id)
{
  return ccx_core.referencepoints_get_rot_from_vertex_id(vertex_id);
}
bool CalculiXCoreInterface::create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.create_surfaceinteraction(surfacebehavior_type,options,options2);
}

bool CalculiXCoreInterface::modify_surfaceinteraction(std::string modify_type, int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.modify_surfaceinteraction(modify_type,surfaceinteraction_id,options,options_marker,options2);
}

bool CalculiXCoreInterface::delete_surfaceinteraction(int surfaceinteraction_id)
{
  return ccx_core.delete_surfaceinteraction(surfaceinteraction_id);
}

bool CalculiXCoreInterface::create_contactpair(std::vector<std::string> options)
{
  return ccx_core.create_contactpair(options);
}

bool CalculiXCoreInterface::modify_contactpair(int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_contactpair(contactpair_id,options,options_marker);
}

bool CalculiXCoreInterface::delete_contactpair(int contactpair_id)
{
  return ccx_core.delete_contactpair(contactpair_id);
}

bool CalculiXCoreInterface::create_contactpair_from_cubitcontactpair(int surfaceinteraction_id, std::string contactpairtype, std::string adjust)
{
  return ccx_core.create_contactpair_from_cubitcontactpair(surfaceinteraction_id,contactpairtype,adjust);
}

bool CalculiXCoreInterface::create_amplitude(std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.create_amplitude(options, options2);
}

bool CalculiXCoreInterface::modify_amplitude(int amplitude_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.modify_amplitude(amplitude_id,options,options_marker, options2);
}

bool CalculiXCoreInterface::delete_amplitude(int amplitude_id)
{
  return ccx_core.delete_amplitude(amplitude_id);
}

bool CalculiXCoreInterface::modify_loadsforces(int force_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadsforces(force_id,options,options_marker);
}

bool CalculiXCoreInterface::modify_loadspressures(int pressure_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadspressures(pressure_id,options,options_marker);
}

bool CalculiXCoreInterface::modify_bcsdisplacements(int displacement_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_bcsdisplacements(displacement_id,options,options_marker);
}

bool CalculiXCoreInterface::modify_bcstemperatures(int temperature_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_bcstemperatures(temperature_id,options,options_marker);
}

bool CalculiXCoreInterface::create_historyoutput(std::vector<std::string> options)
{
  return ccx_core.create_historyoutput(options);
}

bool CalculiXCoreInterface::modify_historyoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_historyoutput(output_id, modify_type, options, options_marker);
}

bool CalculiXCoreInterface::delete_historyoutput(int output_id)
{
  return ccx_core.delete_historyoutput(output_id);
}

std::vector<std::string> CalculiXCoreInterface::get_historyoutput_node_keys()
{
  return  ccx_core.get_historyoutput_node_keys();
}

std::vector<std::string> CalculiXCoreInterface::get_historyoutput_element_keys()
{
  return  ccx_core.get_historyoutput_element_keys();
}

std::vector<std::string> CalculiXCoreInterface::get_historyoutput_contact_keys()
{
  return  ccx_core.get_historyoutput_contact_keys();
}

bool CalculiXCoreInterface::create_fieldoutput(std::vector<std::string> options)
{
  return ccx_core.create_fieldoutput(options);
}

bool CalculiXCoreInterface::modify_fieldoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_fieldoutput(output_id, modify_type, options, options_marker);
}

bool CalculiXCoreInterface::delete_fieldoutput(int output_id)
{
  return ccx_core.delete_fieldoutput(output_id);
}

std::vector<std::string> CalculiXCoreInterface::get_fieldoutput_node_keys()
{
  return  ccx_core.get_fieldoutput_node_keys();
}

std::vector<std::string> CalculiXCoreInterface::get_fieldoutput_element_keys()
{
  return  ccx_core.get_fieldoutput_element_keys();
}

std::vector<std::string> CalculiXCoreInterface::get_fieldoutput_contact_keys()
{
  return  ccx_core.get_fieldoutput_contact_keys();
}

bool CalculiXCoreInterface::create_initialcondition(std::vector<std::string> options)
{
  return ccx_core.create_initialcondition(options);
}

bool CalculiXCoreInterface::modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_initialcondition(initialcondition_id, modify_type, options, options_marker);
}

bool CalculiXCoreInterface::delete_initialcondition(int initialcondition_id)
{
  return ccx_core.delete_initialcondition(initialcondition_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_blocks_tree_data()
{ 
  return ccx_core.get_blocks_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_nodeset_tree_data()
{ 
  return ccx_core.get_nodeset_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_sideset_tree_data()
{ 
  return ccx_core.get_sideset_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_material_tree_data()
{ 
  return ccx_core.get_material_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_sections_tree_data()
{ 
  return ccx_core.get_sections_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_constraints_tree_data()
{ 
  return ccx_core.get_constraints_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_surfaceinteractions_tree_data()
{ 
  return ccx_core.get_surfaceinteractions_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_contactpairs_tree_data()
{ 
  return ccx_core.get_contactpairs_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_amplitudes_tree_data()
{ 
  return ccx_core.get_amplitudes_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_loadsforces_tree_data()
{ 
  return ccx_core.get_loadsforces_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_loadspressures_tree_data()
{ 
  return ccx_core.get_loadspressures_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_bcsdisplacements_tree_data()
{ 
  return ccx_core.get_bcsdisplacements_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_bcstemperatures_tree_data()
{ 
  return ccx_core.get_bcstemperatures_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_historyoutputs_tree_data()
{ 
  return ccx_core.get_historyoutputs_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_fieldoutputs_tree_data()
{ 
  return ccx_core.get_fieldoutputs_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_initialconditions_tree_data()
{ 
  return ccx_core.get_initialconditions_tree_data();
}

std::string CalculiXCoreInterface::get_material_export_data() // gets the export data from materials core
{
  return ccx_core.get_material_export_data();
}

std::string CalculiXCoreInterface::get_section_export_data() // gets the export data from sections core
{
  return ccx_core.get_section_export_data();
}

std::string CalculiXCoreInterface::get_constraint_export_data() // gets the export data from constraints core
{
  return ccx_core.get_constraint_export_data();
}

std::string CalculiXCoreInterface::get_surfaceinteraction_export_data() // gets the export data from surfaceinteractions core
{
  return ccx_core.get_surfaceinteraction_export_data();
}

std::string CalculiXCoreInterface::get_contactpair_export_data() // gets the export data from contactpairs core
{
  return ccx_core.get_contactpair_export_data();
}

std::string CalculiXCoreInterface::get_amplitude_export_data() // gets the export data from amplitudes core
{
  return ccx_core.get_amplitude_export_data();
}