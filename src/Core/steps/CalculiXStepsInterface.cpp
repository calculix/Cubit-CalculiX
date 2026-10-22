#include "CalculiXStepsInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

bool CalculiXStepsInterface::check_step_exists(int step_id)
{
  return ccx_core.check_step_exists(step_id);
}

std::string CalculiXStepsInterface::get_damping_export_data() // gets the export data from damping core
{
  return ccx_core.get_damping_export_data();
}

std::string CalculiXStepsInterface::get_physicalconstants_export_data() // gets the export data from physicalconstants core
{
  return ccx_core.get_physicalconstants_export_data();
}

std::string CalculiXStepsInterface::get_initialcondition_export_data() // gets the export data from core
{
  return ccx_core.get_initialcondition_export_data();
}

std::string CalculiXStepsInterface::get_step_export_data() // gets the export data from core
{
  return ccx_core.get_step_export_data();
}

std::vector<int> CalculiXStepsInterface::get_steps_ids() //gets the step ids from core
{
  return ccx_core.get_steps_ids();
}

bool CalculiXStepsInterface::modify_damping(std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_damping(options, options_marker);
}

bool CalculiXStepsInterface::delete_damping(bool delete_alpha, bool delete_beta)
{
  return ccx_core.delete_damping(delete_alpha, delete_beta);
}

bool CalculiXStepsInterface::modify_physicalconstants(std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_physicalconstants(options, options_marker);
}

bool CalculiXStepsInterface::delete_physicalconstants(bool delete_absolute_zero, bool delete_stefan_boltzmann, bool delete_newton_gravity)
{
  return ccx_core.delete_physicalconstants(delete_absolute_zero, delete_stefan_boltzmann, delete_newton_gravity);
}

bool CalculiXStepsInterface::create_historyoutput(std::vector<std::string> options)
{
  return ccx_core.create_historyoutput(options);
}

bool CalculiXStepsInterface::modify_historyoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_historyoutput(output_id, modify_type, options, options_marker);
}

bool CalculiXStepsInterface::delete_historyoutput(int output_id)
{
  return ccx_core.delete_historyoutput(output_id);
}

std::vector<std::string> CalculiXStepsInterface::get_historyoutput_node_keys()
{
  return  ccx_core.get_historyoutput_node_keys();
}

std::vector<std::string> CalculiXStepsInterface::get_historyoutput_element_keys()
{
  return  ccx_core.get_historyoutput_element_keys();
}

std::vector<std::string> CalculiXStepsInterface::get_historyoutput_contact_keys()
{
  return  ccx_core.get_historyoutput_contact_keys();
}

std::vector<std::string> CalculiXStepsInterface::get_historyoutput_section_keys()
{
  return  ccx_core.get_historyoutput_section_keys();
}

bool CalculiXStepsInterface::create_fieldoutput(std::vector<std::string> options)
{
  return ccx_core.create_fieldoutput(options);
}

bool CalculiXStepsInterface::modify_fieldoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_fieldoutput(output_id, modify_type, options, options_marker);
}

bool CalculiXStepsInterface::delete_fieldoutput(int output_id)
{
  return ccx_core.delete_fieldoutput(output_id);
}

std::vector<std::string> CalculiXStepsInterface::get_fieldoutput_node_keys()
{
  return  ccx_core.get_fieldoutput_node_keys();
}

std::vector<std::string> CalculiXStepsInterface::get_fieldoutput_element_keys()
{
  return  ccx_core.get_fieldoutput_element_keys();
}

std::vector<std::string> CalculiXStepsInterface::get_fieldoutput_contact_keys()
{
  return  ccx_core.get_fieldoutput_contact_keys();
}

bool CalculiXStepsInterface::create_initialcondition(std::vector<std::string> options)
{
  return ccx_core.create_initialcondition(options);
}

bool CalculiXStepsInterface::modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_initialcondition(initialcondition_id, modify_type, options, options_marker);
}

bool CalculiXStepsInterface::add_initialcondition_stress(int initialcondition_id, int modify_type, std::vector<double> options)
{
  return ccx_core.add_initialcondition_stress(initialcondition_id, modify_type, options);
}

bool CalculiXStepsInterface::delete_initialcondition(int initialcondition_id)
{
  return ccx_core.delete_initialcondition(initialcondition_id);
}

bool CalculiXStepsInterface::create_step(std::vector<std::string> options)
{
  return ccx_core.create_step(options);
}

bool CalculiXStepsInterface::modify_step(int step_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_step(step_id, modify_type, options, options_marker);
}

bool CalculiXStepsInterface::delete_step(int step_id)
{
  return ccx_core.delete_step(step_id);
}

bool CalculiXStepsInterface::step_add_loads(int step_id, int load_type, std::vector<int> load_ids)
{
  return ccx_core.step_add_loads(step_id,load_type,load_ids);
}

bool CalculiXStepsInterface::step_add_bcs(int step_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.step_add_bcs(step_id,bc_type,bc_ids);
}

bool CalculiXStepsInterface::step_add_historyoutputs(int step_id, std::vector<int> historyoutput_ids)
{
  return ccx_core.step_add_historyoutputs(step_id,historyoutput_ids);
}

bool CalculiXStepsInterface::step_add_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids)
{
  return ccx_core.step_add_fieldoutputs(step_id,fieldoutput_ids);
}

bool CalculiXStepsInterface::step_remove_loads(int step_id, int load_type, std::vector<int> load_ids)
{
  return ccx_core.step_remove_loads(step_id, load_type, load_ids);
}

bool CalculiXStepsInterface::step_remove_bcs(int step_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.step_remove_bcs(step_id, bc_type, bc_ids);
}

bool CalculiXStepsInterface::step_remove_historyoutputs(int step_id,std::vector<int> historyoutput_ids)
{
  return ccx_core.step_remove_historyoutputs(step_id,historyoutput_ids);
}

bool CalculiXStepsInterface::step_remove_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids)
{
  return ccx_core.step_remove_fieldoutputs(step_id,fieldoutput_ids);
}

bool CalculiXStepsInterface::step_utility_modelchangeelement_dummystep(int step_id, std::vector<int> trajectory_ids)
{
  return ccx_core.step_utility_modelchangeelement_dummystep(step_id, trajectory_ids);
}

StepSplitResult CalculiXStepsInterface::step_utility_split_step(int step_id, std::vector<std::vector<double>> times)
{
  return ccx_core.step_utility_split_step(step_id, times);
}

bool CalculiXStepsInterface::step_utility_save_backup()
{
  return ccx_core.step_utility_save_backup();
}

bool CalculiXStepsInterface::step_utility_load_backup()
{
  return ccx_core.step_utility_save_backup();
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_damping_tree_data()
{ 
  return ccx_core.get_damping_tree_data();
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_physicalconstants_tree_data()
{ 
  return ccx_core.get_physicalconstants_tree_data();
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_historyoutputs_tree_data()
{ 
  return ccx_core.get_historyoutputs_tree_data();
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_fieldoutputs_tree_data()
{ 
  return ccx_core.get_fieldoutputs_tree_data();
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_initialconditions_tree_data()
{ 
  return ccx_core.get_initialconditions_tree_data();
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_tree_data()
{ 
  return ccx_core.get_steps_tree_data();
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsforces_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadsforces_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadspressures_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadspressures_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsheatfluxes_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadsheatfluxes_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsgravity_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadsgravity_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadscentrifugal_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadscentrifugal_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadstrajectory_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadstrajectory_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsfilm_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadsfilm_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsradiation_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadsradiation_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadssurfacetraction_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadssurfacetraction_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsbodyheatflux_tree_data(int step_id)
{ 
  return ccx_core.get_steps_loadsbodyheatflux_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_bcsdisplacements_tree_data(int step_id)
{ 
  return ccx_core.get_steps_bcsdisplacements_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_bcstemperatures_tree_data(int step_id)
{ 
  return ccx_core.get_steps_bcstemperatures_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_historyoutputs_tree_data(int step_id)
{ 
  return ccx_core.get_steps_historyoutputs_tree_data(step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_fieldoutputs_tree_data(int step_id)
{ 
  return ccx_core.get_steps_fieldoutputs_tree_data(step_id);
}
