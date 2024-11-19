#include "ccxMaterialLibraryDeleteMaterialCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxMaterialLibraryDeleteMaterialCommand::ccxMaterialLibraryDeleteMaterialCommand()
{}

ccxMaterialLibraryDeleteMaterialCommand::~ccxMaterialLibraryDeleteMaterialCommand()
{}

std::vector<std::string> ccxMaterialLibraryDeleteMaterialCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("delete materiallibrary material ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax.append("groupname <string:type='unquoted', number='1', label='groupname', help='<groupname>'> ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxMaterialLibraryDeleteMaterialCommand::get_syntax_help()
{
  std::vector<std::string> help(2);
  help[0] = "ccx ";
  help[0].append("delete materiallibrary material ");
  help[0].append("name <name> ");
  help[0].append("groupname <groupname> ");

  return help;
}

std::vector<std::string> ccxMaterialLibraryDeleteMaterialCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxMaterialLibraryDeleteMaterialCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::string name;
  std::string groupname;

  data.get_string("name", name);
  data.get_string("groupname", groupname);
  
  if (!ccx_iface.delete_materiallibrary_material(name,groupname))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}