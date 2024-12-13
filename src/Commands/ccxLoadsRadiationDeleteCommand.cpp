#include "ccxLoadsRadiationDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsRadiationDeleteCommand::ccxLoadsRadiationDeleteCommand()
{}

ccxLoadsRadiationDeleteCommand::~ccxLoadsRadiationDeleteCommand()
{}

std::vector<std::string> ccxLoadsRadiationDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete radiation ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='radiation id',help='<radiation id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='radiation id',help='<radiation id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <radiation id>...
      syntax.append("all except ");
      syntax.append("<value:label='radiation id except'>... ");
    }else if (syn_i==4)
    {
      // all except <radiation id> to <radiation id 2>
      syntax.append("all except ");
      syntax.append("<value:label='radiation id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='radiation id s2'>");
    }else if (syn_i==5)
    {
      // <radiation id> to <radiation id 2>
      syntax.append("<value:label='radiation id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='radiation id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxLoadsRadiationDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete radiation <radiation id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxLoadsRadiationDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsRadiationDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> radiation_ids;
  int radiation_id_s1;
  int radiation_id_s2;
  std::vector<std::string> radiations_string;

  std::string radiation_string = " ";
  
  data.get_value("radiation id s1", radiation_id_s1);
  data.get_value("radiation id s2", radiation_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("radiation id except", radiation_ids))
    {
      radiation_string.append("all except");
      for (size_t i = 0; i < radiation_ids.size(); i++)
      {
        radiation_string.append(" " + std::to_string(radiation_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    radiation_string.append("all except " + std::to_string(radiation_id_s1) + " to " + std::to_string(radiation_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    radiation_string.append(std::to_string(radiation_id_s1) + " to " + std::to_string(radiation_id_s2));
  }else if(data.get_strings("radiation id", radiations_string))
  {
    for (size_t i = 0; i < radiations_string.size(); i++)
    {
      radiation_string.append(radiations_string[i]);
    }
  }
  
  if(!data.get_values("radiation id", radiation_ids))
  {   
    radiation_ids = ccx_iface.parser("loadsradiation", radiation_string);
  }

  if (radiation_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < radiation_ids.size(); i++)
  {    
    if (!ccx_iface.delete_loadsradiation(radiation_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(radiation_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}