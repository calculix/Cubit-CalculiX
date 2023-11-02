#include "ccxResultProjectJobCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

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
  
  return syntax_list;
}

std::vector<std::string> ccxResultProjectJobCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result project job <job_id> {step <step> | totalincrement <totalincrement>} [scale <scale>]"; 

  return help;
}

std::vector<std::string> ccxResultProjectJobCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResultProjectJobCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int job_id = -1;
  int step = -1;
  int totalincrement = -1;
  double scale = 1;

  data.get_value("job_id", job_id);
  data.get_value("step", step);
  data.get_value("totalincrement", totalincrement);
  data.get_value("scale", scale);

  if (!ccx_iface.project_result(job_id,step,totalincrement,scale))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}