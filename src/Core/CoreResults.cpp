#include "CoreResults.hpp"
#include "CoreResultsFrd.hpp"
#include "CoreResultsDat.hpp"
#include "CoreResultsVtkWriter.hpp"
#include "CoreResultsProject.hpp"
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
  CoreResultsFrd frd;
  CoreResultsDat dat;
  
  if (results_data.size()==0)
  {
    result_id = 1;
  }
  else
  {
    result_last = results_data.size() - 1;
    result_id = results_data[result_last][0] + 1;
  }

  this->add_result(result_id, job_id, job_id, job_id);
  // create frd and dat file reader writer object
  frd_data.push_back(frd);
  frd_data[frd_data.size()-1].init(job_id);
  dat_data.push_back(dat);
  dat_data[dat_data.size()-1].init(job_id);

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
  int frd_data_id = get_frd_data_id_from_job_id(job_id);
  int dat_data_id = get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return false;
  } else {
    frd_data.erase(frd_data.begin() + frd_data_id);
    dat_data.erase(dat_data.begin() + dat_data_id);
    results_data.erase(results_data.begin() + results_data_id);
    return true;
  }
}

bool CoreResults::load_result(int job_id)
{
  //std::string log;
  int results_data_id = get_results_data_id_from_job_id(job_id);
  int frd_data_id = get_frd_data_id_from_job_id(job_id);
  int dat_data_id = get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return false;
  } else {
    //log = "Loading results for Job ID " + std::to_string(results_data[results_data_id][1]) + " \n";
    //PRINT_INFO("%s", log.c_str());
    frd_data[frd_data_id].read();
    dat_data[dat_data_id].read();

    return true;
  }
}

int CoreResults::convert_result(int job_id)
{
  std::string log;
  int results_data_id = get_results_data_id_from_job_id(job_id);
  int frd_data_id = get_frd_data_id_from_job_id(job_id);
  int dat_data_id = get_dat_data_id_from_job_id(job_id);
  CoreResultsVtkWriter* vtkWriter = new CoreResultsVtkWriter();

  if (results_data_id == -1)
  {
    return -1;
  } else {
    log = "Converting results for Job ID " + std::to_string(results_data[results_data_id][1]) + " \n";
    PRINT_INFO("%s", log.c_str());
    vtkWriter->init(job_id,&frd_data[frd_data_id],&dat_data[dat_data_id]);
    vtkWriter->write();
    int write_mode = vtkWriter->write_mode;
    ccx_iface->set_job_conversion(job_id,write_mode); // used for manual conversion, twice is better than once

    delete vtkWriter;

    return write_mode;
  }
}

bool CoreResults::project_result(int job_id, int step, int totalincrement,double scale)
{
  std::string log;
  int results_data_id = get_results_data_id_from_job_id(job_id);
  int frd_data_id = get_frd_data_id_from_job_id(job_id);
  int dat_data_id = get_dat_data_id_from_job_id(job_id);
  CoreResultsProject* projector = new CoreResultsProject();

  if (results_data_id == -1)
  {
    return false;
  } else {
    if (step!=-1)
    {
      log = "Project results for Job ID " + std::to_string(results_data[results_data_id][1]) + " for Step " + std::to_string(step) + " \n";
    }else{
      log = "Project results for Job ID " + std::to_string(results_data[results_data_id][1]) + " for total increment " + std::to_string(totalincrement) + " \n";
    }
    PRINT_INFO("%s", log.c_str());
    projector->init(&frd_data[frd_data_id]);
    if (!projector->project(step,totalincrement,scale))
    {
      delete projector;
      return false;
    }

    delete projector;

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

int CoreResults::get_frd_data_id_from_job_id(int job_id)
{ 
  int return_int = -1;
  
  for (size_t i = 0; i < frd_data.size(); i++)
  {
    if (job_id == frd_data[i].job_id)
    {
      return_int = i;
    }  
  }
  
  return return_int;
}

int CoreResults::get_dat_data_id_from_job_id(int job_id)
{ 
  int return_int = -1;
  
  for (size_t i = 0; i < dat_data.size(); i++)
  {
    if (job_id == dat_data[i].job_id)
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