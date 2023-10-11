#include "CoreResultsVtkWriter.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>

CoreResultsVtkWriter::CoreResultsVtkWriter()
{}

CoreResultsVtkWriter::~CoreResultsVtkWriter()
{}

bool CoreResultsVtkWriter::init(int job_id)
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    this->job_id = job_id;

    std::vector<std::string> job_data = ccx_iface->get_job_data(job_id);
    this->filepath = job_data[1] + ".vtk";

    is_initialized = true;  
    return true;
  }
}

bool CoreResultsVtkWriter::reset()
{
  init(-1);
  return true;
}

bool CoreResultsVtkWriter::clear()
{
  
  return true;
}

bool CoreResultsVtkWriter::check_initialized()
{
  return is_initialized;
}