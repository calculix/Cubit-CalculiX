#include "CoreSimulation.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreSimulation::CoreSimulation()
{}

CoreSimulation::~CoreSimulation()
{}

bool CoreSimulation::init()
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

bool CoreSimulation::update()
{ 
  return true;
}

bool CoreSimulation::reset()
{  
  init();
  return true;
}

bool CoreSimulation::check_initialized()
{
  return is_initialized;
}

bool CoreSimulation::solve(std::string filepath)
{
  ccx_iface->log_str(filepath);
  
  return true;
}