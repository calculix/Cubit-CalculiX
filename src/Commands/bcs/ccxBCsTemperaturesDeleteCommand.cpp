#include "ccxBCsTemperaturesDeleteCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxBCsTemperaturesDeleteCommand::ccxBCsTemperaturesDeleteCommand()
{}

ccxBCsTemperaturesDeleteCommand::~ccxBCsTemperaturesDeleteCommand()
{}

std::vector<std::string> ccxBCsTemperaturesDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete temperature ");

    if (syn_i==1)
    {
      syntax.append("<value:label='temperature id',help='<temperature id>'>... ");
    }
    else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='temperature id',help='<temperature id>'>");
      // to catch all or a quoted input string
    }
    else if (syn_i==3)
    {
      // all except <temperature id>...
      syntax.append("all except ");
      syntax.append("<value:label='temperature id except'>... ");
    }
    else if (syn_i==4)
    {
      // all except <temperature id> to <temperature id 2>
      syntax.append("all except ");
      syntax.append("<value:label='temperature id s1'> ");
      syntax.append("to ");
      syntax.append("<value:label='temperature id s2'> ");
    }
    else if (syn_i==5)
    {
      // <temperature id> to <temperature id 2>
      syntax.append("<value:label='temperature id s1'> ");
      syntax.append("to ");
      syntax.append("<value:label='temperature id s2'> ");
    }

    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxBCsTemperaturesDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete temperature <temperature id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";


  return help;
}

std::vector<std::string> ccxBCsTemperaturesDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxBCsTemperaturesDeleteCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXBCsInterface ccx_bcs_iface;



  std::string output;

  std::vector<int> temperature_ids;
  int temperature_id_s1 = -1;
  int temperature_id_s2 = -1;
  std::vector<int> except_ids;
  std::vector<std::string> temperatures_string;

  std::string temperature_string = " ";

  data.get_value("temperature id s1", temperature_id_s1);
  data.get_value("temperature id s2", temperature_id_s2);

  // Resolve the selection using the plugin-owned temperature IDs.

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("temperature id except", except_ids))
    {
      temperature_string.append("all except");
      for (size_t i = 0; i < except_ids.size(); i++)
      {
        temperature_string.append(" " + std::to_string(except_ids[i]) + " ");
      }
    }
  }
  else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    temperature_string.append("all except " + std::to_string(temperature_id_s1) + " to " + std::to_string(temperature_id_s2));
  }
  else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    temperature_string.append(std::to_string(temperature_id_s1) + " to " + std::to_string(temperature_id_s2));
  }
  else if(data.get_strings("temperature id", temperatures_string))
  {
    for (size_t i = 0; i < temperatures_string.size(); i++)
    {
      temperature_string.append(temperatures_string[i] + " ");
    }
  }

  if(!data.get_values("temperature id", temperature_ids))
  {
    temperature_ids = ccx_application_iface.parser("bcstemperatures", temperature_string);
  }

  if (temperature_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < temperature_ids.size(); i++)
  {
    if (!ccx_bcs_iface.delete_bcstemperatures(temperature_ids[i]))
    {
      output = "Failed with ID " + std::to_string(temperature_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
      return false;
    }
  }
  return true;
}
