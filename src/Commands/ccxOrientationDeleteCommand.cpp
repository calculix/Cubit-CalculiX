#include "ccxOrientationDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxOrientationDeleteCommand::ccxOrientationDeleteCommand()
{}

ccxOrientationDeleteCommand::~ccxOrientationDeleteCommand()
{}

std::vector<std::string> ccxOrientationDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete orientation ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='orientation id',help='<orientation id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='orientation id',help='<orientation id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <orientation id>...
      syntax.append("all except ");
      syntax.append("<value:label='orientation id except'>... ");
    }else if (syn_i==4)
    {
      // all except <orientation id> to <orientation id 2>
      syntax.append("all except ");
      syntax.append("<value:label='orientation id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='orientation id s2'>");
    }else if (syn_i==5)
    {
      // <orientation id> to <orientation id 2>
      syntax.append("<value:label='orientation id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='orientation id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxOrientationDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete orientation <orientation id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxOrientationDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxOrientationDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> orientation_ids;
  int orientation_id_s1;
  int orientation_id_s2;
  std::vector<std::string> orientations_string;

  std::string orientation_string = " ";
  
  data.get_value("orientation id s1", orientation_id_s1);
  data.get_value("orientation id s2", orientation_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("orientation id except", orientation_ids))
    {
      orientation_string.append("all except");
      for (size_t i = 0; i < orientation_ids.size(); i++)
      {
        orientation_string.append(" " + std::to_string(orientation_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    orientation_string.append("all except " + std::to_string(orientation_id_s1) + " to " + std::to_string(orientation_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    orientation_string.append(std::to_string(orientation_id_s1) + " to " + std::to_string(orientation_id_s2));
  }else if(data.get_strings("orientation id", orientations_string))
  {
    for (size_t i = 0; i < orientations_string.size(); i++)
    {
      orientation_string.append(orientations_string[i]);
    }
  }
  
  if(!data.get_values("orientation id", orientation_ids))
  {   
    orientation_ids = ccx_iface.parser("orientation", orientation_string);
  }

  if (orientation_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < orientation_ids.size(); i++)
  {    
    if (!ccx_iface.delete_orientation(orientation_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(orientation_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}