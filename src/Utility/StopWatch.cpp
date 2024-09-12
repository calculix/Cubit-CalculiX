#include "StopWatch.hpp"
#include "CubitMessage.hpp"

StopWatch::StopWatch()
{}

StopWatch::~StopWatch()
{}

bool StopWatch::tick(std::string label)
{
  const auto t_runtime_end = std::chrono::high_resolution_clock::now();
  int t_runtime_duration = std::chrono::duration<double, std::milli>(t_runtime_end - t_runtime_start).count();
  int t_runtime_round_duration = std::chrono::duration<double, std::milli>(t_runtime_end - t_runtime_last).count();
  
  t_runtime_last = std::chrono::high_resolution_clock::now();
  
  std::string log;
  log =  label + " " + std::to_string(t_runtime_duration) + " " + std::to_string(t_runtime_round_duration) + "\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}

bool StopWatch::total(std::string label)
{
  const auto t_runtime_end = std::chrono::high_resolution_clock::now();
  int t_runtime_duration = std::chrono::duration<double, std::milli>(t_runtime_end - t_runtime_start).count();
    
  std::string log;
  log =  label + " " + std::to_string(t_runtime_duration) + "\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}