#include "ccxMaterialLibraryImportMaterialCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxMaterialLibraryImportMaterialCommand::ccxMaterialLibraryImportMaterialCommand()
{}

ccxMaterialLibraryImportMaterialCommand::~ccxMaterialLibraryImportMaterialCommand()
{}

std::vector<std::string> ccxMaterialLibraryImportMaterialCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("import materiallibrary material ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax.append("groupname <string:type='unquoted', number='1', label='groupname', help='<groupname>'> ");
  syntax.append("cubit_name <string:type='unquoted', number='1', label='cubit_name', help='<cubit_name>'> ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxMaterialLibraryImportMaterialCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("import materiallibrary material ");
  help[0].append("name <name> ");
  help[0].append("groupname <groupname> ");
  help[0].append("cubit_name <cubit_name> ");
  
  return help;
}

std::vector<std::string> ccxMaterialLibraryImportMaterialCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxMaterialLibraryImportMaterialCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::string name;
  std::string groupname;
  std::string cubit_name;
  
  data.get_string("name", name);
  data.get_string("groupname", groupname);
  data.get_string("cubit_name", cubit_name);
  
  if (!ccx_iface.import_materiallibrary_material(name, groupname, cubit_name))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }   

  return true;
}