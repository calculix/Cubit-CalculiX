#include "CoreInitialConditionExport.hpp"
#include "CoreStepModelChange.hpp"
#include "CalculiXStepsInterface.hpp"
#include "CoreOutputsTreeData.hpp"
#include "CoreHistoryOutputs.hpp"
#include "CoreFieldOutputs.hpp"
#include "CoreInitialConditionsTreeData.hpp"
#include "CoreInitialConditions.hpp"
#include "CoreSteps.hpp"
#include "CoreStepsBCTreeData.hpp"
#include "CoreStepsLoadTreeData.hpp"
#include "CoreStepsTreeData.hpp"
#include <string>
#include <vector>

#include "Core/steps/CoreDamping.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include "Core/loadCalculiXCore.hpp"

bool CalculiXStepsInterface::check_step_exists(int step_id)
{
  return ccx_core.steps->check_step_exists(step_id);
}

std::string CalculiXStepsInterface::get_damping_export_data()
{
  return ccx_core.damping->get_damping_export_data();
}

std::string CalculiXStepsInterface::get_physicalconstants_export_data()
{
  return ccx_core.physicalconstants->get_physicalconstants_export_data();
}

std::string CalculiXStepsInterface::get_initialcondition_export_data()
{
  return CoreInitialConditionExport::get_export(*ccx_core.initialconditions, *ccx_core.bcsdisplacements, *ccx_core.bcstemperatures, *ccx_core.customlines, *ccx_core.cb, *ccx_core.me_iface);
}

std::string CalculiXStepsInterface::get_step_export_data() // gets the export data from core
{
  return ccx_core.get_step_export_data();
}

std::vector<int> CalculiXStepsInterface::get_steps_ids() //gets the step ids from core
{
  return ccx_core.steps->get_steps_ids();
}

bool CalculiXStepsInterface::modify_damping(std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.damping->modify_damping(options, options_marker);
}

bool CalculiXStepsInterface::delete_damping(bool delete_alpha, bool delete_beta)
{
  return ccx_core.damping->delete_damping(delete_alpha, delete_beta);
}

bool CalculiXStepsInterface::modify_physicalconstants(std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.physicalconstants->modify_physicalconstants(options, options_marker);
}

bool CalculiXStepsInterface::delete_physicalconstants(bool delete_absolute_zero, bool delete_stefan_boltzmann, bool delete_newton_gravity)
{
  return ccx_core.physicalconstants->delete_physicalconstants(delete_absolute_zero, delete_stefan_boltzmann, delete_newton_gravity);
}

bool CalculiXStepsInterface::create_historyoutput(std::vector<std::string> options)
{
  return ccx_core.historyoutputs->create_output(options);
}

bool CalculiXStepsInterface::modify_historyoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.historyoutputs->modify_output(output_id, modify_type, options, options_marker);
}

bool CalculiXStepsInterface::delete_historyoutput(int output_id)
{
  return ccx_core.historyoutputs->delete_output(output_id);
}

std::vector<std::string> CalculiXStepsInterface::get_historyoutput_node_keys()
{
  return ccx_core.historyoutputs->node_keys;
}

std::vector<std::string> CalculiXStepsInterface::get_historyoutput_element_keys()
{
  return ccx_core.historyoutputs->element_keys;
}

std::vector<std::string> CalculiXStepsInterface::get_historyoutput_contact_keys()
{
  return ccx_core.historyoutputs->contact_keys;
}

std::vector<std::string> CalculiXStepsInterface::get_historyoutput_section_keys()
{
  return ccx_core.historyoutputs->section_keys;
}

bool CalculiXStepsInterface::create_fieldoutput(std::vector<std::string> options)
{
  return ccx_core.fieldoutputs->create_output(options);
}

bool CalculiXStepsInterface::modify_fieldoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.fieldoutputs->modify_output(output_id, modify_type, options, options_marker);
}

bool CalculiXStepsInterface::delete_fieldoutput(int output_id)
{
  return ccx_core.fieldoutputs->delete_output(output_id);
}

std::vector<std::string> CalculiXStepsInterface::get_fieldoutput_node_keys()
{
  return ccx_core.fieldoutputs->node_keys;
}

std::vector<std::string> CalculiXStepsInterface::get_fieldoutput_element_keys()
{
  return ccx_core.fieldoutputs->element_keys;
}

std::vector<std::string> CalculiXStepsInterface::get_fieldoutput_contact_keys()
{
  return ccx_core.fieldoutputs->contact_keys;
}

bool CalculiXStepsInterface::create_initialcondition(std::vector<std::string> options)
{
  return ccx_core.initialconditions->create_initialcondition(options);
}

bool CalculiXStepsInterface::modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.initialconditions->modify_initialcondition(initialcondition_id, modify_type, options, options_marker);
}

bool CalculiXStepsInterface::add_initialcondition_stress(int initialcondition_id, int modify_type, std::vector<double> options)
{
  return ccx_core.initialconditions->add_initialcondition_stress(initialcondition_id, modify_type, options);
}

bool CalculiXStepsInterface::delete_initialcondition(int initialcondition_id)
{
  return ccx_core.initialconditions->delete_initialcondition(initialcondition_id);
}

bool CalculiXStepsInterface::create_step(std::vector<std::string> options)
{
  return ccx_core.steps->create_step(options);
}

bool CalculiXStepsInterface::modify_step(int step_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.steps->modify_step(step_id, modify_type, options, options_marker);
}

bool CalculiXStepsInterface::delete_step(int step_id)
{
  return ccx_core.steps->delete_step(step_id);
}

bool CalculiXStepsInterface::step_add_loads(int step_id, int load_type, std::vector<int> load_ids)
{
  return ccx_core.steps->add_loads(step_id, load_type, load_ids);
}

bool CalculiXStepsInterface::step_add_bcs(int step_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.steps->add_bcs(step_id, bc_type, bc_ids);
}

bool CalculiXStepsInterface::step_add_historyoutputs(int step_id, std::vector<int> historyoutput_ids)
{
  return ccx_core.steps->add_historyoutputs(step_id, historyoutput_ids);
}

bool CalculiXStepsInterface::step_add_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids)
{
  return ccx_core.steps->add_fieldoutputs(step_id, fieldoutput_ids);
}

bool CalculiXStepsInterface::step_remove_loads(int step_id, int load_type, std::vector<int> load_ids)
{
  return ccx_core.steps->remove_loads(step_id, load_type, load_ids);
}

bool CalculiXStepsInterface::step_remove_bcs(int step_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.steps->remove_bcs(step_id, bc_type, bc_ids);
}

bool CalculiXStepsInterface::step_remove_historyoutputs(int step_id, std::vector<int> historyoutput_ids)
{
  return ccx_core.steps->remove_historyoutputs(step_id, historyoutput_ids);
}

bool CalculiXStepsInterface::step_remove_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids)
{
  return ccx_core.steps->remove_fieldoutputs(step_id, fieldoutput_ids);
}

bool CalculiXStepsInterface::step_utility_modelchangeelement_dummystep(int step_id, std::vector<int> trajectory_ids)
{
  return CoreStepModelChange::create_dummy_step(*ccx_core.steps, *ccx_core.customlines, *ccx_core.loadstrajectory, step_id, trajectory_ids);
}

StepSplitResult CalculiXStepsInterface::step_utility_split_step(int step_id, std::vector<std::vector<double>> times)
{
  return ccx_core.steps->split_step(step_id, times);
}

bool CalculiXStepsInterface::step_utility_save_backup()
{
  return ccx_core.steps->save_backup();
}

bool CalculiXStepsInterface::step_utility_load_backup()
{
  return ccx_core.steps->load_backup();
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_damping_tree_data()
{ 
  return ccx_core.damping->get_tree_data();
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_physicalconstants_tree_data()
{ 
  return ccx_core.physicalconstants->get_tree_data();
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_historyoutputs_tree_data()
{ 
  return CoreOutputsTreeData::get_historyoutputs_tree_data(*ccx_core.historyoutputs);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_fieldoutputs_tree_data()
{ 
  return CoreOutputsTreeData::get_fieldoutputs_tree_data(*ccx_core.fieldoutputs);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_initialconditions_tree_data()
{ 
  return CoreInitialConditionsTreeData::get_initialconditions_tree_data(
    *ccx_core.initialconditions, *ccx_core.bcsdisplacements, *ccx_core.bcstemperatures);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_tree_data()
{ 
  return CoreStepsTreeData::get_steps_tree_data(*ccx_core.steps);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsforces_tree_data(int step_id)
{ 
  return CoreStepsLoadTreeData::get_steps_loadsforces_tree_data(*ccx_core.steps, *ccx_core.loadsforces, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadspressures_tree_data(int step_id)
{ 
  return CoreStepsLoadTreeData::get_steps_loadspressures_tree_data(*ccx_core.steps, *ccx_core.loadspressures, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsheatfluxes_tree_data(int step_id)
{ 
  return CoreStepsLoadTreeData::get_steps_loadsheatfluxes_tree_data(*ccx_core.steps, *ccx_core.loadsheatfluxes, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsgravity_tree_data(int step_id)
{ 
  return CoreStepsLoadTreeData::get_steps_loadsgravity_tree_data(*ccx_core.steps, *ccx_core.loadsgravity, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadscentrifugal_tree_data(int step_id)
{ 
  return CoreStepsLoadTreeData::get_steps_loadscentrifugal_tree_data(*ccx_core.steps, *ccx_core.loadscentrifugal, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadstrajectory_tree_data(int step_id)
{ 
  return CoreStepsLoadTreeData::get_steps_loadstrajectory_tree_data(*ccx_core.steps, *ccx_core.loadstrajectory, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsfilm_tree_data(int step_id)
{ 
  return CoreStepsLoadTreeData::get_steps_loadsfilm_tree_data(*ccx_core.steps, *ccx_core.loadsfilm, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsradiation_tree_data(int step_id)
{ 
  return CoreStepsLoadTreeData::get_steps_loadsradiation_tree_data(*ccx_core.steps, *ccx_core.loadsradiation, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadssurfacetraction_tree_data(int step_id)
{ 
  return CoreStepsLoadTreeData::get_steps_loadssurfacetraction_tree_data(*ccx_core.steps, *ccx_core.loadssurfacetraction, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_loadsbodyheatflux_tree_data(int step_id)
{ 
  return CoreStepsLoadTreeData::get_steps_loadsbodyheatflux_tree_data(*ccx_core.steps, *ccx_core.loadsbodyheatflux, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_bcsdisplacements_tree_data(int step_id)
{ 
  return CoreStepsBCTreeData::get_steps_bcsdisplacements_tree_data(*ccx_core.steps, *ccx_core.bcsdisplacements, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_bcstemperatures_tree_data(int step_id)
{ 
  return CoreStepsBCTreeData::get_steps_bcstemperatures_tree_data(*ccx_core.steps, *ccx_core.bcstemperatures, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_historyoutputs_tree_data(int step_id)
{ 
  return CoreOutputsTreeData::get_steps_historyoutputs_tree_data(*ccx_core.steps, *ccx_core.historyoutputs, step_id);
}

std::vector<std::vector<std::string>> CalculiXStepsInterface::get_steps_fieldoutputs_tree_data(int step_id)
{ 
  return CoreOutputsTreeData::get_steps_fieldoutputs_tree_data(*ccx_core.steps, *ccx_core.fieldoutputs, step_id);
}


bool CalculiXStepsInterface::get_step_settings(int id, StepsSettings& settings, std::string& error)
{
  return ccx_core.steps->get_settings(id, settings, error);
}


bool CalculiXStepsInterface::get_historyoutput_settings(int id, HistoryOutputsSettings& settings, std::string& error)
{
  return ccx_core.historyoutputs->get_settings(id, settings, error);
}


bool CalculiXStepsInterface::get_fieldoutput_settings(int id, FieldOutputsSettings& settings, std::string& error)
{
  return ccx_core.fieldoutputs->get_settings(id, settings, error);
}


bool CalculiXStepsInterface::get_initialcondition_settings(int id, InitialConditionsSettings& settings, std::string& error)
{
  return ccx_core.initialconditions->get_settings(id, settings, error);
}


bool CalculiXStepsInterface::get_damping_settings(int id, DampingSettings& settings, std::string& error)
{
  return ccx_core.damping->get_settings(id, settings, error);
}


bool CalculiXStepsInterface::get_physicalconstants_settings(int id, PhysicalConstantsSettings& settings, std::string& error)
{
  return ccx_core.physicalconstants->get_settings(id, settings, error);
}
