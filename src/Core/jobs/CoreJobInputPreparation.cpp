#include "CoreJobInputPreparation.hpp"
#include "CoreJobExportContext.hpp"
#include "Core/topology/CoreTopology.hpp"
#include "Core/topology/CoreTopologyQuery.hpp"
#include "CubitMessage.hpp"
#include <QScopedValueRollback>

bool CoreJobInputPreparation::prepare(const CoreTopology* topology, CoreJobExportContext& active_context, int job_id, const std::string& filename, const std::function<bool(const std::string&)>& execute_command)
{
  if (job_id <= 0 || filename.empty() || filename.find_first_of("\"\r\n") != std::string::npos)
  {
    PRINT_ERROR("Invalid job input request.\n");
    return false;
  }

  if (active_context.job_id > 0)
  {
    PRINT_ERROR("A job input export is already active.\n");
    return false;
  }

  CoreJobExportContext context;
  context.job_id = job_id;

  if (!topology)
  {
    PRINT_ERROR("Invalid job topology context query.\n");
    return false;
  }

  if (!CoreTopologyQuery::get_job_context(*topology, job_id, context.topology_id, context.iteration_id))
  {
    return false;
  }

  if (context.topology_id > 0)
  {
    const auto& configuration = topology->topology_cases.find(context.topology_id)->second;
    const auto adjoint = configuration.iteration_adjoint_jobs.find(context.iteration_id);

    context.adjoint = adjoint != configuration.iteration_adjoint_jobs.end() && adjoint->second == job_id;
  }

  const QScopedValueRollback<CoreJobExportContext> guard(active_context, context);

  std::string command = "export ccx \"" + filename + "\"";

  if (context.topology_id > 0)
  {
    command += " topology " + std::to_string(context.topology_id);
    command += " iteration " + std::to_string(context.iteration_id);
  }

  command += " overwrite";

  return execute_command(command);
}
