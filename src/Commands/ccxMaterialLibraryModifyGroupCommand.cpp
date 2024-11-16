#include "ccxMaterialLibraryModifyGroupCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxMaterialLibraryModifyGroupCommand::ccxMaterialLibraryModifyGroupCommand()
{}

ccxMaterialLibraryModifyGroupCommand::~ccxMaterialLibraryModifyGroupCommand()
{}

std::vector<std::string> ccxMaterialLibraryModifyGroupCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify materiallibrary group ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax.append("new_name <string:type='unquoted', number='1', label='new_name', help='<new_name>'> ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxMaterialLibraryModifyGroupCommand::get_syntax_help()
{
  std::vector<std::string> help(2);
  help[0] = "ccx ";
  help[0].append("modify materiallibrary group ");
  help[0].append("name <name> ");
  help[0].append("new_name <new_name> ");
  
  return help;
}

std::vector<std::string> ccxMaterialLibraryModifyGroupCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxMaterialLibraryModifyGroupCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::string name;
  std::string new_name;

  data.get_string("name", name);
  data.get_string("new_name", new_name);
  
  if (!ccx_iface.modify_materiallibrary_group(name, new_name))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}