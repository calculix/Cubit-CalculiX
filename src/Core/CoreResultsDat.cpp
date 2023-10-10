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
      
      // if whitespace line, get next line until not a whitespace line
      while(this->is_whitespace(datline))
      {
        std::getline(dat,datline);
        ++currentline;
        if (dat.eof())
        {
          break;
        }
      }

      dat_array = this->split_line(datline);

      //first lets check if the mode is still valid
      this->check_mode(dat_array);

      if (dat.eof())
      {
        break;
      }else if (current_read_mode == 1)
      {
        //this->read_header(dat_array);
      } else if (current_read_mode == 2)
      {
      //  this->read_node(dat_array);
      } else if (current_read_mode == 10)
      {
      //  Buckling, skip block
      }
      
      if (!this->is_number(dat_array[0]))
      {
        log="";
        for (size_t i = 0; i < dat_array.size(); i++)
        {
          log.append(std::to_string(current_read_mode) + "#");
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
  // insert whitespaces after "set" if there is none
  beginSpace = line.find("set");
  if (beginSpace != std::string::npos)
  {
    size_t range = 3;
    if (line[beginSpace+3] != ' ')
    {
      line.replace(beginSpace,range, "set ");
    }
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

bool CoreResultsDat::is_whitespace(std::string line)
{
  size_t strBegin = line.find_first_not_of(" \t\r\f\v\n");
  if (strBegin == std::string::npos)
  {
    return true;
  }
  return false;
}

bool CoreResultsDat::check_mode(std::vector<std::string> line)
{
  if ((line[0] == "B")&&(line[1] == " ")&&(line[2] == "U")) // Buckling!
  {
    current_read_mode = 10;
  } else if (this->is_number(line[0]))
  {
    if (current_read_mode != 10) // if buckling block, skip
    {
      current_read_mode = 2; // read data lines
    }
  } else if (!this->is_number(line[0]))
  {
    current_read_mode = 1; // read header line
  }

  return true;
}

bool CoreResultsDat::read_header(std::vector<std::string> line)
{

  int search = -1;
  for (size_t i = 0; i < line.size(); i++)
  {
    if (line[i]=="set")
    {
      search = i;
    }
  } 
  std::string block_set = line[search + 1];
  double total_time = std::stod(line[line.size()-1]);
  total_times.push_back(total_time);
  current_total_time_id = total_times.size()-1;

  std::string block_type = "";
  for (size_t i = 0; i < search-1; i++)
  {
    block_type.append(line[i]);
  }

  std::vector<std::string> tmp_result_block_components;
  // extract components if possible
  size_t beginSpace = block_type.find_first_of("(");
  if (beginSpace != std::string::npos)
  {  
    std::string str_comp = "";
    size_t endSpace = block_type.find_first_of(")", beginSpace);
    size_t range = endSpace - beginSpace;
    str_comp = block_type.substr(beginSpace + 1,endSpace - 1);
    block_type.replace(beginSpace,range, "");

    std::stringstream ss(str_comp);
    std::string str_temp;
    // split line with delimiter ","
    while (getline(ss,str_temp,','))
    {
      tmp_result_block_components.push_back(str_temp);
    }
  }else{
    tmp_result_block_components.push_back(block_type);
  }
  // write new set of component names
  result_block_components.push_back(tmp_result_block_components);


  std::vector<std::vector<double>> tmp_result_block_data;
  std::vector<std::vector<int>> tmp_result_block_c1_data;
  result_block_data.push_back(tmp_result_block_data);
  result_block_c1_data.push_back(tmp_result_block_c1_data);
    
  current_result_block = result_blocks.size()-1;
  int result_block_data_id = result_block_data.size()-1;
  int result_block_type_data_id = this->get_current_result_block_type(block_type);
  int result_block_set_data_id = this->get_current_result_block_set(block_set);
  
  result_blocks.push_back({current_result_block,current_total_time_id,result_block_type_data_id,result_block_set_data_id,result_block_data_id});
 
  return true;
}

int CoreResultsDat::get_current_result_block_type(std::string result_type)
{

  for (size_t i = 0; i < result_block_type.size(); i++)
  {
    if (result_block_type[i]==result_type)
    {
      return i;
    }
  }
  result_block_type.push_back(result_type);
  int data_id = result_block_type.size()-1;
  
  return data_id;
}

int CoreResultsDat::get_current_result_block_set(std::string result_set)
{

  for (size_t i = 0; i < result_block_set.size(); i++)
  {
    if (result_block_set[i]==result_set)
    {
      return i;
    }
  }
  result_block_set.push_back(result_set);
  int data_id = result_block_set.size()-1;
  
  return data_id;
}