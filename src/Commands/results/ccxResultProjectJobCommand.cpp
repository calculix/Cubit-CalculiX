#include "ccxResultProjectJobCommand.hpp"
#include <string>
#include <vector>
#include "Core/results/CalculiXResultsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxResultProjectJobCommand::ccxResultProjectJobCommand()
{}

ccxResultProjectJobCommand::~ccxResultProjectJobCommand()
{}

std::vector<std::string> ccxResultProjectJobCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result project job <value:label='job_id',help='<job_id>'> ");
  syntax.append("{step <value:label='step',help='<step>'> ");
  syntax.append("|totalincrement <value:label='totalincrement',help='<totalincrement>'>} ");
  syntax.append("[scale <value:label='scale',help='<scale>'>] ");
  syntax_list.push_back(syntax);

  syntax_list.push_back(
    "ccx result project topology <value:label='topology_id'> "
    "[iteration <value:label='iteration_id'>] "
    "[threshold <value:label='threshold'>] "
    "[preview]");
  
  return syntax_list;
}

std::vector<std::string> ccxResultProjectJobCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result project job <job_id> {step <step> | totalincrement <totalincrement>} [scale <scale>]"; 

  help.push_back(
    "ccx result project topology <topology_id> "
    "[iteration <iteration_id>] [threshold <threshold>] "
    "[preview]");

  return help;
}

std::vector<std::string> ccxResultProjectJobCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResultProjectJobCommand::execute(CubitCommandData &data)
{
  CalculiXResultsInterface ccx_results_iface;


  if (data.find_keyword("topology"))
  {
    int topology_id = -1;
    int iteration_id = -1;
    double threshold = 0.5;

    if (!data.get_value("topology_id", topology_id))
    {
      PRINT_ERROR("Missing topology ID.\n");
      return false;
    }

    if (data.find_keyword("iteration"))
    {
      if (!data.get_value("iteration_id", iteration_id))
      {
        PRINT_ERROR("Invalid topology iteration.\n");
        return false;
      }
    }

    if (data.find_keyword("threshold"))
    {
      if (!data.get_value("threshold", threshold))
      {
        PRINT_ERROR("Invalid topology threshold.\n");
        return false;
      }
    }

    return ccx_results_iface.project_topology(
        topology_id,
        iteration_id,
        threshold,
        data.find_keyword("preview"));
  }

  std::string output;
  int job_id = -1;
  int step = -1;
  int totalincrement = -1;
  double scale = 1;

  data.get_value("job_id", job_id);
  data.get_value("step", step);
  data.get_value("totalincrement", totalincrement);
  data.get_value("scale", scale);

  if (!ccx_results_iface.project_result(job_id,step,totalincrement,scale))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}