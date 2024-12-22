#include "ccxMaterialLibraryCreateGroupCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxMaterialLibraryCreateGroupCommand::ccxMaterialLibraryCreateGroupCommand()
{}

ccxMaterialLibraryCreateGroupCommand::~ccxMaterialLibraryCreateGroupCommand()
{}

std::vector<std::string> ccxMaterialLibraryCreateGroupCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create materiallibrary group ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxMaterialLibraryCreateGroupCommand::get_syntax_help()
{
  std::vector<std::string> help(2);
  help[0] = "ccx ";
  help[0].append("create materiallibrary group ");
  help[0].append("name <name> ");
  
  return help;
}

std::vector<std::string> ccxMaterialLibraryCreateGroupCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxMaterialLibraryCreateGroupCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::string name;

  data.get_string("name", name);
  
  if (!ccx_iface.create_materiallibrary_group(name))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}