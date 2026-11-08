#ifndef COREJOBS_HPP
#define COREJOBS_HPP

#include "JobsSettings.hpp"
#include "JobProgress.hpp"
#include "Framework/Interfaces/JobMonitorProviderInterface.hpp"
#include <map>

#include <vector>
#include <string>
#include <set>
#ifdef WIN32
  #include <windows.h>
  #include <thread>
#endif
class CubitProcess;
class ProgressTool;

class CoreJobs
{

public:
  bool get_settings(int id, JobsSettings& settings, std::string& error) const;
  std::vector<int> get_job_ids() const;
  bool get_monitor_snapshot(std::vector<JobMonitorEntry>& output, std::string& error) const;
  void capture_monitor_execution(int job_id, std::uint64_t pid);
  void clear_monitor_executions() noexcept;
  CoreJobs();
  ~CoreJobs();

  std::vector<std::vector<std::string>> get_tree_data() const;

  std::vector<std::vector<std::string>> jobs_data;
  // jobs_data[0][0] job_id
  // jobs_data[0][1] name
  // jobs_data[0][2] filepath
  // jobs_data[0][3] status -1 no process, 1 process running, 2 process finished, 3 process killed, 4 process finished with errors, 5 process running currently waiting
  // jobs_data[0][4] process id
  // jobs_data[0][5] Output Console data id
  // jobs_data[0][6] VTK conversion complete: -1 false, 1 true
  // jobs_data[0][7] .cvg data id
  // jobs_data[0][8] .sta data id
  // jobs_data[0][9] auto conversion 1, no conversion -1

  std::vector<std::vector<std::string>> output_console;
  std::vector<std::vector<std::string>> cvg;
  std::vector<std::vector<std::string>> sta;

  #ifdef WIN32
    std::map<int, HANDLE> running_process_handles;
    std::vector<HANDLE> ProcessPipe;
    std::vector<std::vector<int>> PPTID; //links the process PID, pipe PID and ThreadID 
    std::vector<int> PipePID; //pipe PID
    std::vector<std::thread> PipeThreads; // threads for reading the pipes
    std::vector<bool> PipeThreadsRun; // bools to keep the threads running
  #else
    std::vector<CubitProcess> CubitProcessHandler;
  #endif
  
  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the jobs
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_job(std::vector<std::string> options); // adds new job
  bool create_job(const std::vector<std::string>& options, int& created_job_id); // needed for topology job
  bool modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker); // modify a job
  bool delete_job(int job_id); // deletes job from jobs_data
  bool add_job(int job_id, std::string name, std::string filepath); // adds new job to jobs_data
  bool run_job(int job_id, int option); // runs job
  bool wait_job(int job_id); // waits until the jobs exits
  bool kill_job(int job_id); // kill the jobs exits
  bool check_jobs(); // checks for changes of job processes
  bool get_cvgsta(int job_id); // gets filedata of .cvg and .sta
  void clear_job_progress();
  const CalculiXJobProgress::Data* get_job_progress(int job_id, std::string& error);
  int  get_jobs_data_id_from_job_id(int job_id); // searches for the job_id in the jobs_data and returns the indices or -1 if it fails
  #ifdef WIN32
    int  get_ProcessPipe_data_id_from_PipePID(int PipePID); // searches for the Pipe Handle data id in the Processpipe and returns the indices or -1 if it fails
    int  get_PipePID_from_ProcessPID(int ProcessPID); // searches for the Pipe PID in the PPTID and returns the PipePID or -1 if it fails
    int  get_ThreadID_from_ProcessPID(int ProcessPID); // searches for the Thread ID in the PPTID and returns the Thread ID or -1 if it fails
    void read_pipe(int job_id,int bool_data_id); // make a reading thread for the jobs pipe
    void kill_pipe(int job_id); // kill the reading thread for the jobs pipe
  #else
    int  get_CubitProcessHandler_data_id_from_process_id(int process_id); // searches for the CubitProcessHandler_id in the CubitProcessHandler and returns the indices or -1 if it fails
  #endif
  bool set_job_conversion(int job_id, int conversion); // sets the in-memory VTK conversion value for the job
  std::vector<std::string> get_job_data(int job_id);
  std::vector<std::string> get_job_console_output(int job_id);
  std::vector<std::string> get_job_cvg(int job_id);
  std::vector<std::string> get_job_sta(int job_id);
  int get_job_status(int job_id); //status -2 no job -1 no process, 1 process running, 2 process finished, 3 process killed, 4 process finished with errors, 5 process running currently waiting
  bool is_whitespace(std::string line); // check if line consists only of whitespace
  std::string print_data(); // prints out the jobs_data

  ProgressTool *progressbar; // progressbar

private:
  std::map<int, CalculiXJobProgress::State> job_progress;
  struct MonitorExecution
  {
    std::string id;
    std::optional<ProcessIdentity> process;
  };
  std::map<int, MonitorExecution> monitorExecutions;
  bool complete_job_results(int job_id);
  bool results_ready();
  bool completing_results = false;
  std::set<int> pending_result_jobs;
};

#endif // COREJOBS_HPP
