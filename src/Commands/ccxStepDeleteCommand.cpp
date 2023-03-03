#include "ccxStepDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepDeleteCommand::ccxStepDeleteCommand()
{}

ccxStepDeleteCommand::~ccxStepDeleteCommand()
{}

std::vector<std::string> ccxStepDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete step ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='step id',help='<step id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='step id',help='<step id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <step id>...
      syntax.append("all except ");
      syntax.append("<value:label='step id except'>... ");
    }else if (syn_i==4)
    {
      // all except <step id> to <step id 2>
      syntax.append("all except ");
      syntax.append("<value:label='step id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='step id s2'>");
    }else if (syn_i==5)
    {
      // <step id> to <step id 2>
      syntax.append("<value:label='step id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='step id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxStepDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete step <step id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxStepDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> step_ids;
  int step_id_s1;
  int step_id_s2;
  std::vector<std::string> steps_string;

  std::string step_string = " ";
  
  data.get_value("step id s1", step_id_s1);
  data.get_value("step id s2", step_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("step id except", step_ids))
    {
      step_string.append("all except");
      for (size_t i = 0; i < step_ids.size(); i++)
      {
        step_string.append(" " + std::to_string(step_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    step_string.append("all except " + std::to_string(step_id_s1) + " to " + std::to_string(step_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    step_string.append(std::to_string(step_id_s1) + " to " + std::to_string(step_id_s2));
  }else if(data.get_strings("step id", steps_string))
  {
    for (size_t i = 0; i < steps_string.size(); i++)
    {
      step_string.append(steps_string[i]);
    }
  }
  
  if(!data.get_values("step id", step_ids))
  {   
    step_ids = ccx_iface.parser("step", step_string);
  }

  if (step_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < step_ids.size(); i++)
  {    
    if (!ccx_iface.delete_step(step_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(step_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}