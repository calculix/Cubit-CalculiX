#include "ccxMaterialLibraryModifyMaterialCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxMaterialLibraryModifyMaterialCommand::ccxMaterialLibraryModifyMaterialCommand()
{}

ccxMaterialLibraryModifyMaterialCommand::~ccxMaterialLibraryModifyMaterialCommand()
{}

std::vector<std::string> ccxMaterialLibraryModifyMaterialCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify materiallibrary material ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax.append("new_name <string:type='unquoted', number='1', label='new_name', help='<new_name>'> ");
  syntax.append("groupname <string:type='unquoted', number='1', label='groupname', help='<groupname>'> ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxMaterialLibraryModifyMaterialCommand::get_syntax_help()
{
  std::vector<std::string> help(2);
  help[0] = "ccx ";
  help[0].append("modify materiallibrary material ");
  help[0].append("name <name> ");
  help[0].append("new_name <new_name> ");
  help[0].append("groupname <groupname> ");
  
  return help;
}

std::vector<std::string> ccxMaterialLibraryModifyMaterialCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxMaterialLibraryModifyMaterialCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::string name;
  std::string new_name;
  std::string groupname;

  data.get_string("name", name);
  data.get_string("new_name", new_name);
  data.get_string("groupname", groupname);
  
  if (!ccx_iface.modify_materiallibrary_material(name, new_name, groupname))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}