#include "CalculiXJobsInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

bool CalculiXJobsInterface::create_job(std::vector<std::string> options)
{
  return ccx_core.create_job(options);
}

bool CalculiXJobsInterface::modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_job(job_id, options, options_marker);
}

bool CalculiXJobsInterface::delete_job(int job_id)
{
  return ccx_core.delete_job(job_id);
}

bool CalculiXJobsInterface::get_job_topology_context(int job_id, int& topology_id, int& iteration_id)
{
  return ccx_core.get_job_topology_context(job_id, topology_id, iteration_id);
}

bool CalculiXJobsInterface::prepare_job_input(int job_id, const std::string& filename)
{
  return ccx_core.prepare_job_input(job_id, filename);
}

bool CalculiXJobsInterface::run_job(int job_id, int option)
{
  return ccx_core.run_job(job_id, option);
}

bool CalculiXJobsInterface::check_jobs()
{
  return ccx_core.check_jobs();
}

bool CalculiXJobsInterface::wait_job(int job_id)
{
  return ccx_core.wait_job(job_id);
}

bool CalculiXJobsInterface::kill_job(int job_id)
{
  return ccx_core.kill_job(job_id);
}

bool CalculiXJobsInterface::is_checking_jobs() const
{
  return ccx_core.is_checking_jobs();
}

bool CalculiXJobsInterface::set_job_conversion(int job_id, int conversion)
{
  return ccx_core.set_job_conversion(job_id,conversion);
}

std::vector<std::string> CalculiXJobsInterface::get_job_data(int job_id)
{
  return ccx_core.get_job_data(job_id);
}

std::vector<std::string> CalculiXJobsInterface::get_job_console_output(int job_id)
{
  return ccx_core.get_job_console_output(job_id);
}

std::vector<std::string> CalculiXJobsInterface::get_job_cvg(int job_id)
{
  return ccx_core.get_job_cvg(job_id);
}

std::vector<std::string> CalculiXJobsInterface::get_job_sta(int job_id)
{
  return ccx_core.get_job_sta(job_id);
}

int CalculiXJobsInterface::get_job_status(int job_id)
{
  return ccx_core.get_job_status(job_id);
}

std::vector<std::vector<std::string>> CalculiXJobsInterface::get_jobs_tree_data()
{ 
  return ccx_core.get_jobs_tree_data();
}
