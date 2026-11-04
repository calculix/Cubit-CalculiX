#include "ccxResultConvertJobCommand.hpp"
#include <string>
#include <vector>
#include "Core/results/vtk/CoreResultsVtkStructs.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/results/CalculiXResultsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
//#include "MeshExportInterface.hpp"
//#include "CubitEntity.hpp"

ccxResultConvertJobCommand::ccxResultConvertJobCommand()
{}

ccxResultConvertJobCommand::~ccxResultConvertJobCommand()
{}

std::vector<std::string> ccxResultConvertJobCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result convert job <value:label='job_id',help='<job_id>'> [frd] [dat] [no_partial] [in_memory]");
  syntax.append("[block <value:label='block_ids',help='<block_ids>'>...] " );
  syntax.append("[nodeset <value:label='nodeset_ids',help='<nodeset_ids>'>...] " );
  syntax.append("[sideset <value:label='sideset_ids',help='<sideset_ids>'>...] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxResultConvertJobCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result convert job <job_id> [frd] [dat] [no_partial] [in_memory]";
  help[0].append("[block <block_ids>...] " );
  help[0].append("[nodeset <nodeset_ids>...] " );
  help[0].append("[sideset <sideset_ids>...] " );

  return help;
}

std::vector<std::string> ccxResultConvertJobCommand::get_help()
{
  return {
      "Incomplete result fields are exported by default, "
      "with NaN for missing values. "
      "Use no_partial to skip incomplete fields. "
      "Use in_memory to retain datasets without writing files. "
      "Without frd/dat flags both reader sources are included."
  };
}

bool ccxResultConvertJobCommand::execute(CubitCommandData &data)
{
  CalculiXResultsInterface results;
  int job_id = 0;
  data.get_value("job_id", job_id);
  CoreResultsVtkOptions settings;
  settings.include_partial = !data.find_keyword("NO_PARTIAL");
  const bool frd = data.find_keyword("FRD");
  const bool dat = data.find_keyword("DAT");
  settings.include_frd = frd || !dat;
  settings.include_dat = dat || !frd;
  if (data.find_keyword("BLOCK"))
  {
    data.get_values("block_ids", settings.block_ids);
  }
  if (data.find_keyword("NODESET"))
  {
    data.get_values("nodeset_ids", settings.nodeset_ids);
  }
  if (data.find_keyword("SIDESET"))
  {
    data.get_values("sideset_ids", settings.sideset_ids);
  }
  // Resolve requested IDs against the linked run mesh, not a possibly changed
  // current model. Empty selections retain all groups of that kind.
  if (!results.build_vtk_result(job_id, settings))
  {
    return false;
  }
  if (data.find_keyword("IN_MEMORY"))
  {
    return true;
  }
  if (!results.export_vtk_result(job_id))
  {
    return false;
  }
  return CalculiXJobsInterface().set_job_conversion(job_id, 1);
}
