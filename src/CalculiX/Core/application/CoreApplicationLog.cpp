#include "Core/application/CoreApplicationLog.hpp"

#include <chrono>
#include <ctime>
#include <fstream>

bool CoreApplicationLog::append(const std::string& message)
{
  std::ofstream file("CalculiXCore_logfile", std::ios_base::app);

  if (!file.is_open())
  {
    return false;
  }

  const auto timestamp = std::chrono::system_clock::now();
  const std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
  const char* text = std::ctime(&time);

  if (!text)
  {
    return false;
  }

  file << text << message << "\n";
  file.close();
  return !file.fail();
}
