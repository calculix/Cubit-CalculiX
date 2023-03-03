#include "ccxStepAddBCCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepAddBCCommand::ccxStepAddBCCommand()
{}

ccxStepAddBCCommand::~ccxStepAddBCCommand()
{}

std::vector<std::string> ccxStepAddBCCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("step <value:label='step id',help='<step id>'> ");
  syntax.append("add bc ");
  syntax.append("[displacement <value:label='displacement id',help='<displacement id>'>...] " );
  syntax.append("[temperature <value:label='temperature id',help='<temperature id>'>...] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxStepAddBCCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx step <step id> add bc [displacement <displacement id>...] [temperature <temperature id>...]"; 

  return help;
}

std::vector<std::string> ccxStepAddBCCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepAddBCCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  
  int step_id;
  std::vector<int> displacement_ids;
  std::vector<int> temperature_ids;

  data.get_value("step id", step_id);

  data.get_values("displacement id", displacement_ids);
  data.get_values("temperature id", temperature_ids);
   
  if (!ccx_iface.step_add_bcs(step_id, 1, displacement_ids))
  {
    output = "Failed adding Displacement!\n";
    PRINT_ERROR(output.c_str());
  }
  if (!ccx_iface.step_add_bcs(step_id, 2, temperature_ids))
  {
    output = "Failed adding Temperature!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}