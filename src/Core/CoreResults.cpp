#include "CoreResults.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

#include <fstream>
#include <iostream>
#include "loadUserOptions.hpp"

CoreResults::CoreResults()
{}

CoreResults::~CoreResults()
{}

bool CoreResults::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    is_initialized = true;  
    return true;
  }
}

bool CoreResults::update()
{ 
  return true;
}

bool CoreResults::reset()
{
  results_data.clear();
  frd_data.clear();

  init();
  return true;
}

bool CoreResults::check_initialized()
{
  return is_initialized;
}

bool CoreResults::create_result(int job_id)
{
  int result_id;
  int result_last;
  
  if (results_data.size()==0)
  {
    result_id = 1;
  }
  else
  {
    result_last = results_data.size() - 1;
    result_id = std::stoi(results_data[result_last][0]) + 1;
  }

  this->add_job(result_id, job_id, frd_data.size());
  return true;
}

bool CoreResults::add_result(int result_id, int job_id, int frd_id)
{
  std::vector<std::int> v = {result_id, job_id, frd_id};
  jobs_data.push_back(v);
  return true;
}

bool CoreResults::delete_result(int job_id)
{
  int jobs_data_id = get_jobs_data_id_from_job_id(job_id);
  if (jobs_data_id == -1)
  {
    return false;
  } else {
    jobs_data.erase(jobs_data.begin() + jobs_data_id);
    return true;
  }
}

int CoreResults::get_results_data_id_from_job_id(int job_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < jobs_data.size(); i++)
  {
    if (jobs_data[i][0]==std::to_string(job_id))
    {
      return_int = i;
    }  
  }
  return return_int;
}

std::string CoreResults::print_data()
{
  std::string str_return;
  str_return = "\n CoreJobs jobs_data: \n";
  str_return.append("job_id, name, filepath, status, process id, output \n");

  for (size_t i = 0; i < jobs_data.size(); i++)
  {
    str_return.append(jobs_data[i][0] + " " + jobs_data[i][1] + " " + jobs_data[i][2] + " " + jobs_data[i][3] + " " + jobs_data[i][4] + " \n");
    str_return.append(jobs_data[i][5] + " \n");
    str_return.append(jobs_data[i][6] + " \n");
    str_return.append(jobs_data[i][7] + " \n");
    str_return.append(jobs_data[i][8] + " \n");
  }

  return str_return;
}