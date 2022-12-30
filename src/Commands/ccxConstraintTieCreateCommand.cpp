#include "ccxConstraintTieCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxConstraintTieCreateCommand::ccxConstraintTieCreateCommand()
{}

ccxConstraintTieCreateCommand::~ccxConstraintTieCreateCommand()
{}

std::vector<std::string> ccxConstraintTieCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("create constraint tie ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("master <value:label='master id',help='<master id>'> " );
  syntax.append("slave <value:label='slave id',help='<slave id>'> " );
  syntax.append("[position tolerance <value:label='position tolerance',help='<position tolerance>'>] ");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxConstraintTieCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create constraint tie name <name> master <sideset id> slave <sideset id> [position tolerance <position tolerance>]"; 

  return help;
}

std::vector<std::string> ccxConstraintTieCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxConstraintTieCreateCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string name;
  std::vector<std::string> options;
  double position_tolerance_value;
  std::string position_tolerance;
  int master_value;
  int slave_value;
  std::string master_id;
  std::string slave_id;

  data.get_string("name", name);
  data.get_value("master id", master_value);
  data.get_value("slave id", slave_value);
  master_id = std::to_string(master_value);
  slave_id = std::to_string(slave_value);

  if (!data.get_value("position tolerance", position_tolerance_value))
  {
    position_tolerance = "";
  }
  else
  {
    position_tolerance = std::to_string(position_tolerance_value);
  }
  
  options.push_back(name);
  options.push_back(master_id);
  options.push_back(slave_id);
  options.push_back(position_tolerance);

  if (!ccx_iface.create_constraint("TIE",options))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}