#include "ccxConstraintEquationCreateFromCoincidentNodesCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxConstraintEquationCreateFromCoincidentNodesCommand::ccxConstraintEquationCreateFromCoincidentNodesCommand()
{}

ccxConstraintEquationCreateFromCoincidentNodesCommand::~ccxConstraintEquationCreateFromCoincidentNodesCommand()
{}

std::vector<std::string> ccxConstraintEquationCreateFromCoincidentNodesCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("create constraint equation ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("group_id <value:label='group_id',help='<group_id>'> " );
  syntax.append("tolerance <value:label='tolerance',help='<tolerance>'> " );
  syntax.append("[dof_1] [dof_2] [dof_3] " );
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxConstraintEquationCreateFromCoincidentNodesCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create constraint equation name <name> group_id <group_id> tolerance <tolerance> [dof_1] [dof_2] [dof_3]"; 

  return help;
}

std::vector<std::string> ccxConstraintEquationCreateFromCoincidentNodesCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxConstraintEquationCreateFromCoincidentNodesCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string name;
  int group_id;
  double tolerance;
  bool dof_1 = false;
  bool dof_2 = false;
  bool dof_3 = false;

  data.get_string("name", name);
  data.get_value("group_id", group_id);
  data.get_value("tolerance", tolerance);
  
  if (data.find_keyword("DOF_1"))
  {
    dof_1 = true;
  }
  if (data.find_keyword("DOF_2"))
  {
    dof_2 = true;
  }
  if (data.find_keyword("DOF_3"))
  {
    dof_3 = true;
  }

  if (!ccx_iface.create_constraint_equation_from_coincident_nodes(name, group_id, tolerance, dof_1, dof_2, dof_3))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  
  return true;
}