#include "Core/CalculiXCore.hpp"
#include "CoreJobs.hpp"
#include "CoreJobInputPreparation.hpp"
#include "Core/topology/CoreTopologyOptimization.hpp"
#include "Core/topology/CoreTopologyTrace.hpp"
#include <QScopedValueRollback>

bool CalculiXCore::create_job(std::vector<std::string> options)
{
  return jobs->create_job(options);
}

bool CalculiXCore::modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return jobs->modify_job(job_id, options, options_marker);
}

bool CalculiXCore::delete_job(int job_id)
{
  return jobs->delete_job(job_id);
}

bool CalculiXCore::run_job(int job_id, int option)
{
  return jobs->run_job(job_id, option);
}

bool CalculiXCore::wait_job(int job_id)
{
  return jobs->wait_job(job_id);
}

bool CalculiXCore::kill_job(int job_id)
{
  return jobs->kill_job(job_id);
}

bool CalculiXCore::set_job_conversion(int job_id, int conversion)
{
  return jobs->set_job_conversion(job_id, conversion);
}

std::vector<std::string> CalculiXCore::get_job_data(int job_id)
{
  return jobs->get_job_data(job_id);
}

std::vector<std::string> CalculiXCore::get_job_console_output(int job_id)
{
  return jobs->get_job_console_output(job_id);
}

std::vector<std::string> CalculiXCore::get_job_cvg(int job_id)
{
  return jobs->get_job_cvg(job_id);
}

std::vector<std::string> CalculiXCore::get_job_sta(int job_id)
{
  return jobs->get_job_sta(job_id);
}

int CalculiXCore::get_job_status(int job_id)
{
  return jobs->get_job_status(job_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_jobs_tree_data()
{
  return jobs->get_tree_data();
}



bool CalculiXCore::prepare_job_input(int job_id, const std::string& filename)
{
  const auto execute_command = [this](const std::string& text)
  {
    CoreTopologyTrace::write("job.export_command.enter");
    const bool result = silent_cmd(text);
    CoreTopologyTrace::write(result ? "job.export_command.success" : "job.export_command.failure");
    return result;
  };

  return CoreJobInputPreparation::prepare(topology, job_export_context, job_id, filename, execute_command);
}

bool CalculiXCore::is_topology_adjoint_export() const
{
  return job_export_context.adjoint;
}

bool CalculiXCore::check_jobs()
{
  if (!jobs)
  {
    return false;
  }

  if (checking_jobs)
  {
    return true;
  }

  const QScopedValueRollback<bool> guard(checking_jobs, true);
  const bool result = jobs->check_jobs();

  if (topologyoptimization)
  {
    topologyoptimization->update();
  }

  return result;
}
