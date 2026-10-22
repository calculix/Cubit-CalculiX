#include "CalculiXInteractionsInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::string CalculiXInteractionsInterface::get_surfaceinteraction_name(int surfaceinteraction_id)
{
  return ccx_core.get_surfaceinteraction_name(surfaceinteraction_id);
}

std::vector<std::string> CalculiXInteractionsInterface::get_contactpair_master_slave(int contactpair_id)
{
  return ccx_core.get_contactpair_master_slave(contactpair_id);
}

std::vector<int> CalculiXInteractionsInterface::get_equation_ids()
{
  return ccx_core.get_equation_ids();
}

std::vector<int> CalculiXInteractionsInterface::get_equationgroup_ids()
{
  return ccx_core.get_equationgroup_ids();
}

std::string CalculiXInteractionsInterface::get_constraint_export_data() // gets the export data from constraints core
{
  return ccx_core.get_constraint_export_data();
}

std::string CalculiXInteractionsInterface::get_surfaceinteraction_export_data() // gets the export data from surfaceinteractions core
{
  return ccx_core.get_surfaceinteraction_export_data();
}

std::string CalculiXInteractionsInterface::get_contactpair_export_data() // gets the export data from contactpairs core
{
  return ccx_core.get_contactpair_export_data();
}

bool CalculiXInteractionsInterface::create_constraint(std::string constraint_type, std::vector<std::string> options,std::vector<std::vector<double>> options2)
{
  return ccx_core.create_constraint(constraint_type,options, options2);
}

bool CalculiXInteractionsInterface::modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker,std::vector<std::vector<double>> options2)
{
  return ccx_core.modify_constraint(constraint_type,constraint_id,options, options_marker,options2);
}

bool CalculiXInteractionsInterface::delete_constraint(int constraint_id)
{
  return ccx_core.delete_constraint(constraint_id);
}

bool CalculiXInteractionsInterface::create_constraint_tie_from_cubitcontactpair(std::string name, std::string position_tolerance)
{
  return ccx_core.create_constraint_tie_from_cubitcontactpair(name, position_tolerance);
}

bool CalculiXInteractionsInterface::create_constraint_equation_from_coincident_nodes(std::string name, int group_id, double tolerance, bool dof_1, bool dof_2, bool dof_3)
{
  return ccx_core.create_constraint_equation_from_coincident_nodes(name, group_id, tolerance, dof_1, dof_2, dof_3);
}

bool CalculiXInteractionsInterface::create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.create_surfaceinteraction(surfacebehavior_type,options,options2);
}

bool CalculiXInteractionsInterface::modify_surfaceinteraction(std::string modify_type, int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.modify_surfaceinteraction(modify_type,surfaceinteraction_id,options,options_marker,options2);
}

bool CalculiXInteractionsInterface::delete_surfaceinteraction(int surfaceinteraction_id)
{
  return ccx_core.delete_surfaceinteraction(surfaceinteraction_id);
}

bool CalculiXInteractionsInterface::create_contactpair(std::vector<std::string> options)
{
  return ccx_core.create_contactpair(options);
}

bool CalculiXInteractionsInterface::modify_contactpair(int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_contactpair(contactpair_id,options,options_marker);
}

bool CalculiXInteractionsInterface::delete_contactpair(int contactpair_id)
{
  return ccx_core.delete_contactpair(contactpair_id);
}

bool CalculiXInteractionsInterface::create_contactpair_from_cubitcontactpair(int surfaceinteraction_id, std::string contactpairtype, std::string adjust, std::string adjust_nodeset)
{
  return ccx_core.create_contactpair_from_cubitcontactpair(surfaceinteraction_id,contactpairtype,adjust,adjust_nodeset);
}

std::vector<std::vector<std::string>> CalculiXInteractionsInterface::get_constraints_tree_data()
{ 
  return ccx_core.get_constraints_tree_data();
}

std::vector<std::vector<std::string>> CalculiXInteractionsInterface::get_surfaceinteractions_tree_data()
{ 
  return ccx_core.get_surfaceinteractions_tree_data();
}

std::vector<std::vector<std::string>> CalculiXInteractionsInterface::get_contactpairs_tree_data()
{ 
  return ccx_core.get_contactpairs_tree_data();
}
