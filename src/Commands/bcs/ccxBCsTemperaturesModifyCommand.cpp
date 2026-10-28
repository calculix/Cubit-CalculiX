#include "ccxBCsTemperaturesModifyCommand.hpp"
#include <string>
#include <vector>
#include <cmath>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

ccxBCsTemperaturesModifyCommand::ccxBCsTemperaturesModifyCommand()
{}

ccxBCsTemperaturesModifyCommand::~ccxBCsTemperaturesModifyCommand()
{}

std::vector<std::string> ccxBCsTemperaturesModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx modify temperature <value:label='temperature id',help='<temperature id>'> ";
  syntax.append("[nodeset <value:label='nodeset id',help='<nodeset id>'>] ");

  syntax.append("[magnitude <value:label='magnitude_value',help='<magnitude>'>] ");
  syntax.append("[op {mod | new}] ");
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[keyword_type {boundary | temp}] ");
  syntax.append("[name <string:type='unquoted',number='1',label='name',help='<name>'>] ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxBCsTemperaturesModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx modify temperature <temperature id> [nodeset <nodeset id>] ";
  help[0].append("[magnitude <value>] ");
  help[0].append("[op {mod | new}] [amplitude <amplitude id>] ");
  help[0].append("[timedelay <value>] [keyword_type {boundary | temp}] [name <name>]");
  return help;
}

std::vector<std::string> ccxBCsTemperaturesModifyCommand::get_help()
{
  std::vector<std::string> help;

  return help;
}

bool ccxBCsTemperaturesModifyCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXBCsInterface ccx_bcs_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  std::vector<std::string> options(6);
  std::vector<double> options2(1, 0.0);
  std::vector<int> options_marker(7, 0);
  int temperature_id = -1;
  int nodeset_id = -1;
  int amplitude_id = -1;
  double timedelay_value = 0.0;

  if (!data.get_value("temperature id", temperature_id))
  {
    return false;
  }
  if (data.find_keyword("OP"))
  {
    options[0] = "0";
    if (data.find_keyword("NEW"))
    {
      options[0] = "1";
    }
    options_marker[0] = 1;
  }
  if (data.get_value("amplitude id", amplitude_id))
  {
    options[1] = std::to_string(amplitude_id);
    options_marker[1] = 1;
  }
  if (data.get_value("timedelay", timedelay_value))
  {
    options[2] = ccx_application_iface.to_string_scientific(timedelay_value);
    options_marker[2] = 1;
  }
  if (data.get_value("nodeset id", nodeset_id))
  {
    if (!ccx_mesh_iface.check_nodeset_exists(nodeset_id))
    {
      PRINT_ERROR("Nodeset %d does not exist.\n", nodeset_id);
      return false;
    }
    options[3] = std::to_string(nodeset_id);
    options_marker[3] = 1;
  }
  if (data.get_value("magnitude_value", options2[0]))
  {
    if (!std::isfinite(options2[0]))
    {
      PRINT_ERROR("Invalid temperature magnitude.\n");
      return false;
    }
    options_marker[4] = 1;
  }
  if (data.get_string("name", options[4]))
  {
    options_marker[5] = 1;
  }
  if (data.find_keyword("KEYWORD_TYPE"))
  {
    options[5] = "0";
    if (data.find_keyword("TEMP"))
    {
      options[5] = "1";
    }
    options_marker[6] = 1;
  }
  if (!ccx_bcs_iface.modify_bcstemperatures(temperature_id, options, options2, options_marker))
  {
    PRINT_ERROR("Failed modifying temperature %d.\n", temperature_id);
    return false;
  }
  return true;
}