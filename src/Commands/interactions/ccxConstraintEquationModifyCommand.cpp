#include "ccxConstraintEquationModifyCommand.hpp"
#include <string>
#include <vector>
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxConstraintEquationModifyCommand::ccxConstraintEquationModifyCommand()
{}

ccxConstraintEquationModifyCommand::~ccxConstraintEquationModifyCommand()
{}

std::vector<std::string> ccxConstraintEquationModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("modify constraint equation <value:label='constraint id',help='<constraint id>'> ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[node_dof_coefficient <value:label='node_dof_coefficient',help='<node_dof_coefficient>'>...]" );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxConstraintEquationModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify constraint equation <constraint id> [name <name>] [node_dof_coefficient <node_dof_coefficient>...]"; 

  return help;
}

std::vector<std::string> ccxConstraintEquationModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxConstraintEquationModifyCommand::execute(CubitCommandData &data)
{
  CalculiXInteractionsInterface ccx_interactions_iface;

  

  std::string output;

  std::string name;
  std::vector<std::string> options;
  std::vector<std::vector<double>> options2;
  std::vector<int> options_marker;
  std::vector<double> node_dof_coefficient;
  int constraint_id;

  data.get_value("constraint id", constraint_id);

  if (!data.get_string("name", name))
  {
    name = "";
    options_marker.push_back(0);
  }
  else
  {
    options_marker.push_back(1);
  }
  options.push_back(name);
  
  data.get_values("node_dof_coefficient", node_dof_coefficient);
  
  if (node_dof_coefficient.size() % 3 != 0)
  {
    output = "Failed! node_dof_coefficient must be 3 numbers per node!\n";
    PRINT_ERROR(output.c_str());
    return false;
  }

  for (size_t i = 0; i < int(node_dof_coefficient.size()/3); i++)
  {
    options2.push_back({node_dof_coefficient[i*3+0],node_dof_coefficient[i*3+1],node_dof_coefficient[i*3+2]});
  }

  if (!ccx_interactions_iface.modify_constraint("EQUATION",constraint_id,options,options_marker,options2))
  {
    output = "Failed with ID " + std::to_string(constraint_id) + "!\n";
    PRINT_ERROR(output.c_str());
    return false;
  }

  return true;
}