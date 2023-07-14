#include "ccxHBCAddBCCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxHBCAddBCCommand::ccxHBCAddBCCommand()
{}

ccxHBCAddBCCommand::~ccxHBCAddBCCommand()
{}

std::vector<std::string> ccxHBCAddBCCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("hbc ");
  syntax.append("add bc ");
  syntax.append("[displacement <value:label='displacement id',help='<displacement id>'>...] " );
  syntax.append("[temperature <value:label='temperature id',help='<temperature id>'>...] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxHBCAddBCCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx hbc add bc [displacement <displacement id>...] [temperature <temperature id>...]"; 

  return help;
}

std::vector<std::string> ccxHBCAddBCCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxHBCAddBCCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  
  std::vector<int> displacement_ids;
  std::vector<int> temperature_ids;

  data.get_values("displacement id", displacement_ids);
  data.get_values("temperature id", temperature_ids);
   
  if (!ccx_iface.hbc_add_bcs(0, 1, displacement_ids))
  {
    output = "Failed adding Displacement!\n";
    PRINT_ERROR(output.c_str());
  }
  if (!ccx_iface.hbc_add_bcs(0, 2, temperature_ids))
  {
    output = "Failed adding Temperature!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}