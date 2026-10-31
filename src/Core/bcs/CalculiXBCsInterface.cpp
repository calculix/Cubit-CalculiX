#include "CoreHBCExport.hpp"
#include "CoreBCRemoval.hpp"
#include "CalculiXBCsInterface.hpp"
#include "CoreBCsTreeData.hpp"
#include "CoreBCsDisplacements.hpp"
#include "CoreBCsTemperatures.hpp"
#include "CoreHBCs.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::vector<int> CalculiXBCsInterface::get_bcsdisplacements_ids()
{
  return ccx_core.bcsdisplacements->get_bc_ids();
}

std::vector<int> CalculiXBCsInterface::get_bcstemperatures_ids()
{
  return ccx_core.bcstemperatures->get_bc_ids();
}

std::string CalculiXBCsInterface::get_hbc_export_data() // gets the export data from core
{
  return CoreHBCExport::get_export(*ccx_core.hbcs, *ccx_core.bcsdisplacements, *ccx_core.bcstemperatures, *ccx_core.customlines, *ccx_core.me_iface);
}

bool CalculiXBCsInterface::create_bcsdisplacements(std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3)
{
  return ccx_core.bcsdisplacements->create_bc(options, options2, options3);
}

bool CalculiXBCsInterface::delete_bcsdisplacements(int displacement_id)
{
  return CoreBCRemoval::delete_bc(*ccx_core.bcsdisplacements, *ccx_core.steps, *ccx_core.hbcs, *ccx_core.initialconditions, displacement_id);
}

bool CalculiXBCsInterface::modify_bcsdisplacements(int displacement_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3, std::vector<int> options_marker)
{
  return ccx_core.bcsdisplacements->modify_bc(displacement_id, options, options2, options3, options_marker);
}

bool CalculiXBCsInterface::create_bcstemperatures(std::vector<std::string> options, std::vector<double> options2)
{
  return ccx_core.bcstemperatures->create_bc(options, options2);
}

bool CalculiXBCsInterface::modify_bcstemperatures(int temperature_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return ccx_core.bcstemperatures->modify_bc(temperature_id, options, options2, options_marker);
}

bool CalculiXBCsInterface::delete_bcstemperatures(int temperature_id)
{
  return CoreBCRemoval::delete_bc(*ccx_core.bcstemperatures, *ccx_core.steps, *ccx_core.hbcs, *ccx_core.initialconditions, temperature_id);
}

bool CalculiXBCsInterface::hbc_add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.hbcs->add_bcs(bcs_id, bc_type, bc_ids);
}

bool CalculiXBCsInterface::hbc_remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.hbcs->remove_bcs(bcs_id, bc_type, bc_ids);
}

std::vector<std::vector<std::string>> CalculiXBCsInterface::get_bcsdisplacements_tree_data()
{ 
  return CoreBCsTreeData::get_bcsdisplacements_tree_data(*ccx_core.bcsdisplacements);
}

std::vector<std::vector<std::string>> CalculiXBCsInterface::get_bcstemperatures_tree_data()
{ 
  return CoreBCsTreeData::get_bcstemperatures_tree_data(*ccx_core.bcstemperatures);
}

std::vector<std::vector<std::string>> CalculiXBCsInterface::get_hbcsdisplacements_tree_data()
{ 
  return CoreBCsTreeData::get_hbcsdisplacements_tree_data(*ccx_core.hbcs, *ccx_core.bcsdisplacements);
}

std::vector<std::vector<std::string>> CalculiXBCsInterface::get_hbcstemperatures_tree_data()
{ 
  return CoreBCsTreeData::get_hbcstemperatures_tree_data(*ccx_core.hbcs, *ccx_core.bcstemperatures);
}
