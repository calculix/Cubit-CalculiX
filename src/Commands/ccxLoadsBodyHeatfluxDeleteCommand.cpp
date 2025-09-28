#include "ccxLoadsBodyHeatfluxDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsBodyHeatfluxDeleteCommand::ccxLoadsBodyHeatfluxDeleteCommand()
{}

ccxLoadsBodyHeatfluxDeleteCommand::~ccxLoadsBodyHeatfluxDeleteCommand()
{}

std::vector<std::string> ccxLoadsBodyHeatfluxDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete bodyheatflux ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='bodyheatflux id',help='<bodyheatflux id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='bodyheatflux id',help='<bodyheatflux id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <bodyheatflux id>...
      syntax.append("all except ");
      syntax.append("<value:label='bodyheatflux id except'>... ");
    }else if (syn_i==4)
    {
      // all except <bodyheatflux id> to <bodyheatflux id 2>
      syntax.append("all except ");
      syntax.append("<value:label='bodyheatflux id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='bodyheatflux id s2'>");
    }else if (syn_i==5)
    {
      // <bodyheatflux id> to <bodyheatflux id 2>
      syntax.append("<value:label='bodyheatflux id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='bodyheatflux id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxLoadsBodyHeatfluxDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete bodyheatflux <bodyheatflux id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxLoadsBodyHeatfluxDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsBodyHeatfluxDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> bodyheatflux_ids;
  int bodyheatflux_id_s1;
  int bodyheatflux_id_s2;
  std::vector<std::string> bodyheatfluxs_string;

  std::string bodyheatflux_string = " ";
  
  data.get_value("bodyheatflux id s1", bodyheatflux_id_s1);
  data.get_value("bodyheatflux id s2", bodyheatflux_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("bodyheatflux id except", bodyheatflux_ids))
    {
      bodyheatflux_string.append("all except");
      for (size_t i = 0; i < bodyheatflux_ids.size(); i++)
      {
        bodyheatflux_string.append(" " + std::to_string(bodyheatflux_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    bodyheatflux_string.append("all except " + std::to_string(bodyheatflux_id_s1) + " to " + std::to_string(bodyheatflux_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    bodyheatflux_string.append(std::to_string(bodyheatflux_id_s1) + " to " + std::to_string(bodyheatflux_id_s2));
  }else if(data.get_strings("bodyheatflux id", bodyheatfluxs_string))
  {
    for (size_t i = 0; i < bodyheatfluxs_string.size(); i++)
    {
      bodyheatflux_string.append(bodyheatfluxs_string[i]);
    }
  }
  
  if(!data.get_values("bodyheatflux id", bodyheatflux_ids))
  {   
    bodyheatflux_ids = ccx_iface.parser("loadsbodyheatflux", bodyheatflux_string);
  }

  if (bodyheatflux_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < bodyheatflux_ids.size(); i++)
  {    
    if (!ccx_iface.delete_loadsbodyheatflux(bodyheatflux_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(bodyheatflux_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}