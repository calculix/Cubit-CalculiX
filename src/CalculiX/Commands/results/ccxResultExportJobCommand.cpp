#include "ccxResultExportJobCommand.hpp"
#include "Core/results/CalculiXResultsInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxResultExportJobCommand::get_syntax()
{
  return {"ccx result export job <value:label='job_id',help='<job_id>'> "
    "[directory <string:type='quoted',number='1',label='directory',help='<new export directory>'>]"};
}

std::vector<std::string> ccxResultExportJobCommand::get_syntax_help()
{
  return {"ccx result export job <job_id> [directory \"<new export directory>\"]"};
}

std::vector<std::string> ccxResultExportJobCommand::get_help()
{
  return {"Export an already converted VTK snapshot. Does not load or convert results.",
    "The destination must be new. Without directory, create a unique run folder beside the job."};
}

bool ccxResultExportJobCommand::execute(CubitCommandData& data)
{
  int job_id = -1;
  std::string directory;
  if (!data.get_value("job_id", job_id) ||
    (data.find_keyword("DIRECTORY") && (!data.get_string("directory", directory) || directory.empty())))
  {
    PRINT_ERROR("VTK export requires a job ID and a nonempty quoted directory when supplied.\n");
    return false;
  }
  return CalculiXResultsInterface().export_vtk_result(job_id, directory);
}
