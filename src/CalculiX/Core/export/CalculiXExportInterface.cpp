#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "CalculiXExportInterface.hpp"
#include <string>
#include <vector>

#include "Core/export/CoreCustomLines.hpp"
#include "Core/loadCalculiXCore.hpp"
#include "Core/topology/CoreTopologyTrace.hpp"

std::vector<int> CalculiXExportInterface::get_customline_ids()
{
  return ccx_core.customlines->get_customline_ids();
}

bool CalculiXExportInterface::prepare_export()
{
  return ccx_core.loadstrajectory->prepare_export_if_needed();
}

bool CalculiXExportInterface::clean_export()
{
  return ccx_core.loadstrajectory->clean_export_if_needed();
}

void CalculiXExportInterface::freeze_job_setup() noexcept
{
  const auto& context = ccx_core.job_export_context;
  if (context.job_id > 0)
  {
    CalculiXJobsInterface().freeze_prepared_setup(context.job_id, context.filename, context.topology_id < 0 && !context.adjoint);
  }
}

void CalculiXExportInterface::trace_export(const char* event)
{
  CoreTopologyTrace::write(event);
}

bool CalculiXExportInterface::create_customline(std::vector<std::string> options)
{
  return ccx_core.customlines->create_customline(options);
}

bool CalculiXExportInterface::modify_customline(int customline_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.customlines->modify_customline(customline_id, options, options_marker);
}

bool CalculiXExportInterface::delete_customline(int customline_id)
{
  return ccx_core.customlines->delete_customline(customline_id);
}

std::vector<std::string> CalculiXExportInterface::get_customline_data(std::string position, std::string keyword, int keyword_id)
{
  return ccx_core.customlines->get_customline_data(position, keyword, keyword_id);
}

std::vector<std::vector<std::string>> CalculiXExportInterface::get_customlines_tree_data()
{ 
  return ccx_core.customlines->get_tree_data();
}


bool CalculiXExportInterface::get_customline_settings(int id, CustomLinesSettings& settings, std::string& error)
{
  return ccx_core.customlines->get_settings(id, settings, error);
}
