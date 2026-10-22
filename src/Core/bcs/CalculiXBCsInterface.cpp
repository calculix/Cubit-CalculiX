#include "CalculiXBCsInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::vector<int> CalculiXBCsInterface::get_bcsdisplacements_ids()
{
  return ccx_core.get_bcsdisplacements_ids();
}

std::vector<int> CalculiXBCsInterface::get_bcstemperatures_ids()
{
  return ccx_core.get_bcstemperatures_ids();
}

std::string CalculiXBCsInterface::get_hbc_export_data() // gets the export data from core
{
  return ccx_core.get_hbc_export_data();
}

bool CalculiXBCsInterface::modify_bcsdisplacements(int displacement_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_bcsdisplacements(displacement_id,options,options_marker);
}

bool CalculiXBCsInterface::modify_bcstemperatures(int temperature_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_bcstemperatures(temperature_id,options,options_marker);
}

bool CalculiXBCsInterface::hbc_add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.hbc_add_bcs(bcs_id, bc_type, bc_ids);
}

bool CalculiXBCsInterface::hbc_remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  return ccx_core.hbc_remove_bcs(bcs_id, bc_type, bc_ids);
}

std::vector<std::vector<std::string>> CalculiXBCsInterface::get_bcsdisplacements_tree_data()
{ 
  return ccx_core.get_bcsdisplacements_tree_data();
}

std::vector<std::vector<std::string>> CalculiXBCsInterface::get_bcstemperatures_tree_data()
{ 
  return ccx_core.get_bcstemperatures_tree_data();
}

std::vector<std::vector<std::string>> CalculiXBCsInterface::get_hbcsdisplacements_tree_data()
{ 
  return ccx_core.get_hbcsdisplacements_tree_data();
}

std::vector<std::vector<std::string>> CalculiXBCsInterface::get_hbcstemperatures_tree_data()
{ 
  return ccx_core.get_hbcstemperatures_tree_data();
}
