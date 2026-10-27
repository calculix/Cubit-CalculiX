#include "ccxLoadsHeatfluxesCreateCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

ccxLoadsHeatfluxesCreateCommand::ccxLoadsHeatfluxesCreateCommand()
{}

ccxLoadsHeatfluxesCreateCommand::~ccxLoadsHeatfluxesCreateCommand()
{}

std::vector<std::string> ccxLoadsHeatfluxesCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx create heatflux ";
  syntax.append("sideset <value:label='sideset id',help='<sideset id>'> ");

  syntax.append("magnitude <value:label='magnitude_value',help='<magnitude>'> ");
  syntax.append("[op {mod | new}] ");
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[name <string:type='unquoted',number='1',label='name',help='<name>'>] ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxLoadsHeatfluxesCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx create heatflux sideset <sideset id> ";
  help[0].append("magnitude <value> ");
  help[0].append("[op {mod | new}] [amplitude <amplitude id>] ");
  help[0].append("[timedelay <value>] [name <name>]");
  return help;
}

std::vector<std::string> ccxLoadsHeatfluxesCreateCommand::get_help()
{
  std::vector<std::string> help;

  return help;
}

bool ccxLoadsHeatfluxesCreateCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXLoadsInterface ccx_loads_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  std::vector<std::string> options;
  std::vector<double> options2(1, 0.0);
  int sideset_id = -1;
  int amplitude_id = -1;
  int op_mode = 0;
  double timedelay_value = 0.0;
  std::string timedelay;
  std::string name;

  if (!data.get_value("sideset id", sideset_id))
  {
    return false;
  }
  if (!ccx_mesh_iface.check_sideset_exists(sideset_id))
  {
    PRINT_ERROR("Sideset %d does not exist.\n", sideset_id);
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
  options.push_back(std::to_string(sideset_id));
  options.push_back(name);
  if (!ccx_loads_iface.create_loadsheatfluxes(options, options2))
  {
    PRINT_ERROR("Failed creating heatflux.\n");
    return false;
  }
  return true;
}
