#include "Framework/Reporting/ReportSource.hpp"
#include <QFileInfo>
#include "CalculiXJobsInterface.hpp"
#include "Core/topology/CoreTopologyQuery.hpp"
#include "CubitMessage.hpp"
#include <string>
#include <vector>

#include "Core/jobs/CoreJobs.hpp"
#include "Core/loadCalculiXCore.hpp"

const CalculiXJobProgress::Data* CalculiXJobsInterface::get_job_progress(int job_id, std::string& error)
{
  if (!ccx_core.jobs)
  {
    error = "CalculiX jobs are unavailable.";
    return nullptr;
  }
  return ccx_core.jobs->get_job_progress(job_id, error);
}

bool CalculiXJobsInterface::get_monitor_snapshot(std::vector<JobMonitorEntry>& output, std::string& error) const
{
  if (ccx_core.jobs == nullptr)
  {
    error = "CalculiX jobs are unavailable.";
    return false;
  }
  return ccx_core.jobs->get_monitor_snapshot(output, error);
}

bool CalculiXJobsInterface::create_job(std::vector<std::string> options)
{
  return ccx_core.jobs->create_job(options);
}

bool CalculiXJobsInterface::modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.jobs->modify_job(job_id, options, options_marker);
}

bool CalculiXJobsInterface::delete_job(int job_id)
{
  return ccx_core.jobs->delete_job(job_id);
}

bool CalculiXJobsInterface::get_job_topology_context(int job_id, int& topology_id, int& iteration_id)
{
  topology_id = -1;
  iteration_id = -1;

  if (!ccx_core.topology)
  {
    PRINT_ERROR("Invalid job topology context query.\n");
    return false;
  }

  return CoreTopologyQuery::get_job_context(*ccx_core.topology, job_id, topology_id, iteration_id);
}

bool CalculiXJobsInterface::prepare_job_input(int job_id, const std::string& filename)
{
  return ccx_core.prepare_job_input(job_id, filename);
}

bool CalculiXJobsInterface::run_job(int job_id, int option)
{
  return ccx_core.jobs->run_job(job_id, option);
}

bool CalculiXJobsInterface::check_jobs()
{
  return ccx_core.check_jobs();
}

bool CalculiXJobsInterface::wait_job(int job_id)
{
  return ccx_core.jobs->wait_job(job_id);
}

bool CalculiXJobsInterface::kill_job(int job_id)
{
  return ccx_core.jobs->kill_job(job_id);
}

bool CalculiXJobsInterface::is_checking_jobs() const
{
  return ccx_core.is_checking_jobs();
}

bool CalculiXJobsInterface::set_job_conversion(int job_id, int conversion)
{
  return ccx_core.jobs->set_job_conversion(job_id, conversion);
}

std::vector<std::string> CalculiXJobsInterface::get_job_data(int job_id)
{
  return ccx_core.jobs->get_job_data(job_id);
}

std::vector<std::string> CalculiXJobsInterface::get_job_console_output(int job_id)
{
  return ccx_core.jobs->get_job_console_output(job_id);
}

std::vector<std::string> CalculiXJobsInterface::get_job_cvg(int job_id)
{
  return ccx_core.jobs->get_job_cvg(job_id);
}

std::vector<std::string> CalculiXJobsInterface::get_job_sta(int job_id)
{
  return ccx_core.jobs->get_job_sta(job_id);
}

int CalculiXJobsInterface::get_job_status(int job_id)
{
  return ccx_core.jobs->get_job_status(job_id);
}

std::vector<std::vector<std::string>> CalculiXJobsInterface::get_jobs_tree_data()
{ 
  return ccx_core.jobs->get_tree_data();
}


bool CalculiXJobsInterface::get_job_settings(int id, JobsSettings& settings, std::string& error)
{
  return ccx_core.jobs->get_settings(id, settings, error);
}

bool CalculiXJobsInterface::get_report_source(int id, ReportSource& output, std::string& error)
{
  JobsSettings settings;
  if (id <= 0 || !get_job_settings(id, settings, error))
  {
    error = "Cannot resolve the CalculiX report job.";
    return false;
  }
  if (get_job_status(id) == 1)
  {
    error = "Cannot read report input while the CalculiX job is running.";
    return false;
  }
  // Solver execution writes <job name>.inp in its working directory. Prefer
  // that actual run input over an imported source deck when both exist.
  QString input = QString::fromStdString(settings.name) + ".inp";
  if (!QFileInfo(input).isFile())
  {
    input = QString::fromStdString(settings.filepath);
  }
  const auto canonical = QFileInfo(input).canonicalFilePath();
  if (canonical.isEmpty() || !QFileInfo(canonical).isFile())
  {
    error = "The job input deck is unavailable; reporting will not regenerate it.";
    return false;
  }
  ReportSource source;
  source.workspace_id = "calculix";
  source.job_id = id;
  source.input_files = {canonical.toStdString()};
  source.provenance = "Existing input deck; historical model/result run linkage is unverified.";
  output = std::move(source);
  error.clear();
  return true;
}
