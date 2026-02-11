#include "ccxStepUtilityModelChangeElementDummyStepCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepUtilityModelChangeElementDummyStepCommand::ccxStepUtilityModelChangeElementDummyStepCommand()
{}

ccxStepUtilityModelChangeElementDummyStepCommand::~ccxStepUtilityModelChangeElementDummyStepCommand()
{}

std::vector<std::string> ccxStepUtilityModelChangeElementDummyStepCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("utility step <value:label='step id',help='<step id>'> ");
    syntax.append("modelchangeelement dummystep " );
    syntax.append("trajectory ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='trajectory id',help='<trajectory id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='trajectory id',help='<trajectory id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <trajectory id>...
      syntax.append("all except ");
      syntax.append("<value:label='trajectory id except'>... ");
    }else if (syn_i==4)
    {
      // all except <block id> to <block id 2>
      syntax.append("all except ");
      syntax.append("<value:label='trajectory id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='trajectory id s2'>");
    }else if (syn_i==5)
    {
      // <block id> to <block id 2>
      syntax.append("<value:label='trajectory id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='trajectory id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxStepUtilityModelChangeElementDummyStepCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx utility step <step id> modelchangeelement dummystep trajectory <trajectory id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxStepUtilityModelChangeElementDummyStepCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepUtilityModelChangeElementDummyStepCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int step_id;
  std::vector<int> trajectory_ids;
  int trajectory_id_s1;
  int trajectory_id_s2;
  std::vector<std::string> trajectorys_string;

  std::string trajectory_string = " ";
  
  data.get_value("step id", step_id);

  data.get_value("trajectory id s1", trajectory_id_s1);
  data.get_value("trajectory id s2", trajectory_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("trajectory id except", trajectory_ids))
    {
      trajectory_string.append("all except");
      for (size_t i = 0; i < trajectory_ids.size(); i++)
      {
        trajectory_string.append(" " + std::to_string(trajectory_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    trajectory_string.append("all except " + std::to_string(trajectory_id_s1) + " to " + std::to_string(trajectory_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    trajectory_string.append(std::to_string(trajectory_id_s1) + " to " + std::to_string(trajectory_id_s2));
  }else if(data.get_strings("trajectory id", trajectorys_string))
  {
    for (size_t i = 0; i < trajectorys_string.size(); i++)
    {
      trajectory_string.append(trajectorys_string[i]);
    }
  }
  
  if(!data.get_values("trajectory id", trajectory_ids))
  {   
    trajectory_ids = ccx_iface.parser("loadstrajectory", trajectory_string);
  }else{
    /*for (size_t i = 0; i < trajectory_ids.size(); i++)
    {
      trajectory_string.append(std::to_string(trajectory_ids[i]) + " ");
      trajectory_ids = ccx_iface.parser("loadstrajectory", trajectory_string);
    }*/
  }
  if (trajectory_ids.size()==0)
  {
    PRINT_ERROR("No trajectory found.\n");
    return false;
  }

  if (!ccx_iface.check_step_exists(step_id))
  {
    output = "No Step ID " + std::to_string(step_id) + " found.\n";
    PRINT_ERROR(output.c_str());
    return false;
  }
  
  if (!ccx_iface.step_utility_modelchangeelement_dummystep(step_id,trajectory_ids))
  {
    //PRINT_INFO("%s", output.c_str());  
    output = "Failed with Step ID " + std::to_string(step_id) + "!\n";
    PRINT_ERROR(output.c_str());
  } 

  return true;
}