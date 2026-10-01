#include "ccxInitPythonInterfaceCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxInitPythonInterfaceCommand::ccxInitPythonInterfaceCommand()
{}

ccxInitPythonInterfaceCommand::~ccxInitPythonInterfaceCommand()
{}

std::vector<std::string> ccxInitPythonInterfaceCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  syntax_list.push_back("ccx init pythoninterface");

  return syntax_list;
}

std::vector<std::string> ccxInitPythonInterfaceCommand::get_syntax_help()
{
  std::vector<std::string> help;
  return help;
}

std::vector<std::string> ccxInitPythonInterfaceCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxInitPythonInterfaceCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  if (!ccx_iface.init_pythoninterface())
  {
    output = "Failed! Must already be initialized.\n";
    PRINT_ERROR(output.c_str());
  }

  return true;
}
