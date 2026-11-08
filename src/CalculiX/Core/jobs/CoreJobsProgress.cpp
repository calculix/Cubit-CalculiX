#include "CoreJobs.hpp"

void CoreJobs::clear_job_progress()
{
  job_progress.clear();
}

bool CoreJobs::get_cvgsta(int job_id)
{
  const int index = get_jobs_data_id_from_job_id(job_id);
  if (index < 0)
  {
    return false;
  }
  auto& progress = job_progress[job_id];
  auto& cvg_lines = cvg[std::stoi(jobs_data[index][7])];
  auto& sta_lines = sta[std::stoi(jobs_data[index][8])];
  bool cvg_restart = false;
  bool sta_restart = false;
  std::string cvg_error;
  std::string sta_error;
  const bool cvg_ok = progress.cvg.read(jobs_data[index][1] + ".cvg", cvg_lines, cvg_restart, cvg_error);
  const bool sta_ok = progress.sta.read(jobs_data[index][1] + ".sta", sta_lines, sta_restart, sta_error);
  if (cvg_restart || sta_restart)
  {
    progress.parser.reset();
  }
  progress.parser.update(cvg_lines, sta_lines);
  progress.error = cvg_error;
  if (!sta_error.empty())
  {
    if (!progress.error.empty())
    {
      progress.error += "\n";
    }
    progress.error += sta_error;
  }
  return cvg_ok && sta_ok;
}

const CalculiXJobProgress::Data* CoreJobs::get_job_progress(int job_id, std::string& error)
{
  error.clear();
  if (get_jobs_data_id_from_job_id(job_id) < 0)
  {
    error = "The monitored job no longer exists.";
    return nullptr;
  }
  get_cvgsta(job_id);
  auto& progress = job_progress.at(job_id);
  error = progress.error;
  return &progress.parser.data();
}
