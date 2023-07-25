#include "ccxHBCRemoveBCCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxHBCRemoveBCCommand::ccxHBCRemoveBCCommand()
{}

ccxHBCRemoveBCCommand::~ccxHBCRemoveBCCommand()
{}

std::vector<std::string> ccxHBCRemoveBCCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("hbc ");
  syntax.append("remove bc ");
  syntax.append("[displacement <value:label='displacement id',help='<displacement id>'>...] " );
  syntax.append("[temperature <value:label='temperature id',help='<temperature id>'>...] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxHBCRemoveBCCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx hbc remove bc [displacement <displacement id>...] [temperature <temperature id>...]"; 

  return help;
}

std::vector<std::string> ccxHBCRemoveBCCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxHBCRemoveBCCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  
  std::vector<int> displacement_ids;
  std::vector<int> temperature_ids;

  data.get_values("displacement id", displacement_ids);
  data.get_values("temperature id", temperature_ids);
   
  if (!ccx_iface.hbc_remove_bcs(0, 1, displacement_ids))
  {
    output = "Failed removing Displacement!\n";
    PRINT_ERROR(output.c_str());
  }
  if (!ccx_iface.hbc_remove_bcs(0, 2, temperature_ids))
  {
    output = "Failed removing Temperature!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}