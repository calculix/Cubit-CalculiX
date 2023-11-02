#include "ccxResultConvertJobCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxResultConvertJobCommand::ccxResultConvertJobCommand()
{}

ccxResultConvertJobCommand::~ccxResultConvertJobCommand()
{}

std::vector<std::string> ccxResultConvertJobCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result convert job <value:label='job_id',help='<job_id>'>");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxResultConvertJobCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result convert job <job_id>"; 

  return help;
}

std::vector<std::string> ccxResultConvertJobCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResultConvertJobCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int job_id;

  data.get_value("job_id", job_id);

  if (!ccx_iface.convert_result(job_id))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}