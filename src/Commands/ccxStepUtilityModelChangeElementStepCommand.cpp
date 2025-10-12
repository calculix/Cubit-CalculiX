#include "ccxStepUtilityModelChangeElementStepCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepUtilityModelChangeElementStepCommand::ccxStepUtilityModelChangeElementStepCommand()
{}

ccxStepUtilityModelChangeElementStepCommand::~ccxStepUtilityModelChangeElementStepCommand()
{}

std::vector<std::string> ccxStepUtilityModelChangeElementStepCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("utility step <value:label='step id',help='<step id>'> ");
  syntax.append("modelchangeelement ");
      
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxStepUtilityModelChangeElementStepCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx utility step <step id> modelchangeelement";
  

  return help;
}

std::vector<std::string> ccxStepUtilityModelChangeElementStepCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepUtilityModelChangeElementStepCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int step_id;
  
  data.get_value("step id", step_id);

  if (!ccx_iface.check_step_exists(step_id))
  {
    output = "No Step ID " + std::to_string(step_id) + " found.\n";
    PRINT_ERROR(output.c_str());
    return false;
  }
  
  if (!ccx_iface.step_utility_modelchangeelement_step(step_id))
  {
    //PRINT_INFO("%s", output.c_str());
    output = "Failed with Step ID " + std::to_string(step_id) + "!\n";
    PRINT_ERROR(output.c_str());
  } 

  return true;
}