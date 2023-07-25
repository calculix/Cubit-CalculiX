#include "ccxContactPairDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxContactPairDeleteCommand::ccxContactPairDeleteCommand()
{}

ccxContactPairDeleteCommand::~ccxContactPairDeleteCommand()
{}

std::vector<std::string> ccxContactPairDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete contactpair ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='contactpair id',help='<contactpair id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='contactpair id',help='<contactpair id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <contactpair id>...
      syntax.append("all except ");
      syntax.append("<value:label='contactpair id except'>... ");
    }else if (syn_i==4)
    {
      // all except <contactpair id> to <contactpair id 2>
      syntax.append("all except ");
      syntax.append("<value:label='contactpair id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='contactpair id s2'>");
    }else if (syn_i==5)
    {
      // <contactpair id> to <contactpair id 2>
      syntax.append("<value:label='contactpair id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='contactpair id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxContactPairDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete contactpair <contactpair id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxContactPairDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxContactPairDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> contactpair_ids;
  int contactpair_id_s1;
  int contactpair_id_s2;
  std::vector<std::string> contactpairs_string;

  std::string contactpair_string = " ";
  
  data.get_value("contactpair id s1", contactpair_id_s1);
  data.get_value("contactpair id s2", contactpair_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("contactpair id except", contactpair_ids))
    {
      contactpair_string.append("all except");
      for (size_t i = 0; i < contactpair_ids.size(); i++)
      {
        contactpair_string.append(" " + std::to_string(contactpair_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    contactpair_string.append("all except " + std::to_string(contactpair_id_s1) + " to " + std::to_string(contactpair_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    contactpair_string.append(std::to_string(contactpair_id_s1) + " to " + std::to_string(contactpair_id_s2));
  }else if(data.get_strings("contactpair id", contactpairs_string))
  {
    for (size_t i = 0; i < contactpairs_string.size(); i++)
    {
      contactpair_string.append(contactpairs_string[i]);
    }
  }
  
  if(!data.get_values("contactpair id", contactpair_ids))
  {   
    contactpair_ids = ccx_iface.parser("contactpair", contactpair_string);
  }

  if (contactpair_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < contactpair_ids.size(); i++)
  {    
    if (!ccx_iface.delete_contactpair(contactpair_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(contactpair_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}