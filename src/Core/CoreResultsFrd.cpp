#include "CoreResultsFrd.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"
#include "loadUserOptions.hpp"
#include "StopWatch.hpp"

#include <chrono>
#include <fstream>
#include <cmath>
#include <iostream>

CoreResultsFrd::CoreResultsFrd()
{}

CoreResultsFrd::~CoreResultsFrd()
{}

bool CoreResultsFrd::init(int job_id)
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    this->job_id = job_id;

    std::vector<std::string> job_data = ccx_iface->get_job_data(job_id);
    this->filepath = job_data[1] + ".frd";

    progressbar = new ProgressTool();

    is_initialized = true;  
    return true;
  }
}

bool CoreResultsFrd::reset()
{
  init(-1);
  return true;
}

bool CoreResultsFrd::clear()
{
  header.clear();
  materials.clear();
  nodes.clear();
  nodes_coords.clear();
  elements.clear();
  elements_connectivity.clear();
  
  result_blocks.clear();
  total_times.clear();
  result_block_components.clear();
  result_block_type.clear();
  result_block_data.clear();
  result_block_node_data.clear();
  
  keys.clear();

  return true;
}

bool CoreResultsFrd::check_initialized()
{
  return is_initialized;
}

bool CoreResultsFrd::read()
{
  bool success = false;
  if (ccx_uo.mConverterThreads > 1)
  {
    if (read_parallel())
    {
      success = true;
    }
  }else{
    if (read_single())
    {
      success = true;
    }
  }

  return success;
}

bool CoreResultsFrd::read_single()
{
  StopWatch StopWatch;
  int maxlines = 0;
  int currentline = 0;
  std::string log;
  log = "reading results " + filepath + " for Job ID " + std::to_string(job_id) + " \n";
  PRINT_INFO("%s", log.c_str());

  std::string frdline = "";
  std::vector<std::string> frd_array;
  
  std::ifstream frd;
  frd.open(this->filepath);

  // clear all data before reading and reset read mode
  this->clear();
  current_read_mode = 0;

  if (frd.is_open())
  {
    // scan file for number of lines
    progressbar->start(0,100,"Scanning Results FRD");
    while (std::getline(frd,frdline))
    { 
      ++maxlines;
      if((frd.eof())){
        break;
      }
    }
    frd.close();

    if (maxlines<10)
    {
      // must be no data in the frd
      return false;
    }

    std::vector<std::string> job_data = ccx_iface->get_job_data(job_id);
    //if process is still running quit
    if (job_data[3] == "1")
    {
      return false;
    }

    // check if last line has 9999 in it, if not then add, otherwise on error frd data it crashes
    if ((job_data[3] == "3")||(job_data[3] == "4")||(job_data[3] == "-1"))
    {
      frd.open(this->filepath);
      for (size_t i = 0; i < maxlines; i++)
      {
        std::getline(frd,frdline);
        if (i == maxlines-1)
        {
          frd.close();
          frd_array = this->split_line(frdline);
          if (frd_array[0] != "9999")
          {
            frd.close(); 
            std::ofstream log(this->filepath, std::ios_base::app | std::ios_base::out);
            log << "9999\n";
          }
        }
      }
    }

    progressbar->end();  
    
    frd.open(this->filepath);

    progressbar->start(0,100,"Reading Results FRD");
    auto t_start = std::chrono::high_resolution_clock::now();
    
    log = "";
    while (frd)
    {
      std::getline(frd,frdline);
      ++currentline;
  
      const auto t_end = std::chrono::high_resolution_clock::now();
      int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
      if (duration > 500)
      {
        progressbar->percent(double(currentline)/double(maxlines));
        progressbar->check_interrupt();
        t_start = std::chrono::high_resolution_clock::now();
      }
      
      frd_array = this->split_line(frdline);

      // first lets check if the mode is still valid
      this->check_mode(frd_array);
      if (current_read_mode == 0)
      {
        this->read_header(frd_array);
      } else if (current_read_mode == 1)
      {
        this->read_node(frd_array);
      } else if (current_read_mode == 2)
      {
        this->read_element(frd_array);
      } else if (current_read_mode == 3)
      {
        this->read_parameter_header(frd_array);
      } else if (current_read_mode == 4)
      {
        this->read_nodal_result_block(frd_array);
      } else if ((current_read_mode == 9999)||(frd.eof()))
      {
        break;
      }
      
      /*
      log="";
      for (size_t i = 0; i < frd_array.size(); i++)
      {
        //log.append(std::to_string(current_read_mode) + "#");
        log.append(frd_array[i] + " ");
      }
      log.append("\n");
      PRINT_INFO("%s", log.c_str());
      */
      //ccx_iface->log_str(log);
    }
  }
  frd.close();

  // sorting for faster search
  for (size_t i = 0; i < result_block_node_data.size(); i++)
  {
    std::vector<int> tmp_node_ids;
    std::vector<int> tmp_node_data_ids;

    for (size_t ii = 0; ii < result_block_node_data[i].size(); ii++)
    {
      tmp_node_ids.push_back(result_block_node_data[i][ii][0]);
      tmp_node_data_ids.push_back(result_block_node_data[i][ii][1]);
    }  
    auto p = sort_permutation(tmp_node_ids);
    this->apply_permutation(tmp_node_ids, p);
    this->apply_permutation(tmp_node_data_ids, p);
    sorted_node_ids.push_back(tmp_node_ids);
    sorted_node_data_ids.push_back(tmp_node_data_ids);
  }

  progressbar->end();
  //PRINT_INFO("%s", log.c_str());
  //print_data();

  StopWatch.total("Duration of reading FRD [ms]: ");

  return true;
}

bool CoreResultsFrd::read_parallel()
{
  StopWatch StopWatch;
  int max_threads = ccx_uo.mConverterThreads;
  std::vector<std::thread> ReadThreads; // vector to contain threads for reading frd
  
  int maxlines = 0;
  std::string log;
  log = "reading results " + filepath + " for Job ID " + std::to_string(job_id) + " \n";
  PRINT_INFO("%s", log.c_str());

  std::string frdline = "";
  std::vector<std::string> frd_array;
  
  std::ifstream frd;
  frd.open(this->filepath);

  // clear all data before reading and reset read mode
  this->clear();
  current_read_mode = 0;

  //StopWatch.tick("start");

  if (frd.is_open())
  {
    // scan file for number of lines
    progressbar->start(0,100,"Scanning Results FRD");
    while (std::getline(frd,frdline))
    { 
      ++maxlines;
      //this->key.push_back(get_key(frdline));
      if((frd.eof())){
        break;
      }
    }
    frd.close();
    //StopWatch.tick("check");

    if (maxlines<10)
    {
      // must be no data in the frd
      return false;
    }

    std::vector<std::string> job_data = ccx_iface->get_job_data(job_id);
    //if process is still running quit
    if (job_data[3] == "1")
    {
      return false;
    }

    // check if last line has 9999 in it, if not then add, otherwise on error frd data it crashes
    if ((job_data[3] == "3")||(job_data[3] == "4")||(job_data[3] == "-1"))
    {
      frd.open(this->filepath);
      for (size_t i = 0; i < maxlines; i++)
      {
        std::getline(frd,frdline);
        if (i == maxlines-1)
        {
          frd.close();
          frd_array = this->split_line(frdline);
          if (frd_array[0] != "9999")
          {
            frd.close(); 
            std::ofstream log(this->filepath, std::ios_base::app | std::ios_base::out);
            log << "9999\n";
          }
        }
      }
    }
    //StopWatch.tick("check last line");

    progressbar->end();  

    //get keys
    std::vector<std::string> tmp(maxlines);
    this->keys = tmp;
    
    int keys_per_thread = int(maxlines/max_threads);

    for (size_t i = 0; i < max_threads; i++)
    { 
      int start = 0;
      int end = 0;
      if (i==max_threads-1)
      {
        start = i*keys_per_thread;
        end = maxlines;
      }else{
        start = i*keys_per_thread;
        end = (i+1)*keys_per_thread-1;
      }
      ReadThreads.push_back(std::thread(&CoreResultsFrd::insert_key, this, start, end));
    }
    // wait till all threads are finished
    for (size_t i = 0; i < max_threads; i++)
    { 
      ReadThreads[i].join();
    }
    ReadThreads.clear();
    //StopWatch.tick("keys");

    //reading header!
    //progressbar->start(0,100,"Reading Results FRD");
    //auto t_start = std::chrono::high_resolution_clock::now();
    //StopWatch.tick("reading header");

    frd.open(this->filepath);
    while (frd)
    {
      std::getline(frd,frdline);
      frd_array = this->split_line(frdline);
      /*
      const auto t_end = std::chrono::high_resolution_clock::now();
      int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
      if (duration > 500)
      {
        progressbar->percent(double(currentline)/double(maxlines));
        progressbar->check_interrupt();
        t_start = std::chrono::high_resolution_clock::now();
      }
      */
      this->check_mode(frd_array);
      
      if (current_read_mode == 0)
      {
        this->read_header(frd_array);
      } else if (current_read_mode == 1)
      {
        break;
      } else if ((current_read_mode == 9999)||(frd.eof()))
      {
        break;
      }
    }
    frd.close();
    
    // reading nodes
    progressbar->start(0,100,"Reading Nodes from FRD");
    //StopWatch.tick("Reading nodes");
    std::vector<int> node_range = this->get_node_range();
    int number_of_nodes = node_range[1]-node_range[0] + 1;
    //StopWatch.tick("get range");

    if (number_of_nodes < max_threads)
    {
      log = "More threads reading available than nodes in frd. Reduce number of threads for the converter!\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    int nodes_per_thread = int(number_of_nodes/max_threads);

    for (size_t i = 0; i < number_of_nodes; i++)
    {
      nodes.push_back({});
      nodes_coords.push_back({});
    }  
    //StopWatch.tick("dummy vectors");

    for (size_t i = 0; i < max_threads; i++)
    { 
      int start = 0;
      int end = 0;
      int data_start = 0;
      if (i==max_threads-1)
      {
        start = node_range[0]+i*nodes_per_thread;
        data_start = i*nodes_per_thread;
        end = node_range[1];
      }else{
        start = node_range[0]+i*nodes_per_thread;
        data_start = i*nodes_per_thread;
        end = node_range[0]+(i+1)*nodes_per_thread-1;
      }
      ReadThreads.push_back(std::thread(&CoreResultsFrd::read_nodes_thread, this, start, end, data_start));
    }

    // wait till all threads are finished
    for (size_t i = 0; i < max_threads; i++)
    { 
      ReadThreads[i].join();
    }
    ReadThreads.clear();
    log = "Reading nodes finished!\n";
    
    PRINT_INFO("%s", log.c_str());

    // reading elements
    progressbar->start(0,100,"Reading Elements from FRD");
    //StopWatch.tick("Reading elements");
    std::vector<int> element_range = this->get_element_range();
    int number_of_lines = element_range[1]-element_range[0] + 1;
    int number_of_elements = element_range[2];
    //StopWatch.tick("elements range");

    if (number_of_lines < max_threads*2)
    {
      log = "More threads reading available than elements in frd. Reduce number of threads for the converter!\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    int lines_per_thread = int(number_of_lines/max_threads);
    std::vector<std::vector<int>> thread_ranges;
    
    for (size_t i = 0; i < max_threads; i++)
    {
      int start = 0;
      int end = 0;
      int data_start = 0;
      if (i==max_threads-1)
      {
        start = element_range[0]+i*lines_per_thread;
        data_start = i*lines_per_thread;
        end = element_range[1];
      }else{
        start = element_range[0]+i*lines_per_thread;
        data_start = i*lines_per_thread;
        end = element_range[0]+(i+1)*lines_per_thread-1;
      }
      std::vector<int> range = {start,end,data_start};
      thread_ranges.push_back(range);
    }

    for (size_t i = 0; i < max_threads; i++)
    {
      if (i!=max_threads-1)
      {
        if (!check_key("-1",thread_ranges[i][1] + 1))
        {
          // if next line isn't the next element move range
          while (!check_key("-1",thread_ranges[i][1] + 1))
          {
            thread_ranges[i][1] = thread_ranges[i][1] + 1;
            thread_ranges[i+1][0] = thread_ranges[i+1][0] + 1;
          }
        }
      }
      //log = "range no " + std::to_string(i) + "-" + std::to_string(thread_ranges[i][0]) + "-" + std::to_string(thread_ranges[i][1]) +  " \n";
      //PRINT_INFO("%s", log.c_str());
    }
    //StopWatch.tick("check range");

    this->set_element_range_data_start(thread_ranges);
    
    //StopWatch.tick("set range");

    for (size_t i = 0; i < number_of_elements; i++)
    {
      elements.push_back({});
      elements_connectivity.push_back({});
    }
    //StopWatch.tick("dummy vector");
            
    for (size_t i = 0; i < max_threads; i++)
    { 
      ReadThreads.push_back(std::thread(&CoreResultsFrd::read_elements_thread, this, thread_ranges[i][0], thread_ranges[i][1], thread_ranges[i][2]));
    }
    // wait till all threads are finished
    for (size_t i = 0; i < max_threads; i++)
    { 
      ReadThreads[i].join();
    }
    ReadThreads.clear();
    log = "Reading elements finished!\n";
    //StopWatch.tick("element finished");
    PRINT_INFO("%s", log.c_str());

    // reading result blocks
    current_read_mode = 3; // start with results blocks

    frd.open(this->filepath);

    progressbar->start(0,100,"Reading Headers of Result Blocks FRD");
    auto t_start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::vector<int>> result_block_range;

    log = "";
    int currentline = -1;
    int last_element_line = thread_ranges[max_threads-1][1];
    bool new_result_block = true;
    while (frd)
    {
      std::getline(frd,frdline);
      ++currentline;
      if (currentline > last_element_line)
      {
        const auto t_end = std::chrono::high_resolution_clock::now();
        int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        if (duration > 500)
        {
          progressbar->percent(double(currentline)/double(maxlines));
          progressbar->check_interrupt();
          t_start = std::chrono::high_resolution_clock::now();
        }
        // first lets check if the mode is still valid
        //frd_array = this->split_line(frdline);
        //this->check_mode(frd_array);
        this->check_mode_by_key(this->keys[currentline]);
        
        if (current_read_mode == 3)
        {
          frd_array = this->split_line(frdline);
          this->read_parameter_header(frd_array);
          new_result_block = true;
        } else if (current_read_mode == 4)
        {
          frd_array = this->split_line(frdline);
          if (frd_array[0] == "-5")
          {  
            this->read_nodal_result_block(frd_array);
          }else{
            if (new_result_block)
            {
              new_result_block = false;
              result_block_range.push_back({currentline,currentline});
              this->read_nodal_result_block_add_components(frd_array);
            }
            result_block_range[result_block_range.size()-1][1] = currentline;
          }
        } else if ((current_read_mode == 9999)||(frd.eof()))
        {
          break;
        }
      }
    }
    /*
    for (size_t i = 0; i < result_block_range.size(); i++)
    {
      log = "range no " + std::to_string(i) + "-" + std::to_string(result_block_range[i][0]) + "-" + std::to_string(result_block_range[i][1]) +  " \n";
      PRINT_INFO("%s", log.c_str());
    }
    */

    int loop_c = 0;
    int number_of_result_blocks = result_block_range.size();
    int max_number_of_result_blocks = result_block_range.size();
    
    progressbar->start(0,100,"Reading Result Blocks Data FRD");
    t_start = std::chrono::high_resolution_clock::now();

    while (number_of_result_blocks > 0)
    {
      const auto t_end = std::chrono::high_resolution_clock::now();
      int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
      if (duration > 500)
      {
        progressbar->percent(double(1-number_of_result_blocks)/double(max_number_of_result_blocks));
        progressbar->check_interrupt();
        t_start = std::chrono::high_resolution_clock::now();
      }

      if (number_of_result_blocks > max_threads)
      {
        for (size_t i = 0; i < max_threads; i++)
        { 
          ReadThreads.push_back(std::thread(&CoreResultsFrd::read_nodal_result_block_thread, this,int(loop_c*max_threads+i), result_block_range[loop_c*max_threads+i][0], result_block_range[loop_c*max_threads+i][1]));
        }
      }else{
        for (size_t i = 0; i < number_of_result_blocks; i++)
        { 
          ReadThreads.push_back(std::thread(&CoreResultsFrd::read_nodal_result_block_thread, this,int(loop_c*max_threads+i), result_block_range[loop_c*max_threads+i][0], result_block_range[loop_c*max_threads+i][1]));
        }
      }
      // wait till all threads are finished
      for (size_t i = 0; i < ReadThreads.size(); i++)
      { 
        ReadThreads[i].join();
      }
      number_of_result_blocks = number_of_result_blocks - ReadThreads.size();
      ++loop_c;
      ReadThreads.clear();
    }

  }
  frd.close();

  // sorting for faster search
  int loop_c = 0;
  int number_of_result_blocks = result_block_node_data.size();
  for (size_t i = 0; i < result_block_node_data.size(); i++)
  {
    std::vector<int> tmp_node_ids;
    std::vector<int> tmp_node_data_ids;
    sorted_node_ids.push_back(tmp_node_ids);
    sorted_node_data_ids.push_back(tmp_node_data_ids);
  }

  while (number_of_result_blocks > 0)
  {
    if (number_of_result_blocks > max_threads)
    {
      for (size_t i = 0; i < max_threads; i++)
      { 
        ReadThreads.push_back(std::thread(&CoreResultsFrd::sort_result_block_node_data, this,loop_c*max_threads+i));
      }
    }else{
      for (size_t i = 0; i < number_of_result_blocks; i++)
      { 
        ReadThreads.push_back(std::thread(&CoreResultsFrd::sort_result_block_node_data, this,loop_c*max_threads+i));
      }
    }
    // wait till all threads are finished
    for (size_t i = 0; i < ReadThreads.size(); i++)
    { 
      ReadThreads[i].join();
    }
    number_of_result_blocks = number_of_result_blocks - ReadThreads.size();
    ++loop_c;
    ReadThreads.clear();
  }

  progressbar->end();

  //PRINT_INFO("%s", log.c_str());
  //print_data();
  

  StopWatch.total("Duration of reading FRD [ms]: ");

  return true;
}

std::vector<std::string> CoreResultsFrd::split_line(std::string frdline)
{
  std::vector<std::string> str_array;
  std::string str_temp;

  // trim whitespaces
  size_t strBegin = frdline.find_first_not_of(" \t\r\f\v\n");
  if (strBegin != std::string::npos)
  {
    size_t strEnd = frdline.find_last_not_of(" \t\r\f\v\n");
    size_t strRange = strEnd - strBegin + 1;
    frdline = frdline.substr(strBegin, strRange);
  }
  // reduce whitespaces
  size_t beginSpace = frdline.find_first_of(" \t\r\f\v\n");
  while (beginSpace != std::string::npos)
  {
    size_t endSpace = frdline.find_first_not_of(" \t\r\f\v\n", beginSpace);
    size_t range = endSpace - beginSpace;

    frdline.replace(beginSpace,range, " ");
    size_t newStart = beginSpace + 1;
    beginSpace = frdline.find_first_of(" \t\r\f\v\n",newStart);
  }
  // insert whitespaces before "-" whitespaces but not "E-"
  beginSpace = frdline.find_first_of("-");
  while (beginSpace != std::string::npos)
  {
    size_t range = 1;
    if ((frdline[beginSpace-1] != 'E') && (frdline[beginSpace-1] != ' ') && (beginSpace !=0))
    {
      frdline.replace(beginSpace,range, " -");
    }
    size_t newStart = beginSpace + 2;
    beginSpace = frdline.find_first_of("-",newStart);
  }

  std::stringstream ss(frdline);

  // split line with delimiter " "
  while (getline(ss,str_temp,' '))
  {
    str_array.push_back(str_temp);
  }
  
  return str_array;
}

std::string CoreResultsFrd::get_key(std::string frdline)
{
  frdline = frdline.substr (0,5);

  // trim whitespaces
  size_t strBegin = frdline.find_first_not_of(" \t\r\f\v\n");
  if (strBegin != std::string::npos)
  {
    size_t strEnd = frdline.find_last_not_of(" \t\r\f\v\n");
    size_t strRange = strEnd - strBegin + 1;
    frdline = frdline.substr(strBegin, strRange);
  }
  // reduce whitespaces
  size_t beginSpace = frdline.find_first_of(" \t\r\f\v\n");
  while (beginSpace != std::string::npos)
  {
    size_t endSpace = frdline.find_first_not_of(" \t\r\f\v\n", beginSpace);
    size_t range = endSpace - beginSpace;

    frdline.replace(beginSpace,range, " ");
    size_t newStart = beginSpace + 1;
    beginSpace = frdline.find_first_of(" \t\r\f\v\n",newStart);
  }
  
  return frdline;
}


bool CoreResultsFrd::check_mode(std::vector<std::string> line)
{
  if (line[0] == "9999") // file finished
  {
    current_read_mode = 9999;
  }else if (current_read_mode == 0)
  {
    if (line[0] == "2C")
    {
      current_read_mode = 1;
    }
  }else if (current_read_mode == 1)
  {
    if (line[0] == "3C")
    {
      current_read_mode = 2;
    }
  }else if (current_read_mode == 2)
  {
    if (line[0] == "-3")
    {
      current_read_mode = 3; // result blocks
    }
  }else if (current_read_mode > 2) // switch to parameter mode
  {
    if (line[0] == "-3")
    {
      current_read_mode = 3;
    }
  }
  
  return true;
}

bool CoreResultsFrd::check_mode_by_key(std::string key)
{
  if (key == "9999") // file finished
  {
    current_read_mode = 9999;
  }else if (current_read_mode == 0)
  {
    if (key == "2")
    {
      current_read_mode = 1;
    }
  }else if (current_read_mode == 1)
  {
    if (key == "3")
    {
      current_read_mode = 2;
    }
  }else if (current_read_mode == 2)
  {
    if (key == "-3")
    {
      current_read_mode = 3; // result blocks
    }
  }else if (current_read_mode > 2) // switch to parameter mode
  {
    if (key == "-3")
    {
      current_read_mode = 3;
    }
  }
  
  return true;
}

bool CoreResultsFrd::read_header(std::vector<std::string> line)
{
  std::vector<std::string> tmp;
  /*
  1UUSER                                                              
  1UDATE              18.july.2023                                    
  1UTIME              22:53:17                                        
  1UHOST                                                              
  1UPGM               CalculiX                                        
  1UVERSION           Version 2.19                             
  1UCOMPILETIME       Wed 12 Jan 2022 04:14:21 PM PST                    
  1UDIR                                                               
  1UDBN                                                               
  1UMAT    1STEEL   
  */

  if ((line[0] == "1UUSER")||(line[0] == "1UDATE")||(line[0] == "1UTIME")||(line[0] == "1UHOST")||(line[0] == "1UPGM")||(line[0] == "1UVERSION")||(line[0] == "1UDIR")||(line[0] == "1UDBN"))
  {
    for (size_t i = 0; i < line.size(); i++)
    {
      tmp.push_back(line[i]);
    }
    header.push_back(tmp);
  }else if ((line[0] == "1UMAT"))
  {
    //size_t name_pos = line[1].find_first_not_of("0123456789");
    //tmp.push_back(line[1].substr(0, name_pos));
    //tmp.push_back(line[1].substr(name_pos, line[1].length() - name_pos));
    size_t name_pos = 1;
    if ((tmp.size()>9)&&(tmp.size()<100)) //10-99
    {
      name_pos = 2;
    }else if ((tmp.size()>99)&&(tmp.size()<1000)) //100-999
    {
      name_pos = 3;
    }else if (tmp.size()>1000) // 1000-inf
    {
      name_pos = line[1].find_first_not_of("0123456789");
    }
    tmp.push_back(line[1].substr(0, name_pos));
    tmp.push_back(line[1].substr(name_pos, line[1].length() - name_pos));
    materials.push_back(tmp);
  }

  return true;
}

bool CoreResultsFrd::read_node(std::vector<std::string> line)
{
  int node_id;
  std::vector<double> node_coords(3);
  int nodes_coords_data_id = -1;
  
  if (line[0] == "-1")
  {
    node_id = std::stoi(line[1]);

    node_coords[0] = ccx_iface->string_scientific_to_double(line[2]);
    node_coords[1] = ccx_iface->string_scientific_to_double(line[3]);
    node_coords[2] = ccx_iface->string_scientific_to_double(line[4]);

    nodes_coords.push_back(node_coords);
    int nodes_coords_data_id = int(nodes_coords.size())-1;

    nodes.push_back({node_id,nodes_coords_data_id});
  }

  return true;
}



bool CoreResultsFrd::read_nodes_thread(int start,int end,int data_start)
{
  std::string frdline = "";
  int ic = 0;
  int id = 0;

  std::ifstream frd;
  frd.open(this->filepath);
  while (std::getline(frd,frdline))
  { 
    if ((ic>=start)&&(ic<=end))
    {      
      std::vector<std::string> line = this->split_line(frdline);
      
      int node_id;
      std::vector<double> node_coords(3);
      int nodes_coords_data_id = -1;
      
      if (line[0] == "-1")
      {
        node_id = std::stoi(line[1]);

        node_coords[0] = ccx_iface->string_scientific_to_double(line[2]);
        node_coords[1] = ccx_iface->string_scientific_to_double(line[3]);
        node_coords[2] = ccx_iface->string_scientific_to_double(line[4]);

        nodes_coords[data_start+id] = node_coords;
        nodes[data_start+id] = {node_id,data_start+id};
        ++id;
      }
      /*      
      std::string log;
      log = std::to_string(start)+ " -- " + std::to_string(ic) + " -- " + std::to_string(end) + " -- " + std::to_string(data_start+id) +  " \n";
      PRINT_INFO("%s", log.c_str());
      */
    }else if (ic>end)
    {
      break;
    }
    if((frd.eof())){
      break;
    }
    ++ic;
  }
  frd.close();
  return true;
}

bool CoreResultsFrd::read_element(std::vector<std::string> line)
{
  int element_id;
  int element_type;
  int material_id;
  std::vector<int> connectivity;
  int elements_connectivity_data_id = -1;
  
  if (line[0] == "-1")
  {
    element_id = std::stoi(line[1]);
    element_type = std::stoi(line[2]);
    material_id = std::stoi(line[4]);
    elements.push_back({element_id,element_type,elements_connectivity_data_id,material_id});
  }else if (line[0] == "-2")
  {
    if (elements[elements.size()-1][2] == -1)
    {
      for (size_t i = 1; i < line.size(); i++)
      {
        int tmp_int = std::stoi(line[i]);
        connectivity.push_back(tmp_int);
      }
      elements_connectivity.push_back(connectivity);

      elements_connectivity_data_id = int(elements_connectivity.size()) - 1;
      elements[elements.size()-1][2] = elements_connectivity_data_id;
    }else if (elements[elements.size()-1][2] > -1)
    {
      for (size_t i = 1; i < line.size(); i++)
      {
        int tmp_int = std::stoi(line[i]);
        elements_connectivity[elements[elements.size()-1][2]].push_back(tmp_int);
      }
    }
  }
  return true;
}

bool CoreResultsFrd::read_elements_thread(int start,int end,int data_start)
{
  std::string frdline = "";
  int ic = 0;
  int id = -1;

  std::ifstream frd;
  frd.open(this->filepath);
  while (std::getline(frd,frdline))
  { 
    if ((ic>=start)&&(ic<=end))
    {
      std::vector<std::string> line = this->split_line(frdline);
      
      int element_id;
      int element_type;
      int material_id;
      std::vector<int> connectivity;
      int elements_connectivity_data_id = -1;
      
      if (line[0] == "-1")
      {
        ++id;
        element_id = std::stoi(line[1]);
        element_type = std::stoi(line[2]);
        material_id = std::stoi(line[4]);
        elements[data_start+id] = {element_id,element_type,elements_connectivity_data_id,material_id};
      }else if (line[0] == "-2")
      {
        if (elements[data_start+id][2] == -1)
        {
          for (size_t i = 1; i < line.size(); i++)
          {
            int tmp_int = std::stoi(line[i]);
            connectivity.push_back(tmp_int);
          }
          elements_connectivity[data_start+id] = connectivity;

          elements_connectivity_data_id = data_start+id;
          elements[data_start+id][2] = elements_connectivity_data_id;
        }else if (elements[data_start+id][2] > -1)
        {
          for (size_t i = 1; i < line.size(); i++)
          {
            int tmp_int = std::stoi(line[i]);
            elements_connectivity[elements[data_start+id][2]].push_back(tmp_int);
          }
        }
      }
      
      //std::string log;
      //log = std::to_string(start)+ " -- " + std::to_string(ic) + " -- " + std::to_string(end) + " -- " + std::to_string(data_start+id) +  " \n";
      //PRINT_INFO("%s", log.c_str());

    }else if (ic>end)
    {
      break;
    }
    if((frd.eof())){
      break;
    }
    ++ic;
  }
  frd.close();
  return true;
}

bool CoreResultsFrd::read_parameter_header(std::vector<std::string> line)
{
  // result_blocks[0][0] result block
  // result_blocks[0][1] step
  // result_blocks[0][2] step increment
  // result_blocks[0][3] total increment
  // result_blocks[0][4] total time data id
  // result_blocks[0][5] result block type
  // result_blocks[0][6] result block data id
  // total_time[0] total time
  // result_block_type[0][0] disp
  
  int result_block = -1;
  int step = -1;
  int step_increment = -1;
  int total_increment = -1;
  double total_time = -1.0;
  int result_block_data_id = -1;
  int result_block_type_data_id = -1;
  
  if (line[0] == "1PSTEP")
  {
    result_block = std::stoi(line[1]);
    step_increment = std::stoi(line[2]);
    step = std::stoi(line[3]);

    current_result_block = result_block;
    current_step = step;
    current_step_increment = step_increment;

  } else if (line[0] == "100CL")
  {
    total_time = ccx_iface->string_scientific_to_double(line[2]);
    total_times.push_back(total_time);
    current_total_time_id = int(total_times.size())-1;

    total_increment = ccx_iface->string_scientific_to_double(line[5]);
    current_total_increment = total_increment;

  } else if (line[0] == "-4") // getting result type
  { 
    std::vector<std::vector<double>> tmp_result_block_data;
    std::vector<std::vector<int>> tmp_result_block_node_data;
    result_block_data.push_back(tmp_result_block_data);
    result_block_node_data.push_back(tmp_result_block_node_data);
      
    result_block_data_id = int(result_block_data.size())-1;
    result_block_type_data_id = this->get_current_result_block_type(line[1]);
    
    result_blocks.push_back({current_result_block,current_step,current_step_increment,current_total_increment,current_total_time_id,result_block_type_data_id,result_block_data_id});
    current_read_mode = 4;

    // write new set of component names
    std::vector<std::string> tmp_result_block_components;
    result_block_components.push_back(tmp_result_block_components);
  }
  return true;
}

int CoreResultsFrd::get_current_result_block_type(std::string result_type)
{

  for (size_t i = 0; i < result_block_type.size(); i++)
  {
    if (result_block_type[i]==result_type)
    {
      return int(i);
    }
  }
  result_block_type.push_back(result_type);
  int data_id = int(result_block_type.size())-1;
  
  return data_id;
}

bool CoreResultsFrd::read_nodal_result_block(std::vector<std::string> line)
{
  if (line[0] == "-1")
  {
    int node_id;
    int n_comp = 0;
    int result_block_node_data_id = -1;

    node_id = std::stoi(line[1]);
    
    // add components if needed    
    if (result_block_type[result_blocks[result_blocks.size()-1][5]] == "STRESS")
    {
      if (result_block_data[result_block_data.size()-1].size()==0)
      {
        result_block_components[result_block_components.size()-1].push_back("MISES");
        result_block_components[result_block_components.size()-1].push_back("PS1");
        result_block_components[result_block_components.size()-1].push_back("PS2");
        result_block_components[result_block_components.size()-1].push_back("PS3");
        result_block_components[result_block_components.size()-1].push_back("worstPS");
        result_block_components[result_block_components.size()-1].push_back("maxShear");
      }
    }
    if (result_block_type[result_blocks[result_blocks.size()-1][5]] == "TOSTRAIN")
    {
      if (result_block_data[result_block_data.size()-1].size()==0)
      {
        result_block_components[result_block_components.size()-1].push_back("MISES");
        result_block_components[result_block_components.size()-1].push_back("PS1");
        result_block_components[result_block_components.size()-1].push_back("PS2");
        result_block_components[result_block_components.size()-1].push_back("PS3");
        result_block_components[result_block_components.size()-1].push_back("worstPS");
        result_block_components[result_block_components.size()-1].push_back("maxShear");
      }
    }

    n_comp = int(result_block_components[result_block_components.size()-1].size());
    std::vector<double> result_comp(n_comp);
    
    /*
    if (result_block_components[result_block_components.size()-1][n_comp-1] == "ALL")
    {
      for (size_t i = 0; i < n_comp-1; i++)
      {
        result_comp[i] = ccx_iface->string_scientific_to_double(line[i+2]);
      }
      
      double tmp_all = 0;
      for (size_t i = 0; i < n_comp-1; i++)
      {
        tmp_all = result_comp[i]*result_comp[i];
      }
      result_comp[n_comp-1] = std::sqrt(tmp_all);
    }else{ */

    for (size_t i = 0; i < n_comp; i++)
    {
      if ((result_block_type[result_blocks[result_blocks.size()-1][5]] == "STRESS") && (i > 5))
      {
        if (i == 6)
        {
          result_comp[i] = ccx_iface->compute_von_mises_stress({result_comp[0],result_comp[1],result_comp[2],result_comp[3],result_comp[4],result_comp[5]});
        }
        // compute principal stress
        if (i == 7) 
        {
          std::vector<double> ps = ccx_iface->compute_principal_stresses({result_comp[0],result_comp[1],result_comp[2],result_comp[3],result_comp[4],result_comp[5]});
          result_comp[i] = ps[0];
          result_comp[i+1] = ps[1];
          result_comp[i+2] = ps[2];
          result_comp[i+3] = ps[3];
          result_comp[i+4] = 0.5 * std::max({ps[0]-ps[2],ps[0]-ps[1],ps[1]-ps[2]});
        }
      }else if ((result_block_type[result_blocks[result_blocks.size()-1][5]] == "TOSTRAIN") && (i > 5))
      {
        if (i == 6)
        {
          result_comp[i] = ccx_iface->compute_von_mises_strain({result_comp[0],result_comp[1],result_comp[2],result_comp[3],result_comp[4],result_comp[5]});
        }
        // compute principal strains
        if (i == 7) 
        {
          std::vector<double> pe = ccx_iface->compute_principal_strains({result_comp[0],result_comp[1],result_comp[2],result_comp[3],result_comp[4],result_comp[5]});
          result_comp[i] = pe[0];
          result_comp[i+1] = pe[1];
          result_comp[i+2] = pe[2];
          result_comp[i+3] = pe[3];
          result_comp[i+4] = 0.5 * std::max({pe[0]-pe[2],pe[0]-pe[1],pe[1]-pe[2]});
        }
      }else{
        result_comp[i] = ccx_iface->string_scientific_to_double(line[i+2]);
      }
    }
        
    result_block_data[result_block_data.size()-1].push_back(result_comp);
    result_block_node_data_id = int(result_block_data[result_block_data.size()-1].size())-1;
    result_block_node_data[result_block_node_data.size()-1].push_back({node_id,result_block_node_data_id});
  }else if (line[0] == "-5")
  {
    // ignore "ALL"
    if (line[1] != "ALL")
    {
      result_block_components[result_block_components.size()-1].push_back(line[1]);
    }
  }

  return true;
}

bool CoreResultsFrd::read_nodal_result_block_add_components(std::vector<std::string> line)
{
  if (line[0] == "-1")
  {
    // add components if needed    
    if (result_block_type[result_blocks[result_blocks.size()-1][5]] == "STRESS")
    {
      if (result_block_data[result_block_data.size()-1].size()==0)
      {
        result_block_components[result_block_components.size()-1].push_back("MISES");
        result_block_components[result_block_components.size()-1].push_back("PS1");
        result_block_components[result_block_components.size()-1].push_back("PS2");
        result_block_components[result_block_components.size()-1].push_back("PS3");
        result_block_components[result_block_components.size()-1].push_back("worstPS");
        result_block_components[result_block_components.size()-1].push_back("maxShear");
      }
    }
    if (result_block_type[result_blocks[result_blocks.size()-1][5]] == "TOSTRAIN")
    {
      if (result_block_data[result_block_data.size()-1].size()==0)
      {
        result_block_components[result_block_components.size()-1].push_back("MISES");
        result_block_components[result_block_components.size()-1].push_back("PS1");
        result_block_components[result_block_components.size()-1].push_back("PS2");
        result_block_components[result_block_components.size()-1].push_back("PS3");
        result_block_components[result_block_components.size()-1].push_back("worstPS");
        result_block_components[result_block_components.size()-1].push_back("maxShear");
      }
    }
  }
  return true;
}

bool CoreResultsFrd::read_nodal_result_block_thread(int result_block_data_id,int start,int end)
{
  std::string frdline = "";
  int ic = 0;
  int id = -1;

  std::ifstream frd;
  frd.open(this->filepath);
  while (std::getline(frd,frdline))
  { 
    if ((ic>=start)&&(ic<=end))
    {
      std::vector<std::string> line = this->split_line(frdline);

      if (line[0] == "-1")
      {
        int node_id;
        int n_comp = 0;
        int result_block_node_data_id = -1;

        node_id = std::stoi(line[1]);
        
        n_comp = result_block_components[result_block_data_id].size();
        std::vector<double> result_comp(n_comp);
        
        for (size_t i = 0; i < n_comp; i++)
        {          
          if ((result_block_type[result_blocks[result_block_data_id][5]] == "STRESS") && (i > 5))
          {
            if (i == 6)
            {
              result_comp[i] = ccx_iface->compute_von_mises_stress({result_comp[0],result_comp[1],result_comp[2],result_comp[3],result_comp[4],result_comp[5]});
            }
            // compute principal stress
            if (i == 7) 
            {
              std::vector<double> ps = ccx_iface->compute_principal_stresses({result_comp[0],result_comp[1],result_comp[2],result_comp[3],result_comp[4],result_comp[5]});
              result_comp[i] = ps[0];
              result_comp[i+1] = ps[1];
              result_comp[i+2] = ps[2];
              result_comp[i+3] = ps[3];
              result_comp[i+4] = 0.5 * std::max({ps[0]-ps[2],ps[0]-ps[1],ps[1]-ps[2]});
            }
          }else if ((result_block_type[result_blocks[result_block_data_id][5]] == "TOSTRAIN") && (i > 5))
          {
            if (i == 6)
            {
              result_comp[i] = ccx_iface->compute_von_mises_strain({result_comp[0],result_comp[1],result_comp[2],result_comp[3],result_comp[4],result_comp[5]});
            }
            // compute principal strains
            if (i == 7) 
            {
              std::vector<double> pe = ccx_iface->compute_principal_strains({result_comp[0],result_comp[1],result_comp[2],result_comp[3],result_comp[4],result_comp[5]});
              result_comp[i] = pe[0];
              result_comp[i+1] = pe[1];
              result_comp[i+2] = pe[2];
              result_comp[i+3] = pe[3];
              result_comp[i+4] = 0.5 * std::max({pe[0]-pe[2],pe[0]-pe[1],pe[1]-pe[2]});
            }
          }else{
            /*
            if ((i==0)&&(ic==start))
            {
              std::string log;
              log = "result_block_data_id " + std::to_string(result_block_data_id) + "-" + std::to_string(start) + "-" + std::to_string(end) +  " \n";
              PRINT_INFO("%s", log.c_str());
              log = "n_comp " + std::to_string(n_comp) +  " \n";
              PRINT_INFO("%s", log.c_str());
              log = "line comp " + std::to_string(line.size()-2) +  " \n";
              PRINT_INFO("%s", log.c_str());
            }
            */
            result_comp[i] = ccx_iface->string_scientific_to_double(line[i+2]);
          }
        }
        
        result_block_data[result_block_data_id].push_back(result_comp);
        result_block_node_data_id = int(result_block_data[result_block_data_id].size())-1;
        result_block_node_data[result_block_data_id].push_back({node_id,result_block_node_data_id});
        
      }
    }else if (ic>end)
    {
      break;
    }
    if((frd.eof())){
      break;
    }
    ++ic;
  }
  frd.close();

  return true;
}

std::vector<std::string> CoreResultsFrd::get_result_block_components_from_result_block_type(std::string result_block_type)
{
  // on first match write out the components for the first matching result block
  std::vector<std::string> components;
  for (size_t i = 0; i < this->result_block_type.size(); i++)
  {
    if (this->result_block_type[i]==result_block_type)
    {
      components = this->result_block_components[i];
      return components;
    }
  }
  
  return components;
}

int CoreResultsFrd::get_result_block_component_id(int result_block_type_id,std::string result_block_component)
{
  int component_id = -1;
  std::vector<std::string> components;
  components = get_result_block_components_from_result_block_type(this->result_block_type[result_block_type_id]);

  for (size_t i = 0; i < components.size(); i++)
  {
    if (components[i]==result_block_component)
    {
      component_id = int(i);
    }
  }
    
  return component_id;
}

std::vector<int> CoreResultsFrd::get_node_range()
{
  std::vector<int> tmp;
  std::string frdline = "";
  int ic = 0;
  int start = 0;
  int end = 0;

  std::ifstream frd;
  frd.open(this->filepath);
  while (std::getline(frd,frdline))
  { 
    ++ic;
    //std::string key = this->get_key(frdline);
    if (this->keys[ic-1]=="2")
    {
      start = ic;
    }else if (this->keys[ic-1]=="3")
    {
      end = ic-3;
      break;
    }
    if((frd.eof())){
      break;
    }
  }
  frd.close();

  tmp.push_back(start);
  tmp.push_back(end);

  return tmp;
}

std::vector<int> CoreResultsFrd::get_element_range()
{
  std::vector<int> tmp;
  std::string frdline = "";
  int ic = 0;
  int start = 0;
  int end = 0;
  bool block_started = false;
  std::string last_key = "-2";
  int number_of_elements = 0;

  std::ifstream frd;
  frd.open(this->filepath);
  while (std::getline(frd,frdline))
  { 
    ++ic;
    //std::string key = this->get_key(frdline);
    std::string log;

    if (this->keys[ic-1]=="3")
    {
      start = ic;
      block_started = true;
    }
    if (block_started)
    {
      if ((last_key=="-2")&&(this->keys[ic-1]=="-1"))
      {
        ++number_of_elements;
      }
      if (this->keys[ic-1]=="-3")
      {
        end = ic-2;
        break;
      }
      if (start!=ic)
      {
        last_key = this->keys[ic-1];
      }
    }
    if((frd.eof())){
      break;
    }
  }
  frd.close();

  tmp.push_back(start);
  tmp.push_back(end);
  tmp.push_back(number_of_elements);

  return tmp;
}

bool CoreResultsFrd::insert_key(int start, int end)
{
  std::string frdline = "";
  int ic = 0;

  std::ifstream frd;
  frd.open(this->filepath);
  while (std::getline(frd,frdline))
  { 
    if ((ic>=start)&&(ic<=end))
    {      
      this->keys[ic] = this->get_key(frdline);
    }else if (ic>end)
    {
      break;
    }
    if((frd.eof())){
      break;
    }
    ++ic;
  }
  frd.close();
  return true;
}

bool CoreResultsFrd::check_key(std::string key, int line)
{
  if ((this->keys[line]==key))
  {
    return true;
  }

  return false;
}

void CoreResultsFrd::set_element_range_data_start(std::vector<std::vector<int>> &thread_ranges)
{
  int id = 0;
  for (size_t i = 0; i < thread_ranges.size(); i++)
  {  
    std::vector<int> tmp;
    std::string frdline = "";
    int ic = 0;
    bool block_started = false;
    std::string last_key = "-2";
    int number_of_elements = 0;

    std::ifstream frd;
    frd.open(this->filepath);
    while (std::getline(frd,frdline))
    { 
      if ((ic>=thread_ranges[i][0])&&(ic<=thread_ranges[i][1]))
      {
        std::string key = this->get_key(frdline);
        if ((last_key=="-2")&&(key=="-1"))
        {
          ++number_of_elements;
        }
        last_key = key;
      }
      if (ic>thread_ranges[i][1])
      {
        break;
      }   
      if((frd.eof())){
        break;
      }
      ++ic;
    }
    frd.close();
    thread_ranges[i][2] = id;
    id = id + number_of_elements;
    
    //std::string log;
    //log = "range no " + std::to_string(i) + "-" + std::to_string(thread_ranges[i][0]) + "-" + std::to_string(thread_ranges[i][1]) + "-" + std::to_string(thread_ranges[i][2]) +  " \n";
    //PRINT_INFO("%s", log.c_str());
  }
}

bool CoreResultsFrd::sort_result_block_node_data(int data_id)
{
  // sorting for faster search
  std::vector<int> tmp_node_ids;
  std::vector<int> tmp_node_data_ids;

  for (size_t i = 0; i < result_block_node_data[data_id].size(); i++)
  {
    tmp_node_ids.push_back(result_block_node_data[data_id][i][0]);
    tmp_node_data_ids.push_back(result_block_node_data[data_id][i][1]);
  }  
  auto p = sort_permutation(tmp_node_ids);
  this->apply_permutation(tmp_node_ids, p);
  this->apply_permutation(tmp_node_data_ids, p);
  sorted_node_ids[data_id] = tmp_node_ids;
  sorted_node_data_ids[data_id] = tmp_node_data_ids;

  return true;
}

void CoreResultsFrd::update_progressbar()
{
  int part = 0;
  for (size_t i = 0; i < progress.size()-1; i++)
  {
    part += progress[i];
  }
  progressbar->percent(double(part)/double(progress[progress.size()-1]));
}

bool CoreResultsFrd::print_data()
{
  std::string log;
  log = "FRD DATA for Job ID " + std::to_string(job_id) + " \n";

  log.append(std::to_string(header.size()) + " ++++++ " + std::to_string(materials.size()));
  log.append("\n");
  if (header.size()>0)
  {
    for (size_t i = 0; i < header.size(); i++)
    {
      for (size_t ii = 0; ii < header[i].size(); ii++)
      {
        log.append(header[i][ii] + " ");
      }
      log.append("\n");
    }
  }
  
  if (materials.size()>0)
  {
    for (size_t i = 0; i < materials.size(); i++)
    {
      for (size_t ii = 0; ii < materials[i].size(); ii++)
      {
        log.append(materials[i][ii] + " ");
      }
      log.append("\n");
    }
  }

  if (nodes.size()>0)
  {
    //for (size_t i = 0; i < nodes.size(); i++)
    for (size_t i = nodes.size()-1; i < nodes.size(); i++)
    {
      log.append(std::to_string(nodes[i][0]) + " ## " + std::to_string(nodes[i][1]) + " ## x ");
      log.append(std::to_string(nodes_coords[nodes[i][1]][0]) + " ## y " + std::to_string(nodes_coords[nodes[i][1]][1]) + " ## z " + std::to_string(nodes_coords[nodes[i][1]][2]));

      log.append("\n");
    }
  }

  if (elements.size()>0)
  {
    //for (size_t i = 0; i < elements.size(); i++)
    for (size_t i = elements.size()-1; i < elements.size(); i++)
    {
      log.append(std::to_string(elements[i][0]) + " ## " + std::to_string(elements[i][1]) + " ## " + std::to_string(elements[i][2]) + " #---# ");
      
      for (size_t ii = 0; ii < elements_connectivity[elements[i][2]].size(); ii++)
      {
        log.append(std::to_string(elements_connectivity[elements[i][2]][ii]) + " ");
      }

      log.append("\n");
    }
  }
  if (result_blocks.size()>0)
  {
    for (size_t i = 0; i < result_blocks.size(); i++)
    {
      log.append("result block " + std::to_string(result_blocks[i][0]) + " ## ");
      log.append("step " + std::to_string(result_blocks[i][1]) + " ## ");
      log.append("step increment " + std::to_string(result_blocks[i][2]) + " ## ");
      log.append("total increment " + std::to_string(result_blocks[i][3]) + " ## ");
      log.append("total time data id " + std::to_string(result_blocks[i][4]) + " ## ");
      log.append("result block type " + std::to_string(result_blocks[i][5]) + " ## ");
      log.append("result block data id " + std::to_string(result_blocks[i][6]) + " \n");
      
      log.append("total time " + std::to_string(total_times[result_blocks[i][4]]) + " \n");
      log.append("result block type " + result_block_type[result_blocks[i][5]] + " \n");
      log.append("component names ");
      for (size_t ii = 0; ii < result_block_components[i].size(); ii++)
      {
        log.append(result_block_components[i][ii] + " ");
      }
      log.append("\n");

      for (size_t ii = result_block_node_data[result_blocks[i][6]].size()-1; ii < result_block_node_data[result_blocks[i][6]].size(); ii++)
      {
        log.append(std::to_string(result_block_node_data[result_blocks[i][6]][ii][0]) + " ");
        for (size_t iii = 0; iii < result_block_components[i].size(); iii++)
        {
          log.append(ccx_iface->to_string_scientific(result_block_data[result_blocks[i][6]][ii][iii]) + " ");
        }
        log.append("\n");
      }
      
      log.append("\n");
    }
  }

  PRINT_INFO("%s", log.c_str());
  return true;
}


//sorting of vectors
template <typename T> 
std::vector<std::size_t> CoreResultsFrd::sort_permutation(
    const std::vector<T>& vec)
{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
        [&](std::size_t i, std::size_t j){ return vec[i] < vec[j]; });

    return p;
}

template <typename T> 
void CoreResultsFrd::apply_permutation(
    std::vector<T>& vec,
    const std::vector<std::size_t>& p)
{
    std::vector<bool> done(vec.size());
    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        if (done[i])
        {
            continue;
        }
        done[i] = true;
        std::size_t prev_j = i;
        std::size_t j = p[i];
        while (i != j)
        {
            std::swap(vec[prev_j], vec[j]);
            done[j] = true;
            prev_j = j;
            j = p[j];
        }
    }
}