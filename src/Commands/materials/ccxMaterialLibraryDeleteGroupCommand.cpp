#include "ccxMaterialLibraryDeleteGroupCommand.hpp"
#include <string>
#include <vector>
#include "Core/materials/CalculiXMaterialsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxMaterialLibraryDeleteGroupCommand::ccxMaterialLibraryDeleteGroupCommand()
{}

ccxMaterialLibraryDeleteGroupCommand::~ccxMaterialLibraryDeleteGroupCommand()
{}

std::vector<std::string> ccxMaterialLibraryDeleteGroupCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("delete materiallibrary group ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxMaterialLibraryDeleteGroupCommand::get_syntax_help()
{
  std::vector<std::string> help(2);
  help[0] = "ccx ";
  help[0].append("delete materiallibrary group ");
  help[0].append("name <name> ");
  
  return help;
}

std::vector<std::string> ccxMaterialLibraryDeleteGroupCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxMaterialLibraryDeleteGroupCommand::execute(CubitCommandData &data)
{
  CalculiXMaterialsInterface ccx_materials_iface;


  std::string output;
  std::string name;

  data.get_string("name", name);
  
  if (!ccx_materials_iface.delete_materiallibrary_group(name))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}