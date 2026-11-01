#include "Core/CalculiXCore.hpp"
#include "CoreInitialConditions.hpp"
#include "CoreInitialConditionExport.hpp"
#include "CoreInitialConditionsTreeData.hpp"

bool CalculiXCore::create_initialcondition(std::vector<std::string> options)
{
  return initialconditions->create_initialcondition(options);
}

bool CalculiXCore::modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return initialconditions->modify_initialcondition(initialcondition_id, modify_type, options, options_marker);
}

bool CalculiXCore::add_initialcondition_stress(int initialcondition_id, int modify_type, std::vector<double> options)
{
  return initialconditions->add_initialcondition_stress(initialcondition_id, modify_type, options);
}

bool CalculiXCore::delete_initialcondition(int initialcondition_id)
{
  return initialconditions->delete_initialcondition(initialcondition_id);
}

std::string CalculiXCore::get_initialcondition_export_data()
{
  return CoreInitialConditionExport::get_export(*initialconditions, *bcsdisplacements, *bcstemperatures, *customlines, *cb, *me_iface);
}

std::vector<std::vector<std::string>> CalculiXCore::get_initialconditions_tree_data()
{
  return CoreInitialConditionsTreeData::get_initialconditions_tree_data(*initialconditions, *bcsdisplacements, *bcstemperatures);
}
