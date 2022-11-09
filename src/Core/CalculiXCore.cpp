#include "CalculiXCore.hpp"
#include <fstream>
#include <chrono>
#include <ctime>

CalculiXCore::CalculiXCore()
{
  print_to_log("CalculiXCore Initialization!");
  init();
}

CalculiXCore::~CalculiXCore()
{}

bool CalculiXCore::print_to_log(std::string str_log)
{
  std::ofstream output_file;
  output_file.open("CalculiXCore_logfile", std::ios_base::app);
  auto timestamp = std::chrono::system_clock::now();
  std::time_t time_t_timestamp = std::chrono::system_clock::to_time_t(timestamp);
  output_file << std::ctime(&time_t_timestamp) << str_log << "\n";
  output_file.close();
  return true;
}

bool CalculiXCore::init()
{
  cb.init();
  
  return true;
}

std::string CalculiXCore::print_data()
{
  std::string str_return = "";
  str_return.append(cb.print_data());
  return str_return;
}
