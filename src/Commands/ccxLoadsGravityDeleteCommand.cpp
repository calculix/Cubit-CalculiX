#include "ccxLoadsGravityDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsGravityDeleteCommand::ccxLoadsGravityDeleteCommand()
{}

ccxLoadsGravityDeleteCommand::~ccxLoadsGravityDeleteCommand()
{}

std::vector<std::string> ccxLoadsGravityDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete gravity ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='gravity id',help='<gravity id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='gravity id',help='<gravity id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <gravity id>...
      syntax.append("all except ");
      syntax.append("<value:label='gravity id except'>... ");
    }else if (syn_i==4)
    {
      // all except <gravity id> to <gravity id 2>
      syntax.append("all except ");
      syntax.append("<value:label='gravity id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='gravity id s2'>");
    }else if (syn_i==5)
    {
      // <gravity id> to <gravity id 2>
      syntax.append("<value:label='gravity id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='gravity id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxLoadsGravityDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete gravity <gravity id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxLoadsGravityDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsGravityDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> gravity_ids;
  int gravity_id_s1;
  int gravity_id_s2;
  std::vector<std::string> gravitys_string;

  std::string gravity_string = " ";
  
  data.get_value("gravity id s1", gravity_id_s1);
  data.get_value("gravity id s2", gravity_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("gravity id except", gravity_ids))
    {
      gravity_string.append("all except");
      for (size_t i = 0; i < gravity_ids.size(); i++)
      {
        gravity_string.append(" " + std::to_string(gravity_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    gravity_string.append("all except " + std::to_string(gravity_id_s1) + " to " + std::to_string(gravity_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    gravity_string.append(std::to_string(gravity_id_s1) + " to " + std::to_string(gravity_id_s2));
  }else if(data.get_strings("gravity id", gravitys_string))
  {
    for (size_t i = 0; i < gravitys_string.size(); i++)
    {
      gravity_string.append(gravitys_string[i]);
    }
  }
  
  if(!data.get_values("gravity id", gravity_ids))
  {   
    gravity_ids = ccx_iface.parser("loadsgravity", gravity_string);
  }

  if (gravity_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < gravity_ids.size(); i++)
  {    
    if (!ccx_iface.delete_loadsgravity(gravity_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(gravity_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}