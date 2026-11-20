#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Core/CalculiXCore.hpp"
#include "CoreConstraints.hpp"
#include "CoreCubitContactImport.hpp"
#include "CoreInteractionsTreeData.hpp"

std::vector<int> CalculiXCore::get_equation_ids() {
  return constraints->get_equation_ids();
}

std::vector<int> CalculiXCore::get_equationgroup_ids() {
  return constraints->get_equationgroup_ids();
}


bool CalculiXCore::create_constraint(std::string constraint_type, std::vector<std::string> options, std::vector<std::vector<double>> options2) {
  const bool accepted = constraints->create_constraint(constraint_type, options, options2);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXCore::modify_constraint(std::string constraint_type, int constraint_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<double>> options2) {
  const bool accepted = constraints->modify_constraint(constraint_type, constraint_id, options, options_marker, options2);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXCore::delete_constraint(int constraint_id) {
  return constraints->delete_constraint(constraint_id);
}

bool CalculiXCore::create_constraint_tie_from_cubitcontactpair(std::string name, std::string position_tolerance) {
  const bool accepted = CoreCubitContactImport::create_ties(*constraints, *me_iface, name, position_tolerance);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXCore::create_constraint_equation_from_coincident_nodes(std::string name, int group_id, double tolerance, bool dof_1, bool dof_2, bool dof_3) {
  const bool accepted = constraints->create_constraint_equation_from_coincident_nodes(name, group_id, tolerance, dof_1, dof_2, dof_3);
  if (!accepted) {
    return false;
  }
  return true;
}

std::string CalculiXCore::get_constraint_export_data() {
  return constraints->get_constraint_export();
}

std::vector<std::vector<std::string>> CalculiXCore::get_constraints_tree_data() {
  return CoreInteractionsTreeData::get_constraints_tree_data(*constraints);
}
