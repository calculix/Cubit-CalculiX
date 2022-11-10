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
  output_file << std::ctime(&time_t_timestamp) + str_log + "\n";
  output_file.close();
  return true;
}

bool CalculiXCore::init()
{
  cb.init();
  
  return true;
}

bool CalculiXCore::update()
{
  cb.update();
  
  print_to_log("UPDATE");
  print_to_log(print_data());

  return true;
}

bool CalculiXCore::reset()
{
  cb.reset();
  
  print_to_log("RESET");
  print_to_log(print_data());

  return true;
}

std::string CalculiXCore::print_data()
{
  std::string str_return = "";
  str_return.append(cb.print_data());
  return str_return;
}

std::vector<std::string> CalculiXCore::get_ccx_element_types()
{ 
  return cb.get_ccx_element_types();
}

bool CalculiXCore::set_ccx_element_type(int block_id, std::string ccx_element_type)
{ 
  int new_element_type_ccx;
  new_element_type_ccx = cb.get_ccx_element_type_id(ccx_element_type);

  if (cb.modify_block_element_type_ccx(block_id, new_element_type_ccx))
  {
    return true;
  } else {
    return false;
  }
}