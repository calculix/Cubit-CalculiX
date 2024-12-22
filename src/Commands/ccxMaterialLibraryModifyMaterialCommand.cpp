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
  syntax.append("groupname <string:type='unquoted', number='1', label='groupname', help='<groupname>'> ");
  syntax.append("new_name <string:type='unquoted', number='1', label='new_name', help='<new_name>'> ");
  syntax_list.push_back(syntax);

  syntax = "ccx ";
  syntax.append("modify materiallibrary material ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax.append("groupname <string:type='unquoted', number='1', label='groupname', help='<groupname>'> ");
  syntax.append("description <string:type='unquoted', number='1', label='description', help='<description>'> ");
  syntax_list.push_back(syntax);

  syntax = "ccx ";
  syntax.append("modify materiallibrary material ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax.append("groupname <string:type='unquoted', number='1', label='groupname', help='<groupname>'> ");
  syntax.append("scalar_properties <string:type='unquoted', number='1', label='scalar_properties', help='<scalar_properties>'> ");
  syntax.append("<value:label='value',help='<value>'>" );
  syntax_list.push_back(syntax);

  syntax = "ccx ";
  syntax.append("modify materiallibrary material ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax.append("groupname <string:type='unquoted', number='1', label='groupname', help='<groupname>'> ");
  syntax.append("matrix_property <string:type='unquoted', number='1', label='matrix_property', help='<matrix_property>'> ");
  syntax.append("<value:label='values',help='<values>'>... " );
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxMaterialLibraryModifyMaterialCommand::get_syntax_help()
{
  std::vector<std::string> help(4);
  help[0] = "ccx ";
  help[0].append("modify materiallibrary material ");
  help[0].append("name <name> ");
  help[0].append("groupname <groupname> ");
  help[0].append("new_name <new_name> ");
  
  help[1] = "ccx ";
  help[1].append("modify materiallibrary material ");
  help[1].append("name <name> ");
  help[1].append("groupname <groupname> ");
  help[1].append("description <description> ");

  help[2] = "ccx ";
  help[2].append("modify materiallibrary material ");
  help[2].append("name <name> ");
  help[2].append("groupname <groupname> ");
  help[2].append("scalar_properties <scalar_properties> ");
  help[2].append("<value> ");

  help[3] = "ccx ";
  help[3].append("modify materiallibrary material ");
  help[3].append("name <name> ");
  help[3].append("groupname <groupname> ");
  help[3].append("matrix_property <matrix_property> ");
  help[3].append("<values>... ");

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
  std::string groupname;
  std::string new_name;
  std::string description;
  std::string scalar_properties;
  std::string matrix_property;
  std::vector<double> value_data;
  double value;

  data.get_string("name", name);
  data.get_string("groupname", groupname);
  
  if (data.get_string("new_name", new_name))
  {
    if (!ccx_iface.modify_materiallibrary_material(name, groupname , new_name, 0, value_data))
    {
      output = "Failed!\n";
      PRINT_ERROR(output.c_str());
    }
  }
   
  if (data.get_string("description", description))
  {
    if (!ccx_iface.modify_materiallibrary_material(name, groupname , description, 1, value_data))
    {
      output = "Failed!\n";
      PRINT_ERROR(output.c_str());
    }
  }

  if (data.get_string("scalar_properties", scalar_properties) && data.get_value("value", value))
  {
    value_data.push_back(value);

    if (!ccx_iface.modify_materiallibrary_material(name, groupname , scalar_properties, 2, value_data))
    {
      output = "Failed!\n";
      PRINT_ERROR(output.c_str());
    }
  }

  if (data.get_string("matrix_property", matrix_property) && data.get_values("values", value_data))
  {
    if (!ccx_iface.modify_materiallibrary_material(name, groupname , matrix_property, 3, value_data))
    {
      output = "Failed!\n";
      PRINT_ERROR(output.c_str());
    }
  }

  return true;
}