#include "ccxBCsDisplacementsCreateCommand.hpp"
#include <string>
#include <vector>
#include <cctype>
#include <cmath>
#include <stdexcept>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

ccxBCsDisplacementsCreateCommand::ccxBCsDisplacementsCreateCommand()
{}

ccxBCsDisplacementsCreateCommand::~ccxBCsDisplacementsCreateCommand()
{}

std::vector<std::string> ccxBCsDisplacementsCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx create displacement ";
  syntax.append("nodeset <value:label='nodeset id',help='<nodeset id>'> ");

  for (int dof = 1; dof <= 6; dof++)
  {
    std::string keyword = "dof_" + std::to_string(dof);
    syntax.append(
      "[{" +
      keyword + " <value:label='" + keyword +
      "_magnitude',help='<number>'> | " +
      keyword + " <string:type='unquoted',number='1',label='" +
      keyword + "',help='<free|fixed>'>}] ");
  }

  syntax.append("[op {mod | new}] ");
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[name <string:type='unquoted',number='1',label='name',help='<name>'>] ");

  syntax_list.push_back(syntax);
  return syntax_list;
}

std::vector<std::string> ccxBCsDisplacementsCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx create displacement nodeset <nodeset id> ";
  for (int dof = 1; dof <= 6; dof++)
  {
    help[0].append("[dof_" + std::to_string(dof) + " <number|free|fixed>] ");
  }
  help[0].append("[op {mod | new}] [amplitude <amplitude id>] [timedelay <value>] [name <name>]");
  return help;
}

std::vector<std::string> ccxBCsDisplacementsCreateCommand::get_help()
{
  std::vector<std::string> help;
  help.push_back("A numeric DOF value is prescribed; free omits the DOF; fixed uses CalculiX FIXED.");
  help.push_back("Omitted DOFs default to free.");
  return help;
}

bool ccxBCsDisplacementsCreateCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXBCsInterface ccx_bcs_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  std::vector<std::string> options(5);
  std::vector<double> options2(6, 0.0);
  std::vector<int> options3(6, 0);
  int nodeset_id = -1;
  int amplitude_id = -1;
  double timedelay_value = 0.0;

  if (!data.get_value("nodeset id", nodeset_id))
  {
    return false;
  }
  if (!ccx_mesh_iface.check_nodeset_exists(nodeset_id))
  {
    PRINT_ERROR("Nodeset %d does not exist.\n", nodeset_id);
    return false;
  }

  options[0] = "0";
  if (data.find_keyword("OP"))
  {
    if (data.find_keyword("NEW"))
    {
      options[0] = "1";
    }
  }

  data.get_value("amplitude id", amplitude_id);
  options[1] = std::to_string(amplitude_id);

  if (data.get_value("timedelay", timedelay_value))
  {
    options[2] = ccx_application_iface.to_string_scientific(timedelay_value);
  }

  options[3] = std::to_string(nodeset_id);
  data.get_string("name", options[4]);

  for (int dof = 1; dof <= 6; dof++)
  {
    std::string keyword = "dof_" + std::to_string(dof);
    std::string magnitude_label = keyword + "_magnitude";
    double magnitude = 0.0;

    if (data.get_value(magnitude_label.c_str(), magnitude))
    {
      if (!std::isfinite(magnitude))
      {
        PRINT_ERROR("Invalid magnitude for DOF %d.\n", dof);
        return false;
      }

      options2[dof - 1] = magnitude;
      options3[dof - 1] = 1;
      continue;
    }

    std::string argument;

    if (!data.get_string(keyword.c_str(), argument))
    {
      continue;
    }

    for (char &character : argument)
    {
      character = char(std::tolower(static_cast<unsigned char>(character)));
    }

    if (argument == "free")
    {
      options3[dof - 1] = 0;
    }
    else if (argument == "fixed")
    {
      options3[dof - 1] = 2;
    }
    else
    {
      try
      {
        size_t parsed = 0;
        double value = std::stod(argument, &parsed);
        if (parsed != argument.size() || !std::isfinite(value))
        {
          PRINT_ERROR("Invalid value for DOF %d.\n", dof);
          return false;
        }
        options2[dof - 1] = value;
        options3[dof - 1] = 1;
      }
      catch (const std::exception &)
      {
        PRINT_ERROR("Invalid value for DOF %d.\n", dof);
        return false;
      }
    }
  }

  if (!ccx_bcs_iface.create_bcsdisplacements(options, options2, options3))
  {
    PRINT_ERROR("Failed creating displacement.\n");
    return false;
  }
  return true;
}
