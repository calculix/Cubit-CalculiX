#include "ccxResetCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

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
  CalculiXCoreInterface ccx_iface;

  std::string output;

  if (!ccx_iface.core_reset())
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }

  return true;
}
