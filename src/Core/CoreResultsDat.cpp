#include "CoreResultsDat.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"

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

bool CoreResultsDat::clear()
{
  result_blocks.clear();
  total_times.clear();
  result_block_components.clear();
  result_block_type.clear();
  result_block_set.clear();
  result_block_data.clear();
  result_block_c1_data.clear();

  return true;
}

bool CoreResultsDat::check_initialized()
{
  return is_initialized;
}

bool CoreResultsDat::read()
{
  int maxlines = 0;
  int currentline = 0;
  ProgressTool progressbar;
  std::string log;
  log = "reading results " + filepath + " for Job ID " + std::to_string(job_id) + " \n";
  PRINT_INFO("%s", log.c_str());

  std::string datline = "";
  std::vector<std::string> dat_array;
  
  std::ifstream dat;
  dat.open(this->filepath);

  // clear all data before reading and reset read mode
  this->clear();
  current_read_mode = 0;

  if (dat.is_open())
  {
    // scan file for number of lines
    progressbar.start(0,100,"Scanning Results dat");
    while (std::getline(dat,datline))
    { 
      ++maxlines;
      if(dat.eof()){
        break;
      }
    }
    progressbar.end();
    dat.close();
    dat.open(this->filepath);

    progressbar.start(0,100,"Reading Results DAT");
    auto t_start = std::chrono::high_resolution_clock::now();
    
    log = "";
    while (dat)
    {
      std::getline(dat,datline);
      ++currentline;
  
      const auto t_end = std::chrono::high_resolution_clock::now();
      int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
      if (duration > 500)
      {
        progressbar.percent(double(currentline)/double(maxlines));
        progressbar.check_interrupt();
        t_start = std::chrono::high_resolution_clock::now();
      }
      
      dat_array = this->split_line(datline);

      // first lets check if the mode is still valid
      /*this->check_mode(dat_array);

      if (current_read_mode == 0)
      {
        this->read_header(dat_array);
      } else if (current_read_mode == 1)
      {
        this->read_node(dat_array);
      } else if (current_read_mode == 2)
      {
        this->read_element(dat_array);
      } else if (current_read_mode == 3)
      {
        this->read_parameter_header(dat_array);
      } else if (current_read_mode == 4)
      {
        this->read_nodal_result_block(dat_array);
      } else */if ((current_read_mode == 9999)||(dat.eof()))
      {
        break;
      }
      
      if (!this->is_number(dat_array[0]))
      {
        log="";
        for (size_t i = 0; i < dat_array.size(); i++)
        {
          //log.append(std::to_string(current_read_mode) + "#");
          log.append(dat_array[i] + " ");
        }
        log.append("\n");
        PRINT_INFO("%s", log.c_str());
      }
      
      //ccx_iface->log_str(log);
    }
  }
  dat.close();
  progressbar.end();
  //PRINT_INFO("%s", log.c_str());
  //print_data();
  return true;
}

std::vector<std::string> CoreResultsDat::split_line(std::string line)
{
  std::vector<std::string> str_array;
  std::string str_temp;

  // trim whitespaces
  size_t strBegin = line.find_first_not_of(" \t\r\f\v\n");
  if (strBegin != std::string::npos)
  {
    size_t strEnd = line.find_last_not_of(" \t\r\f\v\n");
    size_t strRange = strEnd - strBegin + 1;
    line = line.substr(strBegin, strRange);
  }
  // reduce whitespaces
  size_t beginSpace = line.find_first_of(" \t\r\f\v\n");
  while (beginSpace != std::string::npos)
  {
    size_t endSpace = line.find_first_not_of(" \t\r\f\v\n", beginSpace);
    size_t range = endSpace - beginSpace;

    line.replace(beginSpace,range, " ");
    size_t newStart = beginSpace + 1;
    beginSpace = line.find_first_of(" \t\r\f\v\n",newStart);
  }
  // insert whitespaces before "-" whitespaces but not "E-"
  beginSpace = line.find_first_of("-");
  while (beginSpace != std::string::npos)
  {
    size_t range = 1;
    if ((line[beginSpace-1] != 'E') && (beginSpace !=0))
    {
      line.replace(beginSpace,range, " -");
    }
    size_t newStart = beginSpace + 2;
    beginSpace = line.find_first_of("-",newStart);
  }

  std::stringstream ss(line);

  // split line with delimiter " "
  while (getline(ss,str_temp,' '))
  {
    str_array.push_back(str_temp);
  }
  
  return str_array;
}

bool CoreResultsDat::is_number(std::string number)
{
  for (size_t i = 0; i < number.length(); i++)
  {
    if (!std::isdigit(number[i]))
    {
      return false;
    }
  }
  return true;
}