#include "ccxConstraintTieCreateFromCubitContactPairCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxConstraintTieCreateFromCubitContactPairCommand::ccxConstraintTieCreateFromCubitContactPairCommand()
{}

ccxConstraintTieCreateFromCubitContactPairCommand::~ccxConstraintTieCreateFromCubitContactPairCommand()
{}

std::vector<std::string> ccxConstraintTieCreateFromCubitContactPairCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create constraint tie ");
  syntax.append("from cubitcontactpair ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("[position tolerance <value:label='position tolerance',help='<position tolerance>'>] ");
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxConstraintTieCreateFromCubitContactPairCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create constraint tie from cubitcontactpair name <name_prefix> [position tolerance <value>]"; 

  return help;
}

std::vector<std::string> ccxConstraintTieCreateFromCubitContactPairCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxConstraintTieCreateFromCubitContactPairCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;
  
  std::string output;

  std::string name;
  std::vector<std::string> options;
  double position_tolerance_value;
  std::string position_tolerance;

  data.get_string("name", name);

  if (!data.get_value("position tolerance", position_tolerance_value))
  {
    position_tolerance = "";
  }
  else
  {
    position_tolerance = std::to_string(position_tolerance_value);
  }
  
  options.push_back(name);
  options.push_back(position_tolerance);

  if (!ccx_iface.create_constraint_tie_from_cubitcontactpair(options[0],options[1]))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear(); 
  return true;
}