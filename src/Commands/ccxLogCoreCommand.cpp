#include "ccxLogCoreCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLogCoreCommand::ccxLogCoreCommand()
{}

ccxLogCoreCommand::~ccxLogCoreCommand()
{}

std::vector<std::string> ccxLogCoreCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  syntax_list.push_back("ccx log core");

  return syntax_list;
}

std::vector<std::string> ccxLogCoreCommand::get_syntax_help()
{
  std::vector<std::string> help;
  return help;
}

std::vector<std::string> ccxLogCoreCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLogCoreCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  
  ccx_iface.log_str(ccx_iface.print_data());

  output = "\tCalculiXPlugin Core\n";
  output.append("\tCore data printed to CalculiXCore_logfile\n");

  PRINT_INFO("%s", output.c_str());

  return true;
}
