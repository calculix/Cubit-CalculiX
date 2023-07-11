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
    syntax.append("delete job ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='job id',help='<job id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='job id',help='<job id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <job id>...
      syntax.append("all except ");
      syntax.append("<value:label='job id except'>... ");
    }else if (syn_i==4)
    {
      // all except <job id> to <job id 2>
      syntax.append("all except ");
      syntax.append("<value:label='job id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='job id s2'>");
    }else if (syn_i==5)
    {
      // <job id> to <job id 2>
      syntax.append("<value:label='job id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='job id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }
  
  return syntax_list;
}

std::vector<std::string> ccxCustomLineDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete job <job_id>";
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
  std::vector<int> job_ids;
  int job_id_s1;
  int job_id_s2;
  std::vector<std::string> jobs_string;

  std::string job_string = " ";
  
  data.get_value("job id s1", job_id_s1);
  data.get_value("job id s2", job_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("job id except", job_ids))
    {
      job_string.append("all except");
      for (size_t i = 0; i < job_ids.size(); i++)
      {
        job_string.append(" " + std::to_string(job_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    job_string.append("all except " + std::to_string(job_id_s1) + " to " + std::to_string(job_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    job_string.append(std::to_string(job_id_s1) + " to " + std::to_string(job_id_s2));
  }else if(data.get_strings("job id", jobs_string))
  {
    for (size_t i = 0; i < jobs_string.size(); i++)
    {
      job_string.append(jobs_string[i]);
    }
  }
  
  if(!data.get_values("job id", job_ids))
  {   
    job_ids = ccx_iface.parser("job", job_string);
  }

  if (job_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < job_ids.size(); i++)
  {    
    if (!ccx_iface.delete_job(job_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(job_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
    
  return true;
}