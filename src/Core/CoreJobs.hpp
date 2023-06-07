#ifndef COREJOBS_HPP
#define COREJOBS_HPP

class CalculiXCoreInterface;
class CubitProcess;

class CoreJobs
{

public:
  CoreJobs();
  ~CoreJobs();

  std::vector<std::vector<std::string>> jobs_data;
  // jobs_data[0][0] job_id
  // jobs_data[0][1] name
  // jobs_data[0][2] filepath
  // jobs_data[0][3] status -1 no process, 1 process running, 2 process finished, 3 process killed
  // jobs_data[0][4] process id
  // jobs_data[0][5] Output

  std::vector<CubitProcess> CubitProcessHandler;

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the jobs
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_job(std::vector<std::string> options); // adds new job
  bool modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a job
  bool delete_job(int job_id); // deletes job from jobs_data
  bool add_job(int job_id, std::string name, std::string filepath); // adds new job to jobs_data
  bool run_job(int job_id); // runs job
  bool wait_job(int job_id); // waits till the jobs exits
  bool check_jobs(); // checks for changes of job processes
  bool result_ccx2paraview_job(int job_id); // converts the result with ccx2paraview
  bool result_cgx_job(int job_id); // opens the results with cgx
  bool result_paraview_job(int job_id); // opens the results with paraview
  int  get_jobs_data_id_from_job_id(int job_id); // searches for the job_id in the jobs_data and returns the indices or -1 if it fails
  int  get_CubitProcessHandler_data_id_from_process_id(int process_id); // searches for the CubitProcessHandler_id in the CubitProcessHandler and returns the indices or -1 if it fails
  std::string print_data(); // prints out the jobs_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREJOBS_HPP