#include "ccxInitialConditionCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxInitialConditionCreateCommand::ccxInitialConditionCreateCommand()
{}

ccxInitialConditionCreateCommand::~ccxInitialConditionCreateCommand()
{}

std::vector<std::string> ccxInitialConditionCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create initialcondition ");
  syntax.append("[{displacement|temperature}]" );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxInitialConditionCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create initialcondition [{displacement|temperature}]"; 

  return help;
}

std::vector<std::string> ccxInitialConditionCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxInitialConditionCreateCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  std::string name;

  if (data.find_keyword("DISPLACEMENT"))
  {
    options.push_back("1");
  } else if (data.find_keyword("TEMPERATURE"))
  {
    options.push_back("2");
  }
  
  if (!ccx_iface.create_initialcondition(options))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}