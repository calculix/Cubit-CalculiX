#include "ccxInitialConditionDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxInitialConditionDeleteCommand::ccxInitialConditionDeleteCommand()
{}

ccxInitialConditionDeleteCommand::~ccxInitialConditionDeleteCommand()
{}

std::vector<std::string> ccxInitialConditionDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete initialcondition ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='initialcondition id',help='<initialcondition id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='initialcondition id',help='<initialcondition id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <initialcondition id>...
      syntax.append("all except ");
      syntax.append("<value:label='initialcondition id except'>... ");
    }else if (syn_i==4)
    {
      // all except <initialcondition id> to <initialcondition id 2>
      syntax.append("all except ");
      syntax.append("<value:label='initialcondition id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='initialcondition id s2'>");
    }else if (syn_i==5)
    {
      // <initialcondition id> to <initialcondition id 2>
      syntax.append("<value:label='initialcondition id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='initialcondition id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxInitialConditionDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete initialcondition <initialcondition id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxInitialConditionDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxInitialConditionDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> initialcondition_ids;
  int initialcondition_id_s1;
  int initialcondition_id_s2;
  std::vector<std::string> initialconditions_string;

  std::string initialcondition_string = " ";
  
  data.get_value("initialcondition id s1", initialcondition_id_s1);
  data.get_value("initialcondition id s2", initialcondition_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("initialcondition id except", initialcondition_ids))
    {
      initialcondition_string.append("all except");
      for (size_t i = 0; i < initialcondition_ids.size(); i++)
      {
        initialcondition_string.append(" " + std::to_string(initialcondition_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    initialcondition_string.append("all except " + std::to_string(initialcondition_id_s1) + " to " + std::to_string(initialcondition_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    initialcondition_string.append(std::to_string(initialcondition_id_s1) + " to " + std::to_string(initialcondition_id_s2));
  }else if(data.get_strings("initialcondition id", initialconditions_string))
  {
    for (size_t i = 0; i < initialconditions_string.size(); i++)
    {
      initialcondition_string.append(initialconditions_string[i]);
    }
  }
  
  if(!data.get_values("initialcondition id", initialcondition_ids))
  {   
    initialcondition_ids = ccx_iface.parser("initialcondition", initialcondition_string);
  }

  if (initialcondition_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < initialcondition_ids.size(); i++)
  {    
    if (!ccx_iface.delete_initialcondition(initialcondition_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(initialcondition_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}