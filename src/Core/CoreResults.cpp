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
  dat_data.clear();

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
    result_id = results_data[result_last][0] + 1;
  }

  this->add_result(result_id, job_id, frd_data.size(), dat_data.size());
  return true;
}

bool CoreResults::add_result(int result_id, int job_id, int frd_id, int dat_id)
{
  std::vector<int> v = {result_id, job_id, frd_id, dat_id};
  results_data.push_back(v);
  return true;
}

bool CoreResults::delete_result(int job_id)
{
  int results_data_id = get_results_data_id_from_job_id(job_id);
  if (results_data_id == -1)
  {
    return false;
  } else {
    results_data.erase(results_data.begin() + results_data_id);
    return true;
  }
}

bool CoreResults::load_result(int job_id)
{
  std::string log;
  int results_data_id = get_results_data_id_from_job_id(job_id);
  if (results_data_id == -1)
  {
    return false;
  } else {
    log = "Loading results for Job ID " + std::to_string(results_data[results_data_id][1]) + " \n";
    PRINT_INFO("%s", log.c_str());
    return true;
  }
}

int CoreResults::get_results_data_id_from_job_id(int job_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < results_data.size(); i++)
  {
    if (results_data[i][1]==job_id)
    {
      return_int = i;
    }  
  }
  return return_int;
}

std::string CoreResults::print_data()
{
  std::string str_return;
  str_return = "\n CoreResults results_data: \n";
  str_return.append("result_id, job_id, frd_id, dat_id \n");

  for (size_t i = 0; i < results_data.size(); i++)
  {
    str_return.append(std::to_string(results_data[i][0]) + " " + std::to_string(results_data[i][1]) + " " + std::to_string(results_data[i][2]) + " " + std::to_string(results_data[i][3]) +  " \n");
  }

  return str_return;
}