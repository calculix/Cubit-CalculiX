#ifndef CORERESULTS_HPP
#define CORERESULTS_HPP

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <atomic>
#include "Framework/Results/VtkResultsData.hpp"
class CoreResultsFrd;
class CoreResultsDat;
class CoreResultsProject;
struct CoreResultsVtkOptions;
struct CoreResultsVtkPlan;

class CoreResults
{

public:
  CoreResults();
  ~CoreResults();

  std::vector<std::vector<int>> results_data;
  // results_data[0][0] result_id
  // results_data[0][1] job_id
  // results_data[0][2] frd_id of frd reader writer object
  // results_data[0][3] dat_id of dat file reader object
  // frd and dat object should be initialized with job_id!

  std::vector<CoreResultsFrd> frd_data;
  // frd_data[0] frd reader writer object

  std::vector<CoreResultsDat> dat_data;
  // dat_data[0] dat file reader object

  bool is_initialized = false;

  bool init(); // initialize
  bool update(); // check for changes of the jobs
  bool reset(); // delete all data and initialize afterwards
  bool check_initialized(); // check if object is initialized
  bool create_result(int job_id); // adds new results data for a job
  bool create_frd_dat(int job_id); // creates frd and dat objects, needed for loading cub
  bool delete_result(int job_id); // deletes results from results_data and deletes frd object
  bool add_result(int result_id, int job_id, int frd_id, int dat_id);
  bool load_result(int job_id); // loads the results for the job if possible
  FeaResults::ConversionStatus get_vtk_status(int job_id) const;
  std::shared_ptr<const FeaResults::VtkResultsData> get_vtk_result(int job_id) const;
  bool prepare_vtk_plan(int job_id, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& plan);
  // Host-thread, synchronous. Callers must use these interfaces rather than
  // mutating the public legacy reader containers during a conversion.
  bool build_vtk_result(int job_id, const CoreResultsVtkOptions& options);
  bool export_vtk_result(int job_id, const std::string& destination = "");
  std::string get_vtk_export_path(int job_id) const;
  bool cancel_vtk_build();
  bool is_vtk_build_active() const;
  int convert_result(int job_id, std::vector<int> options, std::vector<int> block_ids, std::vector<int> nodeset_ids, std::vector<int> sideset_ids); // convert the results for the job if possible
  bool project_result(int job_id,int step,int totalincrement,double scale); // project disp result for job for specific step or increment
  bool project_topology(int topology_id,int iteration_id = -1,double threshold = 0.5,bool preview = false);
  int  get_results_data_id_from_job_id(int job_id); // searches for the job_id in the results_data and returns the indices or -1 if it fails
  int  get_frd_data_id_from_job_id(int job_id); // searches for the frd_id in the frd_data and returns the indices or -1 if it fails
  int  get_dat_data_id_from_job_id(int job_id); // searches for the dat_id in the dat_data and returns the indices or -1 if it fails
  std::string print_data(); // prints out the results_data

private:
  friend class CoreResultsPersistence;
  bool can_change_readers() const;
  bool prepare_vtk_plan_impl(int job_id, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& plan);
  void clear_vtk_cache();
  bool vtk_build_active = false;
  std::atomic<bool> vtk_cancelled{false};
  // Converted data is transient. Published handles may outlive deletion/reload
  // of a job in this registry and must never borrow reader storage.
  std::map<int, FeaResults::ConversionStatus> vtk_status;
  std::map<int, std::shared_ptr<const FeaResults::VtkResultsData>> vtk_results;
  std::map<int, std::string> vtk_exports;

};

#endif // CORERESULTS_HPP
