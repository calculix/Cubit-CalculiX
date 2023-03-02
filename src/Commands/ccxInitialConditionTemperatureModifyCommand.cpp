#include "ccxInitialConditionTemperatureModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxInitialConditionTemperatureModifyCommand::ccxInitialConditionTemperatureModifyCommand()
{}

ccxInitialConditionTemperatureModifyCommand::~ccxInitialConditionTemperatureModifyCommand()
{}

std::vector<std::string> ccxInitialConditionTemperatureModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify initialcondition <value:label='initialcondition id',help='<initialcondition id>'> ");
  syntax.append("temperature ");
  syntax.append("[bc_id <value:label='bc_id',help='<bc_id>'>]");
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxInitialConditionTemperatureModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify initialcondition <initialcondition id> ");
  help[0].append("temperature ");
  help[0].append("[bc_id <bc_id>] " );
  
  return help;
}

std::vector<std::string> ccxInitialConditionTemperatureModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxInitialConditionTemperatureModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int initialcondition_id;
  int bc_id_value;
  std::string bc_id;

  data.get_value("initialcondition id", initialcondition_id);

  if (!data.get_value("bc_id", bc_id_value))
  {
    bc_id = "";
    options_marker.push_back(0);
  }
  else
  {
    bc_id = std::to_string(bc_id_value);
    options_marker.push_back(1);
  }
  options.push_back(bc_id);
   
  if (!ccx_iface.modify_initialcondition(initialcondition_id, 2, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}