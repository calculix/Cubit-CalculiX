#include "ccxRunJobCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxRunJobCommand::ccxRunJobCommand()
{}

ccxRunJobCommand::~ccxRunJobCommand()
{}

std::vector<std::string> ccxRunJobCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("run job <value:label='job_id',help='<job_id>'> [no_conversion]");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxRunJobCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx run job <job_id> [no_conversion]"; 

  return help;
}

std::vector<std::string> ccxRunJobCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxRunJobCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int job_id;
  int option;

  if (data.find_keyword("NO_CONVERSION")){
    option = -1;
  }else{
    option = 1;
  }

  data.get_value("job_id", job_id);

  if (!ccx_iface.run_job(job_id,option))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}