#ifndef CALCULIXJOBSINTERFACE_HPP
#define CALCULIXJOBSINTERFACE_HPP

#include <string>
#include <vector>

class CalculiXJobsInterface
{
public:
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
  bool set_job_conversion(int job_id, int conversion); // sets the paraview conversion value for the job
  std::vector<std::string> get_job_data(int job_id);
  std::vector<std::string> get_job_console_output(int job_id);
  std::vector<std::string> get_job_cvg(int job_id);
  std::vector<std::string> get_job_sta(int job_id);
  int get_job_status(int job_id);
  std::vector<std::vector<std::string>> get_jobs_tree_data(); // gets the data from core to build the tree
};

#endif // CALCULIXJOBSINTERFACE_HPP
