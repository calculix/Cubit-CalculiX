#ifndef CALCULIXJOBSINTERFACE_HPP
#define CALCULIXJOBSINTERFACE_HPP

#include "JobsSettings.hpp"
#include "JobProgress.hpp"
#include "Framework/Interfaces/JobMonitorProviderInterface.hpp"

#include <string>
#include <vector>

class CalculiXJobsInterface
{
public:
  bool get_monitor_snapshot(std::vector<JobMonitorEntry>& output, std::string& error) const;
  bool get_job_settings(int id, JobsSettings& settings, std::string& error);
  bool create_job(std::vector<std::string> options); // adds a new job
  bool modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a job
  bool delete_job(int job_id); // delete job
  bool get_job_topology_context(int job_id, int& topology_id, int& iteration_id); // connection between job and topology
  bool prepare_job_input(int job_id, const std::string& filename);
  bool run_job(int job_id, int option); // runs job
  bool check_jobs(); // checks for changes of job processes
  bool wait_job(int job_id); // runs job
  bool kill_job(int job_id); // kills job
  bool is_checking_jobs() const;
  bool set_job_conversion(int job_id, int conversion); // sets the in-memory VTK conversion value for the job
  std::vector<std::string> get_job_data(int job_id);
  std::vector<std::string> get_job_console_output(int job_id);
  std::vector<std::string> get_job_cvg(int job_id);
  std::vector<std::string> get_job_sta(int job_id);
  // Borrowed until the next job mutation/query; consume on the host thread.
  // A nonempty error with non-null data means cached records remain available.
  const CalculiXJobProgress::Data* get_job_progress(int job_id, std::string& error);
  int get_job_status(int job_id);
  std::vector<std::vector<std::string>> get_jobs_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXJOBSINTERFACE_HPP
