#include "ccxLoadsCentrifugalDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsCentrifugalDeleteCommand::ccxLoadsCentrifugalDeleteCommand()
{}

ccxLoadsCentrifugalDeleteCommand::~ccxLoadsCentrifugalDeleteCommand()
{}

std::vector<std::string> ccxLoadsCentrifugalDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete centrifugal ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='centrifugal id',help='<centrifugal id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='centrifugal id',help='<centrifugal id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <centrifugal id>...
      syntax.append("all except ");
      syntax.append("<value:label='centrifugal id except'>... ");
    }else if (syn_i==4)
    {
      // all except <centrifugal id> to <centrifugal id 2>
      syntax.append("all except ");
      syntax.append("<value:label='centrifugal id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='centrifugal id s2'>");
    }else if (syn_i==5)
    {
      // <centrifugal id> to <centrifugal id 2>
      syntax.append("<value:label='centrifugal id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='centrifugal id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxLoadsCentrifugalDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete centrifugal <centrifugal id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxLoadsCentrifugalDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsCentrifugalDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> centrifugal_ids;
  int centrifugal_id_s1;
  int centrifugal_id_s2;
  std::vector<std::string> centrifugals_string;

  std::string centrifugal_string = " ";
  
  data.get_value("centrifugal id s1", centrifugal_id_s1);
  data.get_value("centrifugal id s2", centrifugal_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("centrifugal id except", centrifugal_ids))
    {
      centrifugal_string.append("all except");
      for (size_t i = 0; i < centrifugal_ids.size(); i++)
      {
        centrifugal_string.append(" " + std::to_string(centrifugal_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    centrifugal_string.append("all except " + std::to_string(centrifugal_id_s1) + " to " + std::to_string(centrifugal_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    centrifugal_string.append(std::to_string(centrifugal_id_s1) + " to " + std::to_string(centrifugal_id_s2));
  }else if(data.get_strings("centrifugal id", centrifugals_string))
  {
    for (size_t i = 0; i < centrifugals_string.size(); i++)
    {
      centrifugal_string.append(centrifugals_string[i]);
    }
  }
  
  if(!data.get_values("centrifugal id", centrifugal_ids))
  {   
    centrifugal_ids = ccx_iface.parser("loadscentrifugal", centrifugal_string);
  }

  if (centrifugal_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < centrifugal_ids.size(); i++)
  {    
    if (!ccx_iface.delete_loadscentrifugal(centrifugal_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(centrifugal_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}