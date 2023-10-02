#ifndef CORERESULTS_HPP
#define CORERESULTS_HPP

class CalculiXCoreInterface;

class CoreResults
{

public:
  CoreResults();
  ~CoreResults();

  std::vector<std::vector<int>> results_data;
  // jobs_data[0][0] result_id
  // jobs_data[0][1] job_id
  // jobs_data[0][2] frd_id data id of frd reader writer object

  std::vector<std::string> frd_data;
  // jobs_data[0] frd reader writer object

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the jobs
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_result(int job_id); // adds new results data for a job
  bool delete_result(int job_id); // deletes results from results_data and deletes frd object
  bool add_result(int result_id, int job_id, int frd_id)
  int  get_results_data_id_from_job_id(int job_id); // searches for the job_id in the results_data and returns the indices or -1 if it fails
  std::string print_data(); // prints out the results_data

  CalculiXCoreInterface *ccx_iface;
};

#endif // CORERESULTS_HPP