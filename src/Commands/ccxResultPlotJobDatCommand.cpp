#include "ccxResultPlotJobDatCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxResultPlotJobDatCommand::ccxResultPlotJobDatCommand()
{}

ccxResultPlotJobDatCommand::~ccxResultPlotJobDatCommand()
{}

std::vector<std::string> ccxResultPlotJobDatCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result plot job <value:label='job_id',help='<job_id>'> ");
  syntax.append("dat ");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxResultPlotJobDatCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result plot job <job_id> dat "; 

  return help;
}

std::vector<std::string> ccxResultPlotJobDatCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResultPlotJobDatCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int job_id;

  data.get_value("job_id", job_id);

  if (!ccx_iface.result_plot_job_dat(job_id))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}