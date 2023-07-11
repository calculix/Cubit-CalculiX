#include "ccxCustomLineDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxCustomLineDeleteCommand::ccxCustomLineDeleteCommand()
{}

ccxCustomLineDeleteCommand::~ccxCustomLineDeleteCommand()
{}

std::vector<std::string> ccxCustomLineDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete customline ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='customline id',help='<customline id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='customline id',help='<customline id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <customline id>...
      syntax.append("all except ");
      syntax.append("<value:label='customline id except'>... ");
    }else if (syn_i==4)
    {
      // all except <customline id> to <customline id 2>
      syntax.append("all except ");
      syntax.append("<value:label='customline id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='customline id s2'>");
    }else if (syn_i==5)
    {
      // <customline id> to <customline id 2>
      syntax.append("<value:label='customline id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='customline id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }
  
  return syntax_list;
}

std::vector<std::string> ccxCustomLineDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete customline <customline_id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";

  return help;
}

std::vector<std::string> ccxCustomLineDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxCustomLineDeleteCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<int> customline_ids;
  int customline_id_s1;
  int customline_id_s2;
  std::vector<std::string> customlines_string;

  std::string customline_string = " ";
  
  data.get_value("customline id s1", customline_id_s1);
  data.get_value("customline id s2", customline_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("customline id except", customline_ids))
    {
      customline_string.append("all except");
      for (size_t i = 0; i < customline_ids.size(); i++)
      {
        customline_string.append(" " + std::to_string(customline_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    customline_string.append("all except " + std::to_string(customline_id_s1) + " to " + std::to_string(customline_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    customline_string.append(std::to_string(customline_id_s1) + " to " + std::to_string(customline_id_s2));
  }else if(data.get_strings("customline id", customlines_string))
  {
    for (size_t i = 0; i < customlines_string.size(); i++)
    {
      customline_string.append(customlines_string[i]);
    }
  }
  
  if(!data.get_values("customline id", customline_ids))
  {   
    customline_ids = ccx_iface.parser("customline", customline_string);
  }

  if (customline_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < customline_ids.size(); i++)
  {    
    if (!ccx_iface.delete_customline(customline_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(customline_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
    
  return true;
}