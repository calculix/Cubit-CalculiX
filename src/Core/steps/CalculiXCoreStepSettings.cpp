#include "Core/CalculiXCore.hpp"
#include "CoreDamping.hpp"
#include "CorePhysicalConstants.hpp"

bool CalculiXCore::modify_damping(std::vector<std::string> options, std::vector<int> options_marker)
{
  return damping->modify_damping(options, options_marker);
}

bool CalculiXCore::delete_damping(bool delete_alpha, bool delete_beta)
{
  return damping->delete_damping(delete_alpha, delete_beta);
}

std::string CalculiXCore::get_damping_export_data()
{
  return damping->get_damping_export_data();
}

std::vector<std::vector<std::string>> CalculiXCore::get_damping_tree_data()
{
  return damping->get_tree_data();
}

bool CalculiXCore::modify_physicalconstants(std::vector<std::string> options, std::vector<int> options_marker)
{
  return physicalconstants->modify_physicalconstants(options, options_marker);
}

bool CalculiXCore::delete_physicalconstants(bool delete_absolute_zero, bool delete_stefan_boltzmann, bool delete_newton_gravity)
{
  return physicalconstants->delete_physicalconstants(delete_absolute_zero, delete_stefan_boltzmann, delete_newton_gravity);
}

std::string CalculiXCore::get_physicalconstants_export_data()
{
  return physicalconstants->get_physicalconstants_export_data();
}

std::vector<std::vector<std::string>> CalculiXCore::get_physicalconstants_tree_data()
{
  return physicalconstants->get_tree_data();
}
