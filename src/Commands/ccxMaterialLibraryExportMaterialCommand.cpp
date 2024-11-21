#include "ccxMaterialLibraryExportMaterialCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxMaterialLibraryExportMaterialCommand::ccxMaterialLibraryExportMaterialCommand()
{}

ccxMaterialLibraryExportMaterialCommand::~ccxMaterialLibraryExportMaterialCommand()
{}

std::vector<std::string> ccxMaterialLibraryExportMaterialCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("export materiallibrary material ");
  syntax.append("cubit_name <string:type='unquoted', number='1', label='cubit_name', help='<cubit_name>'> ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax.append("groupname <string:type='unquoted', number='1', label='groupname', help='<groupname>'> ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxMaterialLibraryExportMaterialCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("export materiallibrary material ");
  help[0].append("cubit_name <cubit_name> ");
  help[0].append("name <name> ");
  help[0].append("groupname <groupname> ");
  
  return help;
}

std::vector<std::string> ccxMaterialLibraryExportMaterialCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxMaterialLibraryExportMaterialCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::string name;
  std::string groupname;
  std::string cubit_name;
  
  data.get_string("name", name);
  data.get_string("groupname", groupname);
  data.get_string("cubit_name", cubit_name);
  
  if (!ccx_iface.export_materiallibrary_material(name, groupname, cubit_name))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }

  return true;
}