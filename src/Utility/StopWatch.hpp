#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP

#include <vector>
#include <string>
#include <chrono>

class StopWatch
{

public:
  StopWatch();
  ~StopWatch();
  
  std::chrono::time_point<std::chrono::high_resolution_clock> t_runtime_start = std::chrono::high_resolution_clock::now();
  std::chrono::time_point<std::chrono::high_resolution_clock> t_runtime_last = std::chrono::high_resolution_clock::now();

  bool tick(std::string label); // prints out a label, the time difference from the last label print and the total runtime
  bool total(std::string label); // prints out a label and the total runtime
};

#endif // STOPWATCH_HPP