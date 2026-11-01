#include "CoreTopologyExportValidation.hpp"
#include "Core/jobs/CoreJobExportContext.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "CubitMessage.hpp"

bool CoreTopologyExportValidation::check(const CoreJobExportContext& context, CoreCustomLines* customlines, int topology_id, int iteration_id, const std::function<bool()>& validate_model)
{
  if (context.job_id > 0)
  {
    if (context.topology_id != topology_id || context.iteration_id != iteration_id)
    {
      PRINT_ERROR("Export selection differs from the active job context.\n");
      return false;
    }
  }

  if (!context.adjoint)
  {
    return true;
  }

  if (topology_id <= 0 || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology adjoint export context.\n");
    return false;
  }

  if (!customlines)
  {
    PRINT_ERROR("Custom input interface is not initialized.\n");
    return false;
  }

  if (!customlines->customlines_data.empty())
  {
    PRINT_ERROR("Adjoint export does not yet support custom input lines.\n");
    return false;
  }

  return validate_model();
}
