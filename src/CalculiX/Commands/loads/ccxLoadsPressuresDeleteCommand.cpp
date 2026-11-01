#include "ccxLoadsPressuresDeleteCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxLoadsPressuresDeleteCommand::ccxLoadsPressuresDeleteCommand()
{}

ccxLoadsPressuresDeleteCommand::~ccxLoadsPressuresDeleteCommand()
{}

std::vector<std::string> ccxLoadsPressuresDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete pressure ");

    if (syn_i==1)
    {
      syntax.append("<value:label='pressure id',help='<pressure id>'>... ");
    }
    else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='pressure id',help='<pressure id>'>");
      // to catch all or a quoted input string
    }
    else if (syn_i==3)
    {
      // all except <pressure id>...
      syntax.append("all except ");
      syntax.append("<value:label='pressure id except'>... ");
    }
    else if (syn_i==4)
    {
      // all except <pressure id> to <pressure id 2>
      syntax.append("all except ");
      syntax.append("<value:label='pressure id s1'> ");
      syntax.append("to ");
      syntax.append("<value:label='pressure id s2'> ");
    }
    else if (syn_i==5)
    {
      // <pressure id> to <pressure id 2>
      syntax.append("<value:label='pressure id s1'> ");
      syntax.append("to ");
      syntax.append("<value:label='pressure id s2'> ");
    }

    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxLoadsPressuresDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete pressure <pressure id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";


  return help;
}

std::vector<std::string> ccxLoadsPressuresDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsPressuresDeleteCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXLoadsInterface ccx_loads_iface;



  std::string output;

  std::vector<int> pressure_ids;
  int pressure_id_s1 = -1;
  int pressure_id_s2 = -1;
  std::vector<int> except_ids;
  std::vector<std::string> pressures_string;

  std::string pressure_string = " ";

  data.get_value("pressure id s1", pressure_id_s1);
  data.get_value("pressure id s2", pressure_id_s2);

  // Resolve the selection using the plugin-owned pressure IDs.

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("pressure id except", except_ids))
    {
      pressure_string.append("all except");
      for (size_t i = 0; i < except_ids.size(); i++)
      {
        pressure_string.append(" " + std::to_string(except_ids[i]) + " ");
      }
    }
  }
  else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    pressure_string.append("all except " + std::to_string(pressure_id_s1) + " to " + std::to_string(pressure_id_s2));
  }
  else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    pressure_string.append(std::to_string(pressure_id_s1) + " to " + std::to_string(pressure_id_s2));
  }
  else if(data.get_strings("pressure id", pressures_string))
  {
    for (size_t i = 0; i < pressures_string.size(); i++)
    {
      pressure_string.append(pressures_string[i] + " ");
    }
  }

  if(!data.get_values("pressure id", pressure_ids))
  {
    pressure_ids = ccx_application_iface.parser("loadspressures", pressure_string);
  }

  if (pressure_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < pressure_ids.size(); i++)
  {
    if (!ccx_loads_iface.delete_loadspressures(pressure_ids[i]))
    {
      output = "Failed with ID " + std::to_string(pressure_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
      return false;
    }
  }
  return true;
}
