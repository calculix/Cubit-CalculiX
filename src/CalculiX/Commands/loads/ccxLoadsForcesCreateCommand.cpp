#include "ccxLoadsForcesCreateCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

ccxLoadsForcesCreateCommand::ccxLoadsForcesCreateCommand()
{}

ccxLoadsForcesCreateCommand::~ccxLoadsForcesCreateCommand()
{}

std::vector<std::string> ccxLoadsForcesCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx create force ";
  syntax.append("nodeset <value:label='nodeset id',help='<nodeset id>'> ");

  for (int dof = 1; dof <= 6; dof++)
  {
    std::string keyword = "dof_" + std::to_string(dof);
    syntax.append(keyword + " <value:label='" + keyword + "_value',help='<value>'> ");
  }

  syntax.append("[op {mod | new}] ");
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[name <string:type='unquoted',number='1',label='name',help='<name>'>] ");
  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxLoadsForcesCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx create force nodeset <nodeset id> ";
  help[0].append("dof_1 <value> dof_2 <value> dof_3 <value> ");
  help[0].append("dof_4 <value> dof_5 <value> dof_6 <value> ");
  help[0].append("[op {mod | new}] [amplitude <amplitude id>] ");
  help[0].append("[timedelay <value>] [name <name>]");
  return help;
}

std::vector<std::string> ccxLoadsForcesCreateCommand::get_help()
{
  std::vector<std::string> help;
  help.push_back("DOFs 1..3 specify force components; DOFs 4..6 specify moment components.");
  return help;
}

bool ccxLoadsForcesCreateCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXLoadsInterface ccx_loads_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  std::vector<std::string> options;
  std::vector<double> options2(6, 0.0);
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
  for (size_t i = 0; i < options2.size(); i++)
  {
    std::string label = "dof_" + std::to_string(i + 1) + "_value";
    if (!data.get_value(label.c_str(), options2[i]))
    {
      return false;
    }
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
  if (!ccx_loads_iface.create_loadsforces(options, options2))
  {
    PRINT_ERROR("Failed creating force.\n");
    return false;
  }
  return true;
}
