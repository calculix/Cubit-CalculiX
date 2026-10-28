#include "ccxBCsTemperaturesCreateCommand.hpp"
#include <string>
#include <vector>
#include <cmath>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

ccxBCsTemperaturesCreateCommand::ccxBCsTemperaturesCreateCommand()
{}

ccxBCsTemperaturesCreateCommand::~ccxBCsTemperaturesCreateCommand()
{}

std::vector<std::string> ccxBCsTemperaturesCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx create temperature ";
  syntax.append("nodeset <value:label='nodeset id',help='<nodeset id>'> ");

  syntax.append("magnitude <value:label='magnitude_value',help='<magnitude>'> ");
  syntax.append("[op {mod | new}] ");
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[keyword_type {boundary | temp}] ");
  syntax.append("[name <string:type='unquoted',number='1',label='name',help='<name>'>] ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxBCsTemperaturesCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx create temperature nodeset <nodeset id> ";
  help[0].append("magnitude <value> ");
  help[0].append("[op {mod | new}] [amplitude <amplitude id>] ");
  help[0].append("[timedelay <value>] [keyword_type {boundary | temp}] [name <name>]");
  return help;
}

std::vector<std::string> ccxBCsTemperaturesCreateCommand::get_help()
{
  std::vector<std::string> help;

  return help;
}

bool ccxBCsTemperaturesCreateCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXBCsInterface ccx_bcs_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  std::vector<std::string> options;
  std::vector<double> options2(1, 0.0);
  int nodeset_id = -1;
  int amplitude_id = -1;
  int op_mode = 0;
  double timedelay_value = 0.0;
  std::string timedelay;
  std::string name;

  if (!data.get_value("nodeset id", nodeset_id))
  {
    return false;
  }
  if (!ccx_mesh_iface.check_nodeset_exists(nodeset_id))
  {
    PRINT_ERROR("Nodeset %d does not exist.\n", nodeset_id);
    return false;
  }
  if (!data.get_value("magnitude_value", options2[0]))
  {
    return false;
  }
  if (data.find_keyword("OP"))
  {
    if (data.find_keyword("NEW"))
    {
      op_mode = 1;
    }
  }
  data.get_value("amplitude id", amplitude_id);
  if (data.get_value("timedelay", timedelay_value))
  {
    timedelay = ccx_application_iface.to_string_scientific(timedelay_value);
  }
  data.get_string("name", name);

  options.push_back(std::to_string(op_mode));
  options.push_back(std::to_string(amplitude_id));
  options.push_back(timedelay);
  options.push_back(std::to_string(nodeset_id));
  options.push_back(name);
  int keyword_type = 0;
  if (data.find_keyword("KEYWORD_TYPE"))
  {
    if (data.find_keyword("TEMP"))
    {
      keyword_type = 1;
    }
  }
  options.push_back(std::to_string(keyword_type));
  if (!std::isfinite(options2[0]))
  {
    PRINT_ERROR("Invalid temperature magnitude.\n");
    return false;
  }
  if (!ccx_bcs_iface.create_bcstemperatures(options, options2))
  {
    PRINT_ERROR("Failed creating temperature.\n");
    return false;
  }
  return true;
}
