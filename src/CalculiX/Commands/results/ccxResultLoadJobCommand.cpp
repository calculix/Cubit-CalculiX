#include "ccxResultLoadJobCommand.hpp"
#include <string>
#include <vector>
#include "Core/results/CalculiXResultsInterface.hpp"
#include "Core/results/vtk/CoreResultsVtkStructs.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxResultLoadJobCommand::ccxResultLoadJobCommand()
{}

ccxResultLoadJobCommand::~ccxResultLoadJobCommand()
{}

std::vector<std::string> ccxResultLoadJobCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result load job <value:label='job_id',help='<job_id>'> [convert]");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxResultLoadJobCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result load job <job_id> [convert]";

  return help;
}

std::vector<std::string> ccxResultLoadJobCommand::get_help()
{
  return {"Load job results. With convert, build retained VTK datasets only after loading succeeds; conversion failure fails the command."};
}

bool ccxResultLoadJobCommand::execute(CubitCommandData& data)
{
  CalculiXResultsInterface results;
  int jobId = 0;
  if (!data.get_value("job_id",jobId))
  {
    PRINT_ERROR("A job ID is required.\n");
    return false;
  }
  if (!results.load_result(jobId))
  {
    PRINT_ERROR("Failed to load results for job %d.\n",jobId);
    return false;
  }
  if (data.find_keyword("CONVERT"))
  {
    CoreResultsVtkOptions settings;
    return results.build_vtk_result(jobId,settings);
  }
  return true;
}
