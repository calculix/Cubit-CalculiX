#include "Core/CalculiXCore.hpp"
#include "CoreBCsDisplacements.hpp"
#include "CoreBCsTemperatures.hpp"
#include "CoreHBCs.hpp"
#include "CoreBCRemoval.hpp"
#include "CoreHBCExport.hpp"
#include "CoreBCsTreeData.hpp"

std::vector<int> CalculiXCore::get_bcsdisplacements_ids()
{
  return bcsdisplacements->get_bc_ids();
}

bool CalculiXCore::create_bcsdisplacements(std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3)
{
  return bcsdisplacements->create_bc(options, options2, options3);
}

bool CalculiXCore::modify_bcsdisplacements(int displacement_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options3, std::vector<int> options_marker)
{
  return bcsdisplacements->modify_bc(displacement_id, options, options2, options3, options_marker);
}

bool CalculiXCore::delete_bcsdisplacements(int displacement_id)
{
  return CoreBCRemoval::delete_bc(*bcsdisplacements, *steps, *hbcs, *initialconditions, displacement_id);
}

std::vector<int> CalculiXCore::get_bcstemperatures_ids()
{
  return bcstemperatures->get_bc_ids();
}

bool CalculiXCore::create_bcstemperatures(std::vector<std::string> options, std::vector<double> options2)
{
  return bcstemperatures->create_bc(options, options2);
}

bool CalculiXCore::modify_bcstemperatures(int temperature_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return bcstemperatures->modify_bc(temperature_id, options, options2, options_marker);
}

bool CalculiXCore::delete_bcstemperatures(int temperature_id)
{
  return CoreBCRemoval::delete_bc(*bcstemperatures, *steps, *hbcs, *initialconditions, temperature_id);
}

bool CalculiXCore::hbc_add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  return hbcs->add_bcs(bcs_id, bc_type, bc_ids);
}

bool CalculiXCore::hbc_remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  return hbcs->remove_bcs(bcs_id, bc_type, bc_ids);
}

std::string CalculiXCore::get_hbc_export_data()
{
  return CoreHBCExport::get_export(*hbcs, *bcsdisplacements, *bcstemperatures, *customlines, *me_iface);
}

std::vector<std::vector<std::string>> CalculiXCore::get_bcsdisplacements_tree_data()
{
  return CoreBCsTreeData::get_bcsdisplacements_tree_data(*bcsdisplacements);
}

std::vector<std::vector<std::string>> CalculiXCore::get_bcstemperatures_tree_data()
{
  return CoreBCsTreeData::get_bcstemperatures_tree_data(*bcstemperatures);
}

std::vector<std::vector<std::string>> CalculiXCore::get_hbcsdisplacements_tree_data()
{
  return CoreBCsTreeData::get_hbcsdisplacements_tree_data(*hbcs, *bcsdisplacements);
}

std::vector<std::vector<std::string>> CalculiXCore::get_hbcstemperatures_tree_data()
{
  return CoreBCsTreeData::get_hbcstemperatures_tree_data(*hbcs, *bcstemperatures);
}
