#include "ccxResetCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxResetCommand::ccxResetCommand()
{}

ccxResetCommand::~ccxResetCommand()
{}

std::vector<std::string> ccxResetCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  syntax_list.push_back("ccx reset");

  return syntax_list;
}

std::vector<std::string> ccxResetCommand::get_syntax_help()
{
  std::vector<std::string> help;
  return help;
}

std::vector<std::string> ccxResetCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResetCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;


  std::string output;

  if (!ccx_application_iface.core_reset())
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }

  return true;
}
