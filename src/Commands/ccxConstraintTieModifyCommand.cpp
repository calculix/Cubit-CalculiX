#include "ccxConstraintTieModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxConstraintTieModifyCommand::ccxConstraintTieModifyCommand()
{}

ccxConstraintTieModifyCommand::~ccxConstraintTieModifyCommand()
{}

std::vector<std::string> ccxConstraintTieModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("modify constraint tie <value:label='constraint id',help='<constraint id>'> ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[master <value:label='master id',help='<master id>'>] " );
  syntax.append("[slave <value:label='slave id',help='<slave id>'>] " );
  syntax.append("[position tolerance <value:label='position tolerance',help='<position tolerance>'>] ");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxConstraintTieModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify constraint tie <constraint id> [name <name>] [master <sideset id>] [slave <sideset id>] [position tolerance <position tolerance>]"; 

  return help;
}

std::vector<std::string> ccxConstraintTieModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxConstraintTieModifyCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string name;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  double position_tolerance_value;
  std::string position_tolerance;
  int master_value;
  int slave_value;
  std::string master_id;
  std::string slave_id;
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
  if (!data.get_value("master id", master_value))
  {
    master_id = "";
    options_marker.push_back(0);
  }
  else
  {
    master_id = std::to_string(master_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("slave id", slave_value))
  {
    slave_id = "";
    options_marker.push_back(0);
  }
  else
  {
    slave_id = std::to_string(slave_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("position tolerance", position_tolerance_value))
  {
    position_tolerance = "";
    options_marker.push_back(0);
  }
  else
  {
    position_tolerance = std::to_string(position_tolerance_value);
    options_marker.push_back(1);
  }
  
  options.push_back(name);
  options.push_back(master_id);
  options.push_back(slave_id);
  options.push_back(position_tolerance);

  if (!ccx_iface.modify_constraint("TIE",constraint_id,options,options_marker))
  {
    output = "Failed with ID " + std::to_string(constraint_id) + "!\n";
    PRINT_ERROR(output.c_str());
    return false;
  }

  return true;
}