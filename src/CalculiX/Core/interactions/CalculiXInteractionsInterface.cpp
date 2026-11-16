#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "CoreCubitContactImport.hpp"
#include "CoreContactPairs.hpp"
#include "CalculiXInteractionsInterface.hpp"
#include "CoreInteractionsTreeData.hpp"
#include "CoreSurfaceInteractions.hpp"
#include "CoreConstraints.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

bool CalculiXInteractionsInterface::get_rotation_node_ids(std::set<int>& nodes, std::string& error) const
{
  if (ccx_core.constraints == nullptr)
  {
    error = "Constraint storage is unavailable.";
    return false;
  }
  return ccx_core.constraints->get_rotation_node_ids(nodes, error);
}

std::string CalculiXInteractionsInterface::get_surfaceinteraction_name(int surfaceinteraction_id) {
  return ccx_core.surfaceinteractions->get_surfaceinteraction_name(surfaceinteraction_id);
}

std::vector<std::string> CalculiXInteractionsInterface::get_contactpair_master_slave(int contactpair_id) {
  return ccx_core.contactpairs->get_contactpair_master_slave(*ccx_core.me_iface, contactpair_id);
}

std::vector<int> CalculiXInteractionsInterface::get_equation_ids() {
  return ccx_core.constraints->get_equation_ids();
}

std::vector<int> CalculiXInteractionsInterface::get_equationgroup_ids() {
  return ccx_core.constraints->get_equationgroup_ids();
}

std::string CalculiXInteractionsInterface::get_constraint_export_data() {
  return ccx_core.constraints->get_constraint_export();
}

std::string CalculiXInteractionsInterface::get_surfaceinteraction_export_data() {
  return ccx_core.surfaceinteractions->get_surfaceinteraction_export();
}

std::string CalculiXInteractionsInterface::get_contactpair_export_data() {
  return ccx_core.contactpairs->get_contactpair_export();
}

bool CalculiXInteractionsInterface::create_constraint(std::string constraint_type, std::vector<std::string> options, std::vector<std::vector<double>> options2) {
  const bool accepted = ccx_core.constraints->create_constraint(constraint_type, options, options2);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXInteractionsInterface::modify_constraint(std::string constraint_type, int constraint_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<double>> options2) {
  const bool accepted = ccx_core.constraints->modify_constraint(constraint_type, constraint_id, options, options_marker, options2);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXInteractionsInterface::delete_constraint(int constraint_id) {
  return ccx_core.constraints->delete_constraint(constraint_id);
}

bool CalculiXInteractionsInterface::create_constraint_tie_from_cubitcontactpair(std::string name, std::string position_tolerance) {
  const bool accepted = CoreCubitContactImport::create_ties(*ccx_core.constraints, *ccx_core.me_iface, name, position_tolerance);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXInteractionsInterface::create_constraint_equation_from_coincident_nodes(std::string name, int group_id, double tolerance, bool dof_1, bool dof_2, bool dof_3) {
  const bool accepted = ccx_core.constraints->create_constraint_equation_from_coincident_nodes(name, group_id, tolerance, dof_1, dof_2, dof_3);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXInteractionsInterface::create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2) {
  const bool accepted = ccx_core.surfaceinteractions->create_surfaceinteraction(surfacebehavior_type, options, options2);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXInteractionsInterface::modify_surfaceinteraction(std::string modify_type, int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2) {
  const bool accepted = ccx_core.surfaceinteractions->modify_surfaceinteraction(modify_type, surfaceinteraction_id, options, options_marker, options2);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXInteractionsInterface::delete_surfaceinteraction(int surfaceinteraction_id) {
  return ccx_core.surfaceinteractions->delete_surfaceinteraction(surfaceinteraction_id);
}

bool CalculiXInteractionsInterface::create_contactpair(std::vector<std::string> options) {
  const bool accepted = ccx_core.contactpairs->create_contactpair(options);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXInteractionsInterface::modify_contactpair(int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker) {
  const bool accepted = ccx_core.contactpairs->modify_contactpair(contactpair_id, options, options_marker);
  if (!accepted) {
    return false;
  }
  return true;
}

bool CalculiXInteractionsInterface::delete_contactpair(int contactpair_id) {
  return ccx_core.contactpairs->delete_contactpair(contactpair_id);
}

bool CalculiXInteractionsInterface::create_contactpair_from_cubitcontactpair(int surfaceinteraction_id, std::string contactpairtype, std::string adjust, std::string adjust_nodeset) {
  const bool accepted = CoreCubitContactImport::create_pairs(*ccx_core.contactpairs, *ccx_core.surfaceinteractions, *ccx_core.me_iface, surfaceinteraction_id, contactpairtype, adjust, adjust_nodeset);
  if (!accepted) {
    return false;
  }
  return true;
}

std::vector<std::vector<std::string>> CalculiXInteractionsInterface::get_constraints_tree_data() {
  return CoreInteractionsTreeData::get_constraints_tree_data(*ccx_core.constraints);
}

std::vector<std::vector<std::string>> CalculiXInteractionsInterface::get_surfaceinteractions_tree_data() {
  return CoreInteractionsTreeData::get_surfaceinteractions_tree_data(*ccx_core.surfaceinteractions);
}

std::vector<std::vector<std::string>> CalculiXInteractionsInterface::get_contactpairs_tree_data() {
  return CoreInteractionsTreeData::get_contactpairs_tree_data(*ccx_core.contactpairs);
}


bool CalculiXInteractionsInterface::get_constraint_settings(int id, ConstraintsSettings& settings, std::string& error) {
  return ccx_core.constraints->get_settings(id, settings, error);
}


bool CalculiXInteractionsInterface::get_contactpair_settings(int id, ContactPairsSettings& settings, std::string& error) {
  return ccx_core.contactpairs->get_settings(id, settings, error);
}


bool CalculiXInteractionsInterface::get_surfaceinteraction_settings(int id, SurfaceInteractionsSettings& settings, std::string& error) {
  return ccx_core.surfaceinteractions->get_settings(id, settings, error);
}
