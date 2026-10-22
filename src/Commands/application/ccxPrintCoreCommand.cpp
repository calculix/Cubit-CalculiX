#include "ccxPrintCoreCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxPrintCoreCommand::ccxPrintCoreCommand()
{}

ccxPrintCoreCommand::~ccxPrintCoreCommand()
{}

std::vector<std::string> ccxPrintCoreCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  syntax_list.push_back("ccx print core");

  return syntax_list;
}

std::vector<std::string> ccxPrintCoreCommand::get_syntax_help()
{
  std::vector<std::string> help;
  return help;
}

std::vector<std::string> ccxPrintCoreCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxPrintCoreCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;


  std::string output;

  output = "\tCalculiXPlugin Core\n";
  output.append(ccx_application_iface.print_data());

  PRINT_INFO("%s", output.c_str());

  return true;
}
