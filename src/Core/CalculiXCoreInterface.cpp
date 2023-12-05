#include "CalculiXCoreInterface.hpp"
#include "loadCalculiXCore.hpp"

CalculiXCoreInterface::CalculiXCoreInterface()
{}

CalculiXCoreInterface::~CalculiXCoreInterface()
{}

bool CalculiXCoreInterface::set_use_ccx_autocleanup(bool bool_use)
{
  ccx_core.use_ccx_autocleanup = bool_use;
  return true;
}

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

std::string  CalculiXCoreInterface::to_string_scientific(double value)
{
  return ccx_core.to_string_scientific(value);
}

double CalculiXCoreInterface::string_scientific_to_double(std::string value)
{
  return ccx_core.string_scientific_to_double(value);
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

std::string CalculiXCoreInterface::get_cubit_element_type_entity(std::string cubit_element_type)
{
  return ccx_core.get_cubit_element_type_entity(cubit_element_type);
}

std::vector<std::vector<int>> CalculiXCoreInterface::get_element_id_type_connectivity()
{
  return ccx_core.get_element_id_type_connectivity();
}

std::string CalculiXCoreInterface::get_block_name(int block_id)
{
  return ccx_core.get_block_name(block_id);
}

std::vector<int> CalculiXCoreInterface::get_block_node_ids(int block_id)
{
  return ccx_core.get_block_node_ids(block_id);
}

std::vector<int> CalculiXCoreInterface::get_block_element_ids(int block_id)
{
  return ccx_core.get_block_element_ids(block_id);
}

std::string CalculiXCoreInterface::get_material_name(int material_id)
{
  return ccx_core.get_material_name(material_id);
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

std::vector<std::string> CalculiXCoreInterface::get_contactpair_master_slave(int contactpair_id)
{
  return ccx_core.get_contactpair_master_slave(contactpair_id);
}

std::string CalculiXCoreInterface::get_amplitude_name(int amplitude_id)
{
  return ccx_core.get_amplitude_name(amplitude_id);
}

bool CalculiXCoreInterface::check_block_exists(int block_id)
{
  return ccx_core.check_block_exists(block_id);
}

bool CalculiXCoreInterface::check_nodeset_exists(int nodeset_id)
{
  return ccx_core.check_nodeset_exists(nodeset_id);
}

bool CalculiXCoreInterface::check_sideset_exists(int sideset_id)
{
  return ccx_core.check_sideset_exists(sideset_id);
}

bool CalculiXCoreInterface::check_vertex_in_nodeset_exists(int vertex_id,int nodeset_id)
{
  return ccx_core.check_vertex_in_nodeset_exists(vertex_id,nodeset_id);
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

int CalculiXCoreInterface::get_ccx_element_side(int element_type,int side)
{
  return ccx_core.get_ccx_element_side(element_type,side);
}

bool CalculiXCoreInterface::add_sideset_face(std::string sideset_id, std::string sideset_name, std::string face, std::string element_type)
{
  return ccx_core.add_sideset_face(sideset_id, sideset_name, face, element_type);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_sideset_face(int sideset_id)
{
  return ccx_core.get_sideset_face(sideset_id);
}

bool CalculiXCoreInterface::create_section(std::string section_type,int block_id, int material_id, std::vector<std::string> options)
{
  return ccx_core.create_section(section_type,block_id,material_id,options);
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

bool CalculiXCoreInterface::create_contactpair_from_cubitcontactpair(int surfaceinteraction_id, std::string contactpairtype, std::string adjust, std::string adjust_nodeset)
{
  return ccx_core.create_contactpair_from_cubitcontactpair(surfaceinteraction_id,contactpairtype,adjust,adjust_nodeset);
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

bool CalculiXCoreInterface::modify_loadsheatfluxes(int heatflux_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadsheatfluxes(heatflux_id,options,options_marker);
}

bool CalculiXCoreInterface::create_loadsgravity(std::vector<std::string> options)
{
  return ccx_core.create_loadsgravity(options);
}

bool CalculiXCoreInterface::modify_loadsgravity(int gravity_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadsgravity(gravity_id,options,options_marker);
}

bool CalculiXCoreInterface::delete_loadsgravity(int gravity_id)
{
  return ccx_core.delete_loadsgravity(gravity_id);
}

bool CalculiXCoreInterface::create_loadscentrifugal(std::vector<std::string> options)
{
  return ccx_core.create_loadscentrifugal(options);
}

bool CalculiXCoreInterface::modify_loadscentrifugal(int centrifugal_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_loadscentrifugal(centrifugal_id,options,options_marker);
}

bool CalculiXCoreInterface::delete_loadscentrifugal(int centrifugal_id)
{
  return ccx_core.delete_loadscentrifugal(centrifugal_id);
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

bool CalculiXCoreInterface::hbc_add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.hbc_add_bcs(bcs_id, bc_type, bc_ids);
}

bool CalculiXCoreInterface::hbc_remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.hbc_remove_bcs(bcs_id, bc_type, bc_ids);
}

bool CalculiXCoreInterface::create_step(std::vector<std::string> options)
{
  return ccx_core.create_step(options);
}

bool CalculiXCoreInterface::modify_step(int step_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_step(step_id, modify_type, options, options_marker);
}

bool CalculiXCoreInterface::delete_step(int step_id)
{
  return ccx_core.delete_step(step_id);
}

bool CalculiXCoreInterface::step_add_loads(int step_id, int load_type, std::vector<int> load_ids)
{
  return ccx_core.step_add_loads(step_id,load_type,load_ids);
}

bool CalculiXCoreInterface::step_add_bcs(int step_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.step_add_bcs(step_id,bc_type,bc_ids);
}

bool CalculiXCoreInterface::step_add_historyoutputs(int step_id, std::vector<int> historyoutput_ids)
{
  return ccx_core.step_add_historyoutputs(step_id,historyoutput_ids);
}

bool CalculiXCoreInterface::step_add_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids)
{
  return ccx_core.step_add_fieldoutputs(step_id,fieldoutput_ids);
}

bool CalculiXCoreInterface::step_remove_loads(int step_id, int load_type, std::vector<int> load_ids)
{
  return ccx_core.step_remove_loads(step_id, load_type, load_ids);
}

bool CalculiXCoreInterface::step_remove_bcs(int step_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.step_remove_bcs(step_id, bc_type, bc_ids);
}

bool CalculiXCoreInterface::step_remove_historyoutputs(int step_id,std::vector<int> historyoutput_ids)
{
  return ccx_core.step_remove_historyoutputs(step_id,historyoutput_ids);
}

bool CalculiXCoreInterface::step_remove_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids)
{
  return ccx_core.step_remove_fieldoutputs(step_id,fieldoutput_ids);
}

bool CalculiXCoreInterface::create_job(std::vector<std::string> options)
{
  return ccx_core.create_job(options);
}

bool CalculiXCoreInterface::modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_job(job_id, options, options_marker);
}

bool CalculiXCoreInterface::delete_job(int job_id)
{
  return ccx_core.delete_job(job_id);
}

bool CalculiXCoreInterface::run_job(int job_id, int option)
{
  return ccx_core.run_job(job_id, option);
}

bool CalculiXCoreInterface::check_jobs()
{
  return ccx_core.check_jobs();
}

bool CalculiXCoreInterface::check_zombie()
{
  return ccx_core.check_zombie();
}

bool CalculiXCoreInterface::wait_job(int job_id)
{
  return ccx_core.wait_job(job_id);
}

bool CalculiXCoreInterface::kill_job(int job_id)
{
  return ccx_core.kill_job(job_id);
}

bool CalculiXCoreInterface::set_job_conversion(int job_id, int conversion)
{
  return ccx_core.set_job_conversion(job_id,conversion);
}

bool CalculiXCoreInterface::result_ccx2paraview_job(int job_id)
{
  return ccx_core.result_ccx2paraview_job(job_id);
}

bool CalculiXCoreInterface::result_cgx_job(int job_id)
{
  return ccx_core.result_cgx_job(job_id);
}

bool CalculiXCoreInterface::result_paraview_job(int job_id)
{
  return ccx_core.result_paraview_job(job_id);
}

std::vector<std::string> CalculiXCoreInterface::get_job_data(int job_id)
{
  return ccx_core.get_job_data(job_id);
}

std::vector<std::string> CalculiXCoreInterface::get_job_console_output(int job_id)
{
  return ccx_core.get_job_console_output(job_id);
}

std::vector<std::string> CalculiXCoreInterface::get_job_cvg(int job_id)
{
  return ccx_core.get_job_cvg(job_id);
}

std::vector<std::string> CalculiXCoreInterface::get_job_sta(int job_id)
{
  return ccx_core.get_job_sta(job_id);
}

bool CalculiXCoreInterface::create_result(int job_id)
{
  return ccx_core.create_result(job_id);
}

bool CalculiXCoreInterface::delete_result(int job_id)
{
  return ccx_core.delete_result(job_id);
}

bool CalculiXCoreInterface::load_result(int job_id)
{
  return ccx_core.load_result(job_id);
}

int CalculiXCoreInterface::convert_result(int job_id, int option, std::vector<int> block_ids, std::vector<int> nodeset_ids, std::vector<int> sideset_ids)
{
  return ccx_core.convert_result(job_id, option, block_ids, nodeset_ids, sideset_ids);
}

bool CalculiXCoreInterface::project_result(int job_id,int step,int totalincrement,double scale)
{
  return ccx_core.project_result(job_id,step,totalincrement,scale);
}

double CalculiXCoreInterface::compute_von_mises_stress(std::vector<double> vec)
{
  return ccx_core.compute_von_mises_stress(vec);
}

double CalculiXCoreInterface::compute_von_mises_strain(std::vector<double> vec)
{
  return ccx_core.compute_von_mises_strain(vec);
}

bool CalculiXCoreInterface::create_customline(std::vector<std::string> options)
{
  return ccx_core.create_customline(options);
}

bool CalculiXCoreInterface::modify_customline(int customline_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_customline(customline_id, options, options_marker);
}

bool CalculiXCoreInterface::delete_customline(int customline_id)
{
  return ccx_core.delete_customline(customline_id);
}

std::vector<std::string> CalculiXCoreInterface::get_customline_data(std::string position,std::string keyword,int keyword_id)
{
  return ccx_core.get_customline_data(position,keyword,keyword_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_entities(std::string entity, int id)
{
  return ccx_core.get_entities(entity,id);
}

QIcon* CalculiXCoreInterface::getIcon(std::string name)
{
  return ccx_core.getIcon(name);
}

QIcon CalculiXCoreInterface::getIcon2(std::string name)
{
  return ccx_core.getIcon2(name);
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

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_material_group_properties()
{ 
  return ccx_core.get_material_group_properties();
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

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_loadsheatfluxes_tree_data()
{ 
  return ccx_core.get_loadsheatfluxes_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_loadsgravity_tree_data()
{ 
  return ccx_core.get_loadsgravity_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_loadscentrifugal_tree_data()
{ 
  return ccx_core.get_loadscentrifugal_tree_data();
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

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_hbcsdisplacements_tree_data()
{ 
  return ccx_core.get_hbcsdisplacements_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_hbcstemperatures_tree_data()
{ 
  return ccx_core.get_hbcstemperatures_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_steps_tree_data()
{ 
  return ccx_core.get_steps_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_steps_loadsforces_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadsforces_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_steps_loadspressures_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadspressures_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_steps_loadsheatfluxes_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadsheatfluxes_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_steps_loadsgravity_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadsgravity_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_steps_loadscentrifugal_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadscentrifugal_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_steps_bcsdisplacements_tree_data(int step_id)
{ 
  return ccx_core.get_steps_bcsdisplacements_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_steps_bcstemperatures_tree_data(int step_id)
{ 
  return ccx_core.get_steps_bcstemperatures_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_steps_historyoutputs_tree_data(int step_id)
{ 
  return ccx_core.get_steps_historyoutputs_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_steps_fieldoutputs_tree_data(int step_id)
{ 
  return ccx_core.get_steps_fieldoutputs_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_customlines_tree_data()
{ 
  return ccx_core.get_customlines_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_jobs_tree_data()
{ 
  return ccx_core.get_jobs_tree_data();
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

std::string CalculiXCoreInterface::get_initialcondition_export_data() // gets the export data from core
{
  return ccx_core.get_initialcondition_export_data();
}

std::string CalculiXCoreInterface::get_hbc_export_data() // gets the export data from core
{
  return ccx_core.get_hbc_export_data();
}

std::string CalculiXCoreInterface::get_step_export_data() // gets the export data from core
{
  return ccx_core.get_step_export_data();
}