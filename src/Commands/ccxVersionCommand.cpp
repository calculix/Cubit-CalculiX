#include "ccxVersionCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxVersionCommand::ccxVersionCommand()
{}

ccxVersionCommand::~ccxVersionCommand()
{}

std::vector<std::string> ccxVersionCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  syntax_list.push_back("ccx version");

  return syntax_list;
}

std::vector<std::string> ccxVersionCommand::get_syntax_help()
{
  std::vector<std::string> help;
  return help;
}

std::vector<std::string> ccxVersionCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxVersionCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;
  std::string cubit_version = CubitInterface::get_version();
  std::string build_number = CubitInterface::get_build_number();
  std::string revision_date = CubitInterface::get_revision_date();
  std::string geom_version = CubitInterface::get_acis_version();
  std::string exodus_version = CubitInterface::get_exodus_version();
  std::string graphics_version = CubitInterface::get_graphics_version();

  std::string output;

  output = "\tCubit-CalculiX Version " + ccx_iface.get_version() + "\n";
  PRINT_INFO("%s", output.c_str());

  output = "\tCubit Version " + cubit_version + " Build " + build_number + "\n";
  PRINT_INFO("%s", output.c_str());

  output = "\t" + graphics_version + "\n";
  PRINT_INFO("%s", output.c_str());

  output = "\t" + geom_version + "\n";
  PRINT_INFO("%s", output.c_str());

  output = "\tExodus Version " + exodus_version + "\n";
  PRINT_INFO("%s", output.c_str());

  output = "\tRevised " + revision_date + "\n\n",
  PRINT_INFO("%s", output.c_str());

  return true;
}
