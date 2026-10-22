#include "ccxLoadsForcesModifyCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

ccxLoadsForcesModifyCommand::ccxLoadsForcesModifyCommand()
{}

ccxLoadsForcesModifyCommand::~ccxLoadsForcesModifyCommand()
{}

std::vector<std::string> ccxLoadsForcesModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx modify force <value:label='force id',help='<force id>'> ";
  syntax.append("[nodeset <value:label='nodeset id',help='<nodeset id>'>] ");

  for (int dof = 1; dof <= 6; dof++)
  {
    std::string keyword = "dof_" + std::to_string(dof);
    syntax.append("[" + keyword + " <value:label='" + keyword + "_value',help='<value>'>] ");
  }

  syntax.append("[op {mod | new}] ");
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[name <string:type='unquoted',number='1',label='name',help='<name>'>] ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxLoadsForcesModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx modify force <force id> [nodeset <nodeset id>] ";
  help[0].append("[dof_1 <value>] [dof_2 <value>] [dof_3 <value>] ");
  help[0].append("[dof_4 <value>] [dof_5 <value>] [dof_6 <value>] ");
  help[0].append("[op {mod | new}] [amplitude <amplitude id>] ");
  help[0].append("[timedelay <value>] [name <name>]");
  return help;
}

std::vector<std::string> ccxLoadsForcesModifyCommand::get_help()
{
  std::vector<std::string> help;
  help.push_back("DOFs 1..3 specify force components; DOFs 4..6 specify moment components.");
  return help;
}

bool ccxLoadsForcesModifyCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXLoadsInterface ccx_loads_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  std::vector<std::string> options(5);
  std::vector<double> options2(6, 0.0);
  std::vector<int> options_marker(11, 0);
  int force_id = -1;
  int nodeset_id = -1;
  int amplitude_id = -1;
  double timedelay_value = 0.0;

  if (!data.get_value("force id", force_id))
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
  for (size_t i = 0; i < options2.size(); i++)
  {
    std::string label = "dof_" + std::to_string(i + 1) + "_value";
    if (data.get_value(label.c_str(), options2[i]))
    {
      options_marker[i + 4] = 1;
    }
  }
  if (data.get_string("name", options[4]))
  {
    options_marker[10] = 1;
  }
  if (!ccx_loads_iface.modify_loadsforces(force_id, options, options2, options_marker))
  {
    PRINT_ERROR("Failed modifying force %d.\n", force_id);
    return false;
  }
  return true;
}
