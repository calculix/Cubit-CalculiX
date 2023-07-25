#include "ccxSurfaceInteractionDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionDeleteCommand::ccxSurfaceInteractionDeleteCommand()
{}

ccxSurfaceInteractionDeleteCommand::~ccxSurfaceInteractionDeleteCommand()
{}

std::vector<std::string> ccxSurfaceInteractionDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete surfaceinteraction ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='surfaceinteraction id',help='<surfaceinteraction id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='surfaceinteraction id',help='<surfaceinteraction id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <surfaceinteraction id>...
      syntax.append("all except ");
      syntax.append("<value:label='surfaceinteraction id except'>... ");
    }else if (syn_i==4)
    {
      // all except <surfaceinteraction id> to <surfaceinteraction id 2>
      syntax.append("all except ");
      syntax.append("<value:label='surfaceinteraction id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='surfaceinteraction id s2'>");
    }else if (syn_i==5)
    {
      // <surfaceinteraction id> to <surfaceinteraction id 2>
      syntax.append("<value:label='surfaceinteraction id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='surfaceinteraction id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }


  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete surfaceinteraction <surfaceinteraction id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxSurfaceInteractionDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> surfaceinteraction_ids;
  int surfaceinteraction_id_s1;
  int surfaceinteraction_id_s2;
  std::vector<std::string> surfaceinteractions_string;

  std::string surfaceinteraction_string = " ";
  
  data.get_value("surfaceinteraction id s1", surfaceinteraction_id_s1);
  data.get_value("surfaceinteraction id s2", surfaceinteraction_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("surfaceinteraction id except", surfaceinteraction_ids))
    {
      surfaceinteraction_string.append("all except");
      for (size_t i = 0; i < surfaceinteraction_ids.size(); i++)
      {
        surfaceinteraction_string.append(" " + std::to_string(surfaceinteraction_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    surfaceinteraction_string.append("all except " + std::to_string(surfaceinteraction_id_s1) + " to " + std::to_string(surfaceinteraction_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    surfaceinteraction_string.append(std::to_string(surfaceinteraction_id_s1) + " to " + std::to_string(surfaceinteraction_id_s2));
  }else if(data.get_strings("surfaceinteraction id", surfaceinteractions_string))
  {
    for (size_t i = 0; i < surfaceinteractions_string.size(); i++)
    {
      surfaceinteraction_string.append(surfaceinteractions_string[i]);
    }
  }
  
  if(!data.get_values("surfaceinteraction id", surfaceinteraction_ids))
  {   
    surfaceinteraction_ids = ccx_iface.parser("surfaceinteraction", surfaceinteraction_string);
  }

  if (surfaceinteraction_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < surfaceinteraction_ids.size(); i++)
  {    
    if (!ccx_iface.delete_surfaceinteraction(surfaceinteraction_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(surfaceinteraction_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}