#include "CoreResultsDat.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

#include <fstream>
#include <iostream>
#include "loadUserOptions.hpp"

CoreResultsDat::CoreResultsDat()
{}

CoreResultsDat::~CoreResultsDat()
{}

bool CoreResultsDat::init(int job_id)
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    this->job_id = job_id;
    
    std::vector<std::string> job_data = ccx_iface->get_job_data(job_id);
    this->filepath = job_data[1] + ".dat";

    is_initialized = true;
    return true;
    return true;
  }
}

bool CoreResultsDat::reset()
{
  init(-1);
  return true;
}

bool CoreResultsDat::check_initialized()
{
  return is_initialized;
}

bool CoreResultsDat::read()
{
  std::string log;
  log = "reading results " + filepath + " for Job ID " + std::to_string(job_id) + " \n";
  PRINT_INFO("%s", log.c_str());
  return true;
}