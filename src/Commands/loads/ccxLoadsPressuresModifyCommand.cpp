#include "ccxLoadsPressuresModifyCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

ccxLoadsPressuresModifyCommand::ccxLoadsPressuresModifyCommand()
{}

ccxLoadsPressuresModifyCommand::~ccxLoadsPressuresModifyCommand()
{}

std::vector<std::string> ccxLoadsPressuresModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx modify pressure <value:label='pressure id',help='<pressure id>'> ";
  syntax.append("[sideset <value:label='sideset id',help='<sideset id>'>] ");

  syntax.append("[magnitude <value:label='magnitude_value',help='<magnitude>'>] ");
  syntax.append("[op {mod | new}] ");
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[name <string:type='unquoted',number='1',label='name',help='<name>'>] ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxLoadsPressuresModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx modify pressure <pressure id> [sideset <sideset id>] ";
  help[0].append("[magnitude <value>] ");
  help[0].append("[op {mod | new}] [amplitude <amplitude id>] ");
  help[0].append("[timedelay <value>] [name <name>]");
  return help;
}

std::vector<std::string> ccxLoadsPressuresModifyCommand::get_help()
{
  std::vector<std::string> help;

  return help;
}

bool ccxLoadsPressuresModifyCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXLoadsInterface ccx_loads_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  std::vector<std::string> options(5);
  std::vector<double> options2(1, 0.0);
  std::vector<int> options_marker(6, 0);
  int pressure_id = -1;
  int sideset_id = -1;
  int amplitude_id = -1;
  double timedelay_value = 0.0;

  if (!data.get_value("pressure id", pressure_id))
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
  if (data.get_value("sideset id", sideset_id))
  {
    if (!ccx_mesh_iface.check_sideset_exists(sideset_id))
    {
      PRINT_ERROR("Sideset %d does not exist.\n", sideset_id);
      return false;
    }
    options[3] = std::to_string(sideset_id);
    options_marker[3] = 1;
  }
  if (data.get_value("magnitude_value", options2[0]))
  {
    options_marker[4] = 1;
  }
  if (data.get_string("name", options[4]))
  {
    options_marker[5] = 1;
  }
  if (!ccx_loads_iface.modify_loadspressures(pressure_id, options, options2, options_marker))
  {
    PRINT_ERROR("Failed modifying pressure %d.\n", pressure_id);
    return false;
  }
  return true;
}
