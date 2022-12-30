#include "ccxConstraintDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxConstraintDeleteCommand::ccxConstraintDeleteCommand()
{}

ccxConstraintDeleteCommand::~ccxConstraintDeleteCommand()
{}

std::vector<std::string> ccxConstraintDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete constraint ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='constraint id',help='<constraint id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='constraint id',help='<constraint id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <constraint id>...
      syntax.append("all except ");
      syntax.append("<value:label='constraint id except'>... ");
    }else if (syn_i==4)
    {
      // all except <constraint id> to <constraint id 2>
      syntax.append("all except ");
      syntax.append("<value:label='constraint id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='constraint id s2'>");
    }else if (syn_i==5)
    {
      // <constraint id> to <constraint id 2>
      syntax.append("<value:label='constraint id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='constraint id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }


  return syntax_list;
}

std::vector<std::string> ccxConstraintDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete constraint <constraint id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxConstraintDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxConstraintDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> constraint_ids;
  int constraint_id_s1;
  int constraint_id_s2;
  std::vector<std::string> constraints_string;

  std::string constraint_string = " ";
  
  data.get_value("constraint id s1", constraint_id_s1);
  data.get_value("constraint id s2", constraint_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("constraint id except", constraint_ids))
    {
      constraint_string.append("all except");
      for (size_t i = 0; i < constraint_ids.size(); i++)
      {
        constraint_string.append(" " + std::to_string(constraint_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    constraint_string.append("all except " + std::to_string(constraint_id_s1) + " to " + std::to_string(constraint_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    constraint_string.append(std::to_string(constraint_id_s1) + " to " + std::to_string(constraint_id_s2));
  }else if(data.get_strings("constraint id", constraints_string))
  {
    for (size_t i = 0; i < constraints_string.size(); i++)
    {
      constraint_string.append(constraints_string[i]);
    }
  }
  
  if(!data.get_values("constraint id", constraint_ids))
  {   
    constraint_ids = ccx_iface.parser("constraint", constraint_string);
  }

  if (constraint_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }
  
  for (size_t i = 0; i < constraint_ids.size(); i++)
  {    
    if (!ccx_iface.delete_constraint(constraint_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(constraint_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}