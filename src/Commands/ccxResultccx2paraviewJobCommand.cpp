#include "ccxResultccx2paraviewJobCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxResultccx2paraviewJobCommand::ccxResultccx2paraviewJobCommand()
{}

ccxResultccx2paraviewJobCommand::~ccxResultccx2paraviewJobCommand()
{}

std::vector<std::string> ccxResultccx2paraviewJobCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result ccx2paraview job <value:label='job_id',help='<job_id>'>");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxResultccx2paraviewJobCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result ccx2paraview  job <job_id>"; 

  return help;
}

std::vector<std::string> ccxResultccx2paraviewJobCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResultccx2paraviewJobCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int job_id;

  data.get_value("job_id", job_id);

  if (!ccx_iface.result_ccx2paraview_job(job_id))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}