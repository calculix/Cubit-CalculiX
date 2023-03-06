#include "ccxUseAutoCleanupCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxUseAutoCleanupCommand::ccxUseAutoCleanupCommand()
{}

ccxUseAutoCleanupCommand::~ccxUseAutoCleanupCommand()
{}

std::vector<std::string> ccxUseAutoCleanupCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  syntax_list.push_back("ccx use autocleanup { on | off }");

  return syntax_list;
}

std::vector<std::string> ccxUseAutoCleanupCommand::get_syntax_help()
{
  std::vector<std::string> help;
  return help;
}

std::vector<std::string> ccxUseAutoCleanupCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxUseAutoCleanupCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  if (data.find_keyword("ON")){
    if (!ccx_iface.set_use_ccx_autocleanup(true))
    {
      output = "Failed!\n";
      PRINT_ERROR(output.c_str());
    }
  } else if (data.find_keyword("OFF"))
  {
    if (!ccx_iface.set_use_ccx_autocleanup(false))
    {
      output = "Failed!\n";
      PRINT_ERROR(output.c_str());
    }
  }
  return true;
}