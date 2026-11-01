#include "ccxLoadsHeatfluxesDeleteCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxLoadsHeatfluxesDeleteCommand::ccxLoadsHeatfluxesDeleteCommand()
{}

ccxLoadsHeatfluxesDeleteCommand::~ccxLoadsHeatfluxesDeleteCommand()
{}

std::vector<std::string> ccxLoadsHeatfluxesDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete heatflux ");

    if (syn_i==1)
    {
      syntax.append("<value:label='heatflux id',help='<heatflux id>'>... ");
    }
    else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='heatflux id',help='<heatflux id>'>");
      // to catch all or a quoted input string
    }
    else if (syn_i==3)
    {
      // all except <heatflux id>...
      syntax.append("all except ");
      syntax.append("<value:label='heatflux id except'>... ");
    }
    else if (syn_i==4)
    {
      // all except <heatflux id> to <heatflux id 2>
      syntax.append("all except ");
      syntax.append("<value:label='heatflux id s1'> ");
      syntax.append("to ");
      syntax.append("<value:label='heatflux id s2'> ");
    }
    else if (syn_i==5)
    {
      // <heatflux id> to <heatflux id 2>
      syntax.append("<value:label='heatflux id s1'> ");
      syntax.append("to ");
      syntax.append("<value:label='heatflux id s2'> ");
    }

    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxLoadsHeatfluxesDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete heatflux <heatflux id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";


  return help;
}

std::vector<std::string> ccxLoadsHeatfluxesDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsHeatfluxesDeleteCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXLoadsInterface ccx_loads_iface;



  std::string output;

  std::vector<int> heatflux_ids;
  int heatflux_id_s1 = -1;
  int heatflux_id_s2 = -1;
  std::vector<int> except_ids;
  std::vector<std::string> heatfluxes_string;

  std::string heatflux_string = " ";

  data.get_value("heatflux id s1", heatflux_id_s1);
  data.get_value("heatflux id s2", heatflux_id_s2);

  // Resolve the selection using the plugin-owned heatflux IDs.

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("heatflux id except", except_ids))
    {
      heatflux_string.append("all except");
      for (size_t i = 0; i < except_ids.size(); i++)
      {
        heatflux_string.append(" " + std::to_string(except_ids[i]) + " ");
      }
    }
  }
  else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    heatflux_string.append("all except " + std::to_string(heatflux_id_s1) + " to " + std::to_string(heatflux_id_s2));
  }
  else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    heatflux_string.append(std::to_string(heatflux_id_s1) + " to " + std::to_string(heatflux_id_s2));
  }
  else if(data.get_strings("heatflux id", heatfluxes_string))
  {
    for (size_t i = 0; i < heatfluxes_string.size(); i++)
    {
      heatflux_string.append(heatfluxes_string[i] + " ");
    }
  }

  if(!data.get_values("heatflux id", heatflux_ids))
  {
    heatflux_ids = ccx_application_iface.parser("loadsheatfluxes", heatflux_string);
  }

  if (heatflux_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < heatflux_ids.size(); i++)
  {
    if (!ccx_loads_iface.delete_loadsheatfluxes(heatflux_ids[i]))
    {
      output = "Failed with ID " + std::to_string(heatflux_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
      return false;
    }
  }
  return true;
}
