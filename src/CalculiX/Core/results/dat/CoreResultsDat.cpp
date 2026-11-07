#include "Core/results/dat/CoreResultsDat.hpp"
#include "Core/results/dat/CoreResultsDatFormat.hpp"
#include <numeric>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <array>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/results/CalculiXResultsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "StopWatch.hpp"
#include "loadUserOptions.hpp"

#include "ThreadPool.hpp"
#include "Core/results/ResultsReaderPipeline.hpp"
#include "Core/results/ResultReadBuffer.hpp"
#include "Core/results/ResultReadNumbers.hpp"
#include "Core/results/ResultReadTimings.hpp"
#include <functional>

#include <algorithm>
#include <iterator>
#include <fstream>
#include <iostream>
#include <sstream>

bool CoreResultsDat::init(int job_id)
{
  CalculiXJobsInterface ccx_jobs_iface;

  if (is_initialized)
  {
    return false; // already initialized
  }else{
    this->job_id = job_id;
    
    std::vector<std::string> job_data = ccx_jobs_iface.get_job_data(job_id);
    this->filepath = job_data[1] + ".dat";

    progressbar = CubitInterface::app_util().get()->progress_tool();;

    result_section_label.push_back("time");
    result_section_label.push_back("increment");
    result_section_label.push_back("total surface force fx");
    result_section_label.push_back("total surface force fy");
    result_section_label.push_back("total surface force fz");
    result_section_label.push_back("moment about the origin mx"); 
    result_section_label.push_back("moment about the origin my");
    result_section_label.push_back("moment about the origin mz");
    result_section_label.push_back("center of gravity 1");
    result_section_label.push_back("center of gravity 2");
    result_section_label.push_back("center of gravity 3");
    result_section_label.push_back("mean normal 1");
    result_section_label.push_back("mean normal 2");
    result_section_label.push_back("mean normal 3");
    result_section_label.push_back("moment about the center of gravity mx");
    result_section_label.push_back("moment about the center of gravity my");
    result_section_label.push_back("moment about the center of gravity mz");
    result_section_label.push_back("area");
    result_section_label.push_back("normal force");
    result_section_label.push_back("shear force");
    result_section_label.push_back("torque moment");
    result_section_label.push_back("bending moment");

    is_initialized = true;
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
  result_tables.clear();
  pending_local_tables.clear();
  diagnostics.clear();
  total_times.clear();
  result_block_components.clear();
  result_block_type.clear();
  result_block_set.clear();
  result_block_data.clear();
  result_block_c1_data.clear();
  buckle_data.clear();
  result_section.clear();
  result_section_data.clear();
  result_section_set.clear();
  dat_arrays.clear();

  sorted_c1.clear();
  sorted_result_block_c1_data_id.clear();
  sorted_result_block_c1_data_type.clear();
  progress.clear();
  current_read_mode = 0;
  current_result_block = -1;
  current_total_time_id = -1;
  current_step = -1;
  current_increment = -1;


  return true;
}

bool CoreResultsDat::check_initialized()
{
  return is_initialized;
}

bool CoreResultsDat::read()
{
  return read_parallel_boost();
}

bool CoreResultsDat::read_single()
{
  CalculiXResultsInterface ccx_results_iface;

  StopWatch StopWatch;
  int maxlines = 0;
  int currentline = 0;
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
    progressbar->start(0,100,"Scanning Results dat");
    while (std::getline(dat,datline))
    { 
      ++maxlines;
      if(dat.eof()){
        break;
      }
    }
    progressbar->end();
    dat.close();
    dat.open(this->filepath);

    progressbar->start(0,100,"Reading Results DAT");
    t_start = std::chrono::high_resolution_clock::now();
    
    log = "";
    while (dat)
    {
      if ((!std::getline(dat,datline))||(dat.eof()))
      {
        break;
      }
      ++currentline;
  
      const auto t_end = std::chrono::high_resolution_clock::now();
      int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
      if (duration > 500)
      {
        progressbar->percent(double(currentline)/double(maxlines));
        progressbar->check_interrupt();
        t_start = std::chrono::high_resolution_clock::now();
      }
      
      // if whitespace line, get next line until not a whitespace line
      while(this->is_whitespace(datline))
      {
        if ((!std::getline(dat,datline))||(dat.eof()))
        {
          break;
        }
        ++currentline;
      }

      dat_array = this->split_line(datline);
      if (dat_array.empty())
      {
        continue;
      }
      
      //first lets check if the step or increment needs an update
      this->check_step(dat_array);
      //second lets check if the mode is still valid
      this->check_mode(dat_array);

      if (dat.eof())
      {
        break;
      }else if (current_read_mode == 1)
      {
        this->read_header(dat_array);
      } else if (current_read_mode == 2)
      {
        this->read_line(dat_array);
      } else if (current_read_mode == 10)
      {
        // do nothing here
      } else if (current_read_mode == 11)
      {
        this->header_emas(dat_array);
      } else if (current_read_mode == 102)
      {
        // Buckling, save data extra
        this->read_line_buckle(dat_array);
      } else if (current_read_mode == 111)
      {
        this->header_section_1(dat_array);
      } else if (current_read_mode == 112)
      {
        this->header_section_2(dat_array);
      } else if (current_read_mode == 114)
      {
        this->header_section_3(dat_array);
      } else if (current_read_mode == 116)
      {
        this->header_section_4(dat_array);
      } else if (current_read_mode == 118)
      {
        this->header_section_5(dat_array);
      } else if (current_read_mode == 113)
      {
        this->read_line_section_1(dat_array);
      } else if (current_read_mode == 115)
      {
        this->read_line_section_2(dat_array);
      } else if (current_read_mode == 117)
      {
        this->read_line_section_3(dat_array);
      } else if (current_read_mode == 119)
      {
        this->read_line_section_4(dat_array);
      }
      
      /*if (!this->is_number(dat_array[0]))
      {
        log="";
        for (size_t i = 0; i < dat_array.size(); i++)
        {
          log.append(std::to_string(current_read_mode) + "#");
          log.append(dat_array[i] + " ");
        }
        log.append("\n");
        PRINT_INFO("%s", log.c_str());
      }*/
    }
  }
  dat.close();
  progressbar->end();

  // for stress/strain data compute extra components
  progressbar->start(0,100,"Reading Results DAT: pre-defined calculations");
  t_start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < result_blocks.size(); i++)
  {
    //log = "i " + std::to_string(i) + " type '" + result_block_type[result_blocks[i][2]] + "'\n";
    //PRINT_INFO("%s", log.c_str());

    //stresses
    if (this->erase_whitespace(result_block_type[result_blocks[i][2]])=="stresses") 
    {
      // add components
      result_block_components[result_blocks[i][4]].push_back("mises");

      // compute values
      for (size_t ii = 0; ii < result_block_data[result_blocks[i][4]].size(); ii++)
      {
        result_block_data[result_blocks[i][4]][ii].push_back(ccx_results_iface.compute_von_mises_stress({
        result_block_data[result_blocks[i][4]][ii][2],
        result_block_data[result_blocks[i][4]][ii][3],
        result_block_data[result_blocks[i][4]][ii][4],
        result_block_data[result_blocks[i][4]][ii][5],
        result_block_data[result_blocks[i][4]][ii][6],
        result_block_data[result_blocks[i][4]][ii][7]}));
      }

      // add components
      result_block_components[result_blocks[i][4]].push_back("PS1");
      result_block_components[result_blocks[i][4]].push_back("PS2");
      result_block_components[result_blocks[i][4]].push_back("PS3");
      result_block_components[result_blocks[i][4]].push_back("worstPS");
      result_block_components[result_blocks[i][4]].push_back("maxShear");

      // compute values
      for (size_t ii = 0; ii < result_block_data[result_blocks[i][4]].size(); ii++)
      {
        std::vector<double> ps = ccx_results_iface.compute_principal_stresses({
        result_block_data[result_blocks[i][4]][ii][2],
        result_block_data[result_blocks[i][4]][ii][3],
        result_block_data[result_blocks[i][4]][ii][4],
        result_block_data[result_blocks[i][4]][ii][5],
        result_block_data[result_blocks[i][4]][ii][6],
        result_block_data[result_blocks[i][4]][ii][7]});

        result_block_data[result_blocks[i][4]][ii].push_back(ps[0]);
        result_block_data[result_blocks[i][4]][ii].push_back(ps[1]);
        result_block_data[result_blocks[i][4]][ii].push_back(ps[2]);
        result_block_data[result_blocks[i][4]][ii].push_back(ps[3]);
        result_block_data[result_blocks[i][4]][ii].push_back(0.5 * std::max({ps[0]-ps[2],ps[0]-ps[1],ps[1]-ps[2]}));
      }
    }

    //strains
    if (this->erase_whitespace(result_block_type[result_blocks[i][2]])=="strains") 
    {
      // add components
      result_block_components[result_blocks[i][4]].push_back("mises");

      // compute values      
      for (size_t ii = 0; ii < result_block_data[result_blocks[i][4]].size(); ii++)
      {
        result_block_data[result_blocks[i][4]][ii].push_back(ccx_results_iface.compute_von_mises_strain({
        result_block_data[result_blocks[i][4]][ii][2],
        result_block_data[result_blocks[i][4]][ii][3],
        result_block_data[result_blocks[i][4]][ii][4],
        result_block_data[result_blocks[i][4]][ii][5],
        result_block_data[result_blocks[i][4]][ii][6],
        result_block_data[result_blocks[i][4]][ii][7]}));
      }

      // add components
      result_block_components[result_blocks[i][4]].push_back("PE1");
      result_block_components[result_blocks[i][4]].push_back("PE2");
      result_block_components[result_blocks[i][4]].push_back("PE3");
      result_block_components[result_blocks[i][4]].push_back("worstPE");
      result_block_components[result_blocks[i][4]].push_back("maxShear");

      // compute values
      for (size_t ii = 0; ii < result_block_data[result_blocks[i][4]].size(); ii++)
      {
        std::vector<double> pe = ccx_results_iface.compute_principal_strains({
        result_block_data[result_blocks[i][4]][ii][2],
        result_block_data[result_blocks[i][4]][ii][3],
        result_block_data[result_blocks[i][4]][ii][4],
        result_block_data[result_blocks[i][4]][ii][5],
        result_block_data[result_blocks[i][4]][ii][6],
        result_block_data[result_blocks[i][4]][ii][7]});

        result_block_data[result_blocks[i][4]][ii].push_back(pe[0]);
        result_block_data[result_blocks[i][4]][ii].push_back(pe[1]);
        result_block_data[result_blocks[i][4]][ii].push_back(pe[2]);
        result_block_data[result_blocks[i][4]][ii].push_back(pe[3]);
        result_block_data[result_blocks[i][4]][ii].push_back(0.5 * std::max({pe[0]-pe[2],pe[0]-pe[1],pe[1]-pe[2]}));
      }
    }
     
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    if (duration > 500)
    {
      progressbar->percent(double(i+1)/double(result_blocks.size()));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }

  this->check_element_sets();

  // sorting for faster search
  for (size_t i = 0; i < result_block_c1_data.size(); i++)
  {
    std::vector<int> tmp_c1;
    std::vector<int> tmp_result_block_c1_data_id;
    std::vector<int> tmp_result_block_c1_data_type;

    for (size_t ii = 0; ii < result_block_c1_data[i].size(); ii++)
    {
      tmp_c1.push_back(result_block_c1_data[i][ii][0]);
      tmp_result_block_c1_data_id.push_back(result_block_c1_data[i][ii][1]);
      tmp_result_block_c1_data_type.push_back(result_block_c1_data[i][ii][2]);
    }  
    auto p = sort_permutation(tmp_c1);
    this->apply_permutation(tmp_c1, p);
    this->apply_permutation(tmp_result_block_c1_data_id, p);
    this->apply_permutation(tmp_result_block_c1_data_type, p);
    sorted_c1.push_back(tmp_c1);
    sorted_result_block_c1_data_id.push_back(tmp_result_block_c1_data_id);
    sorted_result_block_c1_data_type.push_back(tmp_result_block_c1_data_type);
  }

  progressbar->end();

  //PRINT_INFO("%s", log.c_str());
  //print_data();

  StopWatch.total("Duration of reading DAT (single-thread) [ms]: ");

  return true;
}

bool CoreResultsDat::read_parallel()
{
  StopWatch StopWatch;
  int max_threads = ccx_uo.mConverterThreads;
  std::vector<std::thread> ReadThreads; // vector to contain threads for reading frd
  int maxlines = 0;
  int currentline = 0;
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
    progressbar->start(0,100,"Scanning Results dat");
    while (std::getline(dat,datline))
    { 
      ++maxlines;
      if(dat.eof()){
        break;
      }
    }
    progressbar->end();
    dat.close();
    dat.open(this->filepath);

    progressbar->start(0,100,"Reading Result Headers DAT");
    t_start = std::chrono::high_resolution_clock::now();
    
    log = "";
    while (dat)
    {
      if ((!std::getline(dat,datline))||(dat.eof()))
      {
        break;
      }
      ++currentline;
  
      const auto t_end = std::chrono::high_resolution_clock::now();
      int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
      if (duration > 500)
      {
        progressbar->percent(double(currentline)/double(maxlines));
        progressbar->check_interrupt();
        t_start = std::chrono::high_resolution_clock::now();
      }
      
      // if whitespace line, get next line until not a whitespace line
      while(this->is_whitespace(datline))
      {
        if ((!std::getline(dat,datline))||(dat.eof()))
        {
          break;
        }
        ++currentline;
      }

      dat_array = this->split_line(datline);
      int current_block_data_id = int(result_blocks.size())-1;
      
      if (dat_array.size()!=0)
      {
        //first lets check if the step or increment needs an update
        this->check_step(dat_array);
        //second lets check if the mode is still valid
        this->check_mode(dat_array);
        //log = "current_line: " + std::to_string(currentline) + " dat_array: " + std::to_string(dat_array.size()) + " \n";
        //PRINT_INFO("%s", log.c_str());

        if (dat.eof())
        {
          break;
        }else if (current_read_mode == 1)
        {
          this->read_header(dat_array);
        } else if (current_read_mode == 2)
        {
          //this->read_line(dat_array);
          this->dat_arrays[current_block_data_id].push_back(dat_array);
        } else if (current_read_mode == 10)
        {
          // do nothing here
        } else if (current_read_mode == 11)
        {
          this->header_emas(dat_array);
        } else if (current_read_mode == 102)
        {
          // Buckling, save data extra
          this->read_line_buckle(dat_array);
        } else if (current_read_mode == 111)
        {
          this->header_section_1(dat_array);
        } else if (current_read_mode == 112)
        {
          this->header_section_2(dat_array);
        } else if (current_read_mode == 114)
        {
          this->header_section_3(dat_array);
        } else if (current_read_mode == 116)
        {
          this->header_section_4(dat_array);
        } else if (current_read_mode == 118)
        {
          this->header_section_5(dat_array);
        } else if (current_read_mode == 113)
        {
          this->read_line_section_1(dat_array);
        } else if (current_read_mode == 115)
        {
          this->read_line_section_2(dat_array);
        } else if (current_read_mode == 117)
        {
          this->read_line_section_3(dat_array);
        } else if (current_read_mode == 119)
        {
          this->read_line_section_4(dat_array);
        }
      }
    }
  }
  dat.close();
  progressbar->end();

  // processing data lines
  /*
  for (size_t i = 0; i < dat_arrays.size(); i++)
  {
    this->read_lines_thread(i);
  }
  */
  progressbar->start(0,100,"Reading Result Blocks DAT");

  this->progress = std::vector<int>(max_threads + 1, 0);
  progress[max_threads] = get_result_block_lines(dat_arrays);

  int loop_c = 0;
  int number_of_result_blocks = int(dat_arrays.size());
  /*
  while (number_of_result_blocks > 0)
  {
    if (number_of_result_blocks > max_threads)
    {
      for (size_t i = 0; i < max_threads; i++)
      { 
        ReadThreads.push_back(std::thread(&CoreResultsDat::read_lines_thread, this,loop_c*max_threads+i,i));
      }
    }else{
      for (size_t i = 0; i < number_of_result_blocks; i++)
      { 
        ReadThreads.push_back(std::thread(&CoreResultsDat::read_lines_thread, this,loop_c*max_threads+i,i));
      }
    }
    // wait till all threads are finished
    int it=0;
    while (ReadThreads.size()>0)
    {
      if (ReadThreads[it].joinable())
      {
        ReadThreads[it].join();
        ReadThreads.erase(ReadThreads.begin() + it);
        number_of_result_blocks = number_of_result_blocks - 1;
        it=-1;
      }
      if (it==ReadThreads.size())
      {
        it=-1;
      }
      update_progressbar();
      ++it;
    }
    ++loop_c;
  }
  */
  
  ThreadPool tp;
  
  tp.start(max_threads);
  for (size_t i = 0; i < number_of_result_blocks; i++)
  {
      std::function<void()> f = std::bind(&CoreResultsDat::read_lines_thread, this,i,0);
      tp.queueJob(f);
  }
  while(tp.busy())
  {
    update_progressbar();
  }
  tp.stop();

  /*
  log = "result_blocks.size() " + std::to_string(result_blocks.size()) + "\n";
  PRINT_INFO("%s", log.c_str());
  log = "result_block_data.size() " + std::to_string(result_block_data.size()) + "\n";
  PRINT_INFO("%s", log.c_str());

  log = "number of blocks " + std::to_string(dat_arrays.size()) + "\n";
  PRINT_INFO("%s", log.c_str());

  */

  /*
  for (size_t i = 0; i < result_blocks.size(); i++)
  {    
    log = "block id " + std::to_string(i) + " data id " + std::to_string(result_blocks[i][4]) + "\n";
    PRINT_INFO("%s", log.c_str());
  }
  
  for (size_t i = 0; i < result_block_c1_data.size(); i++)
  {    
    //log = "block id " + std::to_string(i) + " type " + std::to_string(result_block_c1_data[i][0][2]) + "\n";
    log = "block id " + std::to_string(i) + " size " + std::to_string(result_block_c1_data[i].size()) + "\n";
    PRINT_INFO("%s", log.c_str());
  }
  */

  // for stress/strain data compute extra components
  
  progressbar->start(0,100,"Computing pre-defined calculations for Results DAT");
  
  this->progress = std::vector<int>(max_threads + 1, 0);
  progress[max_threads] = get_result_blocks_predefined();

  loop_c = 0;
  number_of_result_blocks = int(result_blocks.size());
  /*
  while (number_of_result_blocks > 0)
  {
    if (number_of_result_blocks > max_threads)
    {
      for (size_t i = 0; i < max_threads; i++)
      { 
        ReadThreads.push_back(std::thread(&CoreResultsDat::compute_predefined, this,loop_c*max_threads+i,i));
      }
    }else{
      for (size_t i = 0; i < number_of_result_blocks; i++)
      { 
        ReadThreads.push_back(std::thread(&CoreResultsDat::compute_predefined, this,loop_c*max_threads+i,i));
      }
    }
    // wait till all threads are finished
    int it=0;
    while (ReadThreads.size()>0)
    {
      if (ReadThreads[it].joinable())
      {
        ReadThreads[it].join();
        ReadThreads.erase(ReadThreads.begin() + it);
        number_of_result_blocks = number_of_result_blocks - 1;
        it=-1;
      }
      if (it==ReadThreads.size())
      {
        it=-1;
      }
      update_progressbar();
      ++it;
    }
    ++loop_c;
  }
  */

  ThreadPool tp2;
  tp2.start(max_threads);
  for (size_t i = 0; i < number_of_result_blocks; i++)
  {
      std::function<void()> f = std::bind(&CoreResultsDat::compute_predefined, this,i,0);
      tp2.queueJob(f);
  }
  while(tp2.busy())
  {
    update_progressbar();
  }
  tp2.stop();
  
  this->check_element_sets();

  // sorting for faster search
  loop_c = 0;
  number_of_result_blocks = int(result_block_c1_data.size());
  for (size_t i = 0; i < result_block_c1_data.size(); i++)
  {
    std::vector<int> tmp_c1;
    std::vector<int> tmp_result_block_c1_data_id;
    std::vector<int> tmp_result_block_c1_data_type;
    sorted_c1.push_back(tmp_c1);
    sorted_result_block_c1_data_id.push_back(tmp_result_block_c1_data_id);
    sorted_result_block_c1_data_type.push_back(tmp_result_block_c1_data_type);
  }

  while (number_of_result_blocks > 0)
  {
    if (number_of_result_blocks > max_threads)
    {
      for (size_t i = 0; i < max_threads; i++)
      { 
        ReadThreads.push_back(std::thread(&CoreResultsDat::sort_data, this,loop_c*max_threads+i));
      }
    }else{
      for (size_t i = 0; i < number_of_result_blocks; i++)
      { 
        ReadThreads.push_back(std::thread(&CoreResultsDat::sort_data, this,loop_c*max_threads+i));
      }
    }
    // wait till all threads are finished
    for (size_t i = 0; i < ReadThreads.size(); i++)
    { 
      ReadThreads[i].join();
    }
    number_of_result_blocks = number_of_result_blocks - int(ReadThreads.size());
    ++loop_c;
    ReadThreads.clear();
  }

  progressbar->end();
  
  //PRINT_INFO("%s", log.c_str());
  //print_data();

  StopWatch.total("Duration of reading DAT (legacy parallel) [ms]: ");
  return true;
}


bool CoreResultsDat::read_parallel_boost()
{
  StopWatch stopwatch;
  if (!is_initialized || !progressbar)
  {
    PRINT_ERROR("DAT reader is not initialized.\n");
    return false;
  }
  if (ccx_uo.mConverterThreads < 1)
  {
    PRINT_ERROR("DAT reader requires at least one converter thread.\n");
    return false;
  }
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open())
  {
    PRINT_ERROR("Cannot open DAT file: %s\n", filepath.c_str());
    return false;
  }
  DatReadState state;
  if (!ResultsReaderPipeline::file_size(file, state.file_size))
  {
    PRINT_ERROR("Cannot determine DAT file size.\n");
    return false;
  }

  ResultsReaderPipeline::BufferedInput input(file, ResultsReaderPipeline::max_line_bytes);
  clear();
  progressbar->start(0, 100, "Reading Results DAT");
  ResultReadTimings::Phases phases;
  bool success = ResultsReaderPipeline::run_raw<DatRawReadTask>(
      static_cast<std::size_t>(ccx_uo.mConverterThreads), "DAT", phases,
      [&](DatRawReadBatch& batch, bool& finished)
      {
        return read_next_raw_batch(input, state, batch, finished);
      },
      [](const DatRawReadBatch& batch, const std::atomic<bool>& cancelled)
      {
        return parse_raw_batch(batch, cancelled);
      },
      [&](DatParsedBatch& batch)
      {
        return append_boost_batch(batch);
      },
      [&]()
      {
        return ResultsReaderPipeline::poll(input.consumed_bytes(), state.file_size, progressbar, state.progress_time);
      });

  if (success)
  {
    try
    {
      const auto finalize_begin = ResultReadTimings::Clock::now();
      success = finalize_boost_results();
      phases.finalize_ns += ResultReadTimings::elapsed(finalize_begin);
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("DAT finalization failed: %s\n", error.what());
      success = false;
    }
    catch (...)
    {
      PRINT_ERROR("Unexpected DAT finalization failure.\n");
      success = false;
    }
  }
  if (success)
  {
    const bool has_results = !result_tables.empty() || !result_section_data.empty()
        || std::any_of(result_block_data.begin(), result_block_data.end(), [](const auto& block) { return !block.empty(); })
        || std::any_of(buckle_data.begin(), buckle_data.end(), [](const auto& block) { return !block.empty(); });
    for (const auto& diagnostic : diagnostics)
    {
      PRINT_WARNING("DAT: %s\n", diagnostic.c_str());
    }
    if (!has_results)
    {
      PRINT_INFO("DAT file contains no results: %s\n", filepath.c_str());
    }
    progressbar->percent(1.0);
  }
  progressbar->end();
  if (!success)
  {
    clear();
  }
  if (success)
  {
    stopwatch.total("Duration of reading DAT (Boost, " + std::to_string(ccx_uo.mConverterThreads) + " threads) [ms]: ");
    ResultReadTimings::print("DAT", phases);
  }
  return success;
}

bool CoreResultsDat::read_next_raw_batch(ResultsReaderPipeline::BufferedInput& input, DatReadState& state, DatRawReadBatch& batch, bool& finished)
{
  using namespace CoreResultsDatFormat;
  finished = false;
  if (state.finished)
  {
    finished = true;
    return true;
  }
  std::size_t batch_bytes = 0;
  std::string_view raw;
  std::streamoff next_poll = input.consumed_bytes();
  const auto finish_frequency = [&]()
  {
    // No other tables are appended while this summary state is active.
    if (state.frequency_table_id >= 0 && result_tables.back().rows.empty())
    {
      result_tables.pop_back();
      diagnostics.push_back("Discarded empty DAT eigenvalue summary.");
    }
    state.frequency = DatReadState::FrequencyState::None;
    state.frequency_table_id = -1;
  };
  while (true)
  {
    if (input.consumed_bytes() >= next_poll)
    {
      if (!ResultsReaderPipeline::poll(input.consumed_bytes(), state.file_size, progressbar, state.progress_time))
      {
        return false;
      }
      next_poll = input.consumed_bytes() + 256 * 1024;
    }
    if (state.has_pending_line)
    {
      raw = state.pending_line;
      state.has_pending_line = false;
    }
    else
    {
      const auto status = input.next(raw);
      if (status != ResultsReaderPipeline::LineStatus::Line)
      {
        if (status != ResultsReaderPipeline::LineStatus::End)
        {
          PRINT_ERROR("Cannot read DAT results: I/O error or line exceeds 64 KiB.\n");
          return false;
        }
        if (state.frequency != DatReadState::FrequencyState::None)
        {
          if (state.frequency != DatReadState::FrequencyState::Rows && state.frequency != DatReadState::FrequencyState::Unsupported)
          {
            diagnostics.push_back("Discarded incomplete trailing DAT eigenvalue summary header.");
          }
          finish_frequency();
        }
        if (state.table_id >= 0 && ((state.summary_section && (result_tables.back().rows.empty() ||
          result_tables.back().rows.back().size() != result_tables.back().columns.size())) ||
          (result_tables.back().rows.empty() && (result_tables.back().name.find("total ") == 0 ||
            result_tables.back().name.find("center of gravity") == 0))))
        {
          diagnostics.push_back("Discarded incomplete trailing DAT summary/statistics block.");
          result_tables.pop_back();
        }
        if (current_read_mode >= 111 && current_read_mode <= 118)
        {
          // Section rows are committed only after all four records exist.
          if (!result_section_data.empty())
          {
            result_section_data.pop_back();
            result_section.pop_back();
          }
          diagnostics.push_back("Discarded incomplete trailing DAT section statistics.");
        }
        state.finished = true;
        finished = batch.records.empty();
        return true;
      }
      ++state.line_number;
    }
    raw = ResultReadNumbers::trim(raw);
    if (raw.empty())
    {
      continue;
    }
    const bool field_row = state.association > 0 && ResultReadNumbers::dat_record(raw);
    auto row_bytes = raw.size() + sizeof(DatRawRecord) +
      ResultsReaderPipeline::result_row_storage(state.components.size(), 3);
    if (!batch.records.empty() && (!field_row || batch.records.size() >= ResultsReaderPipeline::max_batch_lines ||
      batch_bytes + row_bytes > ResultsReaderPipeline::max_batch_bytes))
    {
      state.pending_line.assign(raw.data(), raw.size());
      state.has_pending_line = true;
      return true;
    }
    if (field_row)
    {
      if (state.variable_components)
      {
        std::string_view last;
        const auto count = ResultReadNumbers::field_count(raw, &last);
        const std::size_t identifiers = state.association == 2 ? 2 : 1;
        const bool suffix = !last.empty() && (last.front() == '_' || last == "L");
        const std::size_t size = count - (suffix ? 1 : 0);
        if (size <= identifiers)
        {
          PRINT_ERROR("DAT variable record has no values at line %zu.\n", state.line_number);
          return false;
        }
        if (size > ResultsReaderPipeline::max_batch_bytes / (sizeof(double) + sizeof(std::string)))
        {
          PRINT_ERROR("DAT component layout exceeds the reader batch limit at line %zu.\n", state.line_number);
          return false;
        }
        for (std::size_t i = identifiers; i < size; ++i)
        {
          state.components.push_back((state.result_type == "internal state variables" ? "SDV" : "D") + std::to_string(i - identifiers + 1));
        }
        result_block_components[state.data_id] = state.components;
        state.variable_components = false;
      }
      row_bytes = raw.size() + sizeof(DatRawRecord) +
        ResultsReaderPipeline::result_row_storage(state.components.size(), 3);
      if (row_bytes > ResultsReaderPipeline::max_batch_bytes)
      {
        PRINT_ERROR("DAT record exceeds the reader batch limit at line %zu.\n", state.line_number);
        return false;
      }
      if (input.consumed_bytes() >= state.file_size)
      {
        // A killed solver can leave a partial final physical row. Validate
        // only this row on the scanner; normal numeric parsing stays in Boost.
        std::string_view last;
        const auto count = ResultReadNumbers::field_count(raw, &last);
        const bool local = last == "L" && state.association == 1 && local_vector_type(state.result_type) && state.components.size() == 3;
        const bool suffix = !last.empty() && (last.front() == '_' || local);
        const std::size_t offset = state.association == 1 ? 1 : 0;
        bool valid = count == state.components.size() + offset + (suffix ? 1 : 0);
        std::size_t position = 0;
        std::string_view token;
        for (std::size_t i = 0; valid && i < count - (suffix ? 1 : 0); ++i)
        {
          double value = 0.0;
          valid = ResultReadNumbers::next_field(raw, position, token);
          if (valid && (i == 0 || (state.association == 2 && i == 1)))
          {
            int identifier = 0;
            valid = ResultReadNumbers::integer(token, identifier) && identifier > 0;
          }
          else if (valid)
          {
            valid = state.result_type == "global coordinates" ? ResultReadNumbers::real(token, value) : ResultReadNumbers::result_value(token, value);
          }
        }
        if (!valid)
        {
          diagnostics.push_back("Discarded incomplete trailing DAT field row at line " + std::to_string(state.line_number) + ".");
          state.finished = true;
          finished = batch.records.empty();
          return true;
        }
      }
      batch.association = state.association;
      ResultsReaderPipeline::append_raw_record(batch, state, raw);
      batch_bytes += row_bytes;
      continue;
    }

    const std::string line = normalize(raw);
    using FrequencyState = DatReadState::FrequencyState;
    if (frequency_title(raw))
    {
      if ((state.table_id >= 0 && state.summary_section) || (current_read_mode >= 111 && current_read_mode <= 118))
      {
        PRINT_ERROR("Incomplete DAT statistics before eigenvalue summary at line %zu.\n", state.line_number);
        return false;
      }
      finish_frequency();
      state.frequency = FrequencyState::Columns;
      state.frequency_source_line = state.line_number;
      state.association = 0;
      state.data_id = -1;
      state.table_id = -1;
      state.summary_section = false;
      current_read_mode = 0;
      continue;
    }
    if (state.frequency == FrequencyState::Columns || state.frequency == FrequencyState::Parts || state.frequency == FrequencyState::Units)
    {
      if (!frequency_heading(state.frequency, raw))
      {
        diagnostics.push_back("Unsupported DAT complex/cyclic or unrecognized eigenvalue summary layout at line " + std::to_string(state.line_number) + ": " + std::string(raw));
        state.frequency = FrequencyState::Unsupported;
        // Reprocess a new section/step rather than swallowing its heading.
        state.pending_line.assign(raw.data(), raw.size());
        state.has_pending_line = true;
        continue;
      }
      if (state.frequency == FrequencyState::Columns)
      {
        state.frequency = FrequencyState::Parts;
      }
      else if (state.frequency == FrequencyState::Parts)
      {
        state.frequency = FrequencyState::Units;
      }
      else
      {
        if (!ResultsReaderPipeline::ids_fit(result_tables.size(), 1))
        {
          PRINT_ERROR("DAT eigenvalue table index overflow.\n");
          return false;
        }
        DatResultTable table;
        table.scope = DatResultTableScope::StepSummary;
        table.key = "FREQUENCY";
        table.name = "eigenvalue/frequency summary";
        table.step = current_step;
        table.increment = -1;
        table.source_line = state.frequency_source_line;
        table.columns = frequency_columns();
        result_tables.push_back(std::move(table));
        state.frequency_table_id = static_cast<int>(result_tables.size() - 1);
        state.frequency = FrequencyState::Rows;
      }
      continue;
    }
    if (state.frequency == FrequencyState::Unsupported)
    {
      if (ResultReadNumbers::dat_record(raw) || line.find("mode no") == 0 || line.find("nodal") == 0 ||
        line.find("diameter") == 0 || line.find("real part") == 0 || line.find("(rad/time)") == 0)
      {
        continue;
      }
      finish_frequency();
    }
    if (state.frequency == FrequencyState::Rows)
    {
      if (ResultReadNumbers::dat_record(raw))
      {
        std::vector<double> values;
        auto& rows = result_tables[state.frequency_table_id].rows;
        const bool valid = frequency_row(raw, values);
        if (!valid || (!rows.empty() && values[0] <= rows.back()[0]))
        {
          if (!valid && input.consumed_bytes() >= state.file_size)
          {
            diagnostics.push_back("Discarded incomplete trailing DAT eigenvalue record.");
            finish_frequency();
            continue;
          }
          PRINT_ERROR("Invalid DAT eigenvalue record at line %zu.\n", state.line_number);
          return false;
        }
        rows.push_back(std::move(values));
        continue;
      }
      finish_frequency();
    }
    const auto definition = header(raw);
    if (definition.kind != Kind::Unknown)
    {
      if (state.table_id >= 0 && state.summary_section && (result_tables.back().rows.empty() ||
        result_tables.back().rows.back().size() != result_tables.back().columns.size()))
      {
        PRINT_ERROR("Incomplete DAT contact statistics before line %zu.\n", state.line_number);
        return false;
      }
      if (current_read_mode >= 111 && current_read_mode <= 118)
      {
        PRINT_ERROR("Incomplete DAT section statistics before line %zu.\n", state.line_number);
        return false;
      }
      state.association = 0;
      state.data_id = -1;
      state.table_id = -1;
      state.summary_section = definition.kind == Kind::ContactStatistics;
      current_read_mode = 0;
      if (definition.kind == Kind::Field)
      {
        if (!ResultsReaderPipeline::ids_fit(result_blocks.size(), 1))
        {
          PRINT_ERROR("DAT result block index overflow.\n");
          return false;
        }
        state.association = definition.association;
        state.variable_components = definition.variable;
        state.components = definition.components;
        state.result_type = definition.table.key == "EMAS" ? "emas" : definition.table.name;
        state.data_id = static_cast<int>(result_block_data.size());
        total_times.push_back(definition.table.time);
        result_blocks.push_back({static_cast<int>(result_blocks.size() + 1), static_cast<int>(total_times.size() - 1),
          get_current_result_block_type(state.result_type), get_current_result_block_set(definition.table.set_name),
          state.data_id, current_step, current_increment});
        result_block_components.push_back(state.components);
        result_block_data.emplace_back();
        result_block_c1_data.emplace_back();
        dat_arrays.emplace_back();
      }
      else if (definition.kind == Kind::Section)
      {
        current_read_mode = 111;
        if (!header_section_1(split_line(std::string(raw))))
        {
          return false;
        }
      }
      else
      {
        if (!ResultsReaderPipeline::ids_fit(result_tables.size(), 1))
        {
          PRINT_ERROR("DAT table index overflow.\n");
          return false;
        }
        auto table = definition.table;
        table.step = current_step;
        table.increment = current_increment;
        table.source_line = state.line_number;
        result_tables.push_back(std::move(table));
        state.table_id = static_cast<int>(result_tables.size() - 1);
        state.table_width = result_tables.back().columns.size();
      }
      continue;
    }

    if (state.table_id >= 0)
    {
      auto& table = result_tables[state.table_id];
      std::vector<double> values;
      if (numeric(raw, values))
      {
        if (state.summary_section)
        {
          if (table.rows.empty())
          {
            table.rows.emplace_back();
          }
          auto& row = table.rows.back();
          const std::size_t expected = row.size() == 0 || row.size() == 6 ? 6 : row.size() == 12 || row.size() == 15 ? 3 : 0;
          if (values.size() != expected || row.size() + values.size() > state.table_width)
          {
            if (input.consumed_bytes() >= state.file_size)
            {
              result_tables.pop_back();
              diagnostics.push_back("Discarded incomplete trailing DAT contact statistics.");
              state.finished = true;
              finished = batch.records.empty();
              return true;
            }
            PRINT_ERROR("Invalid DAT contact statistics at line %zu.\n", state.line_number);
            return false;
          }
          row.insert(row.end(), values.begin(), values.end());
          if (row.size() == state.table_width)
          {
            if (!identifiers(table.columns, row))
            {
              PRINT_ERROR("Invalid DAT statistics coordinates at line %zu.\n", state.line_number);
              return false;
            }
            state.table_id = -1;
            state.summary_section = false;
          }
        }
        else
        {
          if (values.size() != state.table_width || !identifiers(table.columns, values))
          {
            if (input.consumed_bytes() >= state.file_size)
            {
              diagnostics.push_back("Discarded incomplete trailing DAT table row at line " + std::to_string(state.line_number) + ".");
              state.finished = true;
              finished = batch.records.empty();
              return true;
            }
            PRINT_ERROR("DAT table width mismatch at line %zu.\n", state.line_number);
            return false;
          }
          table.rows.push_back(std::move(values));
          if (table.name.find("total ") == 0 || table.name.find("center of gravity") == 0)
          {
            state.table_id = -1;
          }
        }
        continue;
      }
      if (!raw.empty() && ((raw.front() >= '0' && raw.front() <= '9') || raw.front() == '-' || raw.front() == '+' || raw.front() == '.'))
      {
        if (input.consumed_bytes() >= state.file_size)
        {
          if (state.summary_section)
          {
            result_tables.pop_back();
          }
          diagnostics.push_back("Discarded incomplete trailing DAT table record at line " + std::to_string(state.line_number) + ".");
          state.finished = true;
          finished = batch.records.empty();
          return true;
        }
        PRINT_ERROR("Invalid numeric DAT table record at line %zu.\n", state.line_number);
        return false;
      }
      if (state.summary_section)
      {
        if (line.find(" and time ") != std::string::npos || line.find("s t e p ") == 0 || line.find("increment ") == 0)
        {
          PRINT_ERROR("Incomplete DAT contact statistics before line %zu.\n", state.line_number);
          return false;
        }
        if (line.find("total normal surface force") == 0)
        {
          table.key = "CFN";
        }
        else if (line.find("total shear surface force") == 0)
        {
          table.key = "CFS";
        }
        if (line.find(" and time ") == std::string::npos)
        {
          continue; // Named separators between the four statistics records.
        }
      }
      else if (table.key == "FLUX" && line.find("el fa int") == 0)
      {
        continue;
      }
      state.table_id = -1;
      state.summary_section = false;
    }

    auto fields = split_line(std::string(raw));
    if (fields.empty())
    {
      continue;
    }
    if (fields.size() == 4 && fields[0] == "S" && fields[1] == "T" && fields[2] == "E" && fields[3] == "P")
    {
      PRINT_ERROR("Incomplete DAT step header at line %zu.\n", state.line_number);
      return false;
    }
    if (check_step(fields))
    {
      if (current_read_mode >= 111 && current_read_mode <= 118)
      {
        PRINT_ERROR("Incomplete DAT section statistics before a step/increment header.\n");
        return false;
      }
      state.association = 0;
      current_read_mode = 0;
      continue;
    }
    if (current_read_mode >= 111 && current_read_mode <= 118)
    {
      ++current_read_mode;
      if (current_read_mode == 113 || current_read_mode == 115 || current_read_mode == 117 || current_read_mode == 119)
      {
        const std::size_t count = current_read_mode == 117 ? 3 : current_read_mode == 119 ? 5 : 6;
        std::vector<double> values;
        if (!numeric(raw, values) || values.size() != count)
        {
          PRINT_ERROR("Invalid DAT section record at line %zu.\n", state.line_number);
          return false;
        }
        if (result_section_data.empty() || result_section_data.back().size() != 22 ||
          (current_read_mode == 115 && !identifiers({"CenterX", "CenterY", "CenterZ", "NormalX", "NormalY", "NormalZ"}, values)))
        {
          PRINT_ERROR("Invalid DAT section metadata at line %zu.\n", state.line_number);
          return false;
        }
        const std::size_t offset = current_read_mode == 113 ? 2 : current_read_mode == 115 ? 8 : current_read_mode == 117 ? 14 : 17;
        std::copy(values.begin(), values.end(), result_section_data.back().begin() + offset);
      }
      continue;
    }
    if (fields[0] == "FACTOR")
    {
      current_read_mode = 101;
      state.association = 0;
      continue;
    }
    if ((current_read_mode == 101 || current_read_mode == 102) && ResultReadNumbers::dat_record(raw))
    {
      if (current_read_mode == 101)
      {
        buckle_data.emplace_back();
      }
      current_read_mode = 102;
      int mode = 0;
      double factor = 0;
      if (fields.size() != 2 || !ResultReadNumbers::integer(fields[0], mode) || mode <= 0 ||
        !ResultReadNumbers::result_value(fields[1], factor))
      {
        PRINT_ERROR("Invalid DAT buckling record at line %zu.\n", state.line_number);
        return false;
      }
      buckle_data.back().push_back({static_cast<double>(mode), factor});
      continue;
    }
    // An unrecognized heading must never leave the previous field active.
    state.association = 0;
    state.data_id = -1;
    current_read_mode = 0;
    if (line.find("time ") != std::string::npos || line.find("for set ") != std::string::npos || line.find("statistics for ") == 0)
    {
      diagnostics.push_back("Unsupported DAT result header at line " + std::to_string(state.line_number) + ": " + std::string(raw));
    }
  }
}


DatParsedBatch CoreResultsDat::parse_raw_batch(const DatRawReadBatch& batch, const std::atomic<bool>& cancelled)
{
  DatParsedBatch output;
  output.data_id = batch.data_id;
  output.first_line = batch.first_line;

  CoreResultsDat reader;
  reader.result_block_type.push_back(batch.result_type);
  reader.result_block_components.push_back(batch.components);
  reader.result_blocks.push_back({0, 0, 0, 0, 0, 0, 0});
  reader.result_block_data.emplace_back();
  reader.result_block_c1_data.emplace_back();
  reader.progress.assign(1, 0);

  auto& values = reader.result_block_data[0];
  auto& mapping = reader.result_block_c1_data[0];
  values.reserve(batch.records.size());
  mapping.reserve(batch.records.size());
  const std::size_t component_count = batch.components.size();
  const auto type = reader.erase_whitespace(batch.result_type);
  const bool derived = type == "stresses" || type == "strains";
  if (component_count == 0 || (derived && component_count < 8))
  {
    output.error = "Invalid DAT component layout.";
    return output;
  }

  const auto parse_begin = ResultReadTimings::Clock::now();
  for (const auto& record : batch.records)
  {
    if (cancelled.load(std::memory_order_relaxed))
    {
      output.error = "Reading cancelled.";
      return output;
    }
    if (record.offset > batch.text.size() || record.length > batch.text.size() - record.offset)
    {
      output.error = "Invalid DAT raw record range.";
      return output;
    }

    const std::string_view line(batch.text.data() + record.offset, record.length);
    std::string_view last;
    const auto count = ResultReadNumbers::field_count(line, &last);
    std::size_t position = 0;
    std::string_view token;
    int identifier = 0;
    if (!ResultReadNumbers::next_field(line, position, token) || !ResultReadNumbers::integer(token, identifier) || identifier <= 0)
    {
      output.error = "Invalid DAT identifier at line " + std::to_string(record.source_line);
      return output;
    }

    const bool shell_suffix = !last.empty() && last.front() == '_';
    const bool local = last == "L" && batch.association == 1 &&
      CoreResultsDatFormat::local_vector_type(type) && component_count == 3;
    const bool elemental = batch.association > 0 ? batch.association != 1 : count == component_count || shell_suffix;
    const std::size_t offset = elemental ? 0 : 1;
    if (count != component_count + offset + (shell_suffix || local ? 1 : 0))
    {
      output.error = "Incomplete DAT result row at line " + std::to_string(record.source_line);
      return output;
    }
    int identifier_type = batch.association;
    if (identifier_type == 0)
    {
      identifier_type = elemental ? (component_count == 2 || type == "emas" ? 3 : 2) : 1;
    }
    std::vector<double> row(component_count);
    row.reserve(component_count + (derived ? 6 : 0));
    for (std::size_t component = 0; component < component_count; ++component)
    {
      // Element/IP IDs remain part of the stored elemental row. Reuse the
      // already consumed first token, then advance the non-owning cursor.
      if ((component != 0 || !elemental) && !ResultReadNumbers::next_field(line, position, token))
      {
        output.error = "Incomplete DAT result row at line " + std::to_string(record.source_line);
        return output;
      }
      const bool identity = elemental && (component == 0 || (identifier_type == 2 && component == 1));
      int numeric_id = 0;
      const bool valid = identity ? ResultReadNumbers::integer(token, numeric_id) && numeric_id > 0 :
        (type == "globalcoordinates" ? ResultReadNumbers::real(token, row[component]) : ResultReadNumbers::result_value(token, row[component]));
      if (!valid)
      {
        output.error = "Invalid DAT value at line " + std::to_string(record.source_line);
        return output;
      }
      if (identity)
      {
        row[component] = numeric_id;
      }
    }
    if (local)
    {
      if (output.local_rows.empty())
      {
        output.first_local_source_line = record.source_line;
      }
      row.insert(row.begin(), identifier);
      output.local_rows.push_back(std::move(row));
      continue;
    }
    const int local_id = static_cast<int>(values.size());
    values.push_back(std::move(row));
    mapping.push_back({identifier, local_id, identifier_type});
  }
  output.parse_ns = ResultReadTimings::elapsed(parse_begin);
  if (cancelled.load(std::memory_order_relaxed))
  {
    output.error = "Reading cancelled.";
    return output;
  }

  // Reuse numerical methods only for finite tensors. NaN/Inf remain payloads;
  // derived values are unavailable and must not enter the eigensolver.
  const auto derived_begin = ResultReadTimings::Clock::now();
  if (derived)
  {
    const bool stress = type == "stresses";
    auto& components = reader.result_block_components[0];
    const std::vector<std::string> extra = stress ?
      std::vector<std::string>{"mises", "PS1", "PS2", "PS3", "worstPS", "maxShear"} :
      std::vector<std::string>{"mises", "PE1", "PE2", "PE3", "worstPE", "maxShear"};
    components.insert(components.end(), extra.begin(), extra.end());
    CalculiXResultsInterface results;
    for (auto& row : values)
    {
      if (cancelled.load(std::memory_order_relaxed))
      {
        output.error = "Reading cancelled.";
        return output;
      }
      const std::vector<double> tensor(row.begin() + 2, row.begin() + 8);
      if (!std::all_of(tensor.begin(), tensor.end(), [](double value)
      {
        return std::isfinite(value);
      }))
      {
        row.insert(row.end(), 6, std::numeric_limits<double>::quiet_NaN());
        continue;
      }
      row.push_back(stress ? results.compute_von_mises_stress(tensor) : results.compute_von_mises_strain(tensor));
      const auto principal = stress ? results.compute_principal_stresses(tensor) : results.compute_principal_strains(tensor);
      if (principal.size() < 4)
      {
        output.error = "Invalid DAT principal component result.";
        return output;
      }
      row.insert(row.end(), principal.begin(), principal.begin() + 4);
      row.push_back(0.5 * std::max({principal[0] - principal[2], principal[0] - principal[1], principal[1] - principal[2]}));
    }
  }
  output.derived_ns = ResultReadTimings::elapsed(derived_begin);
  output.components = std::move(reader.result_block_components[0]);
  output.values = std::move(values);
  output.c1_data = std::move(mapping);
  output.valid = true;
  return output;
}

bool CoreResultsDat::read_next_boost_batch(std::ifstream& file, DatReadState& state, DatReadBatch& batch, bool& finished)
{
  finished = false;
  if (state.finished)
  {
    finished = true;
    return true;
  }

  std::size_t batch_bytes = 0;
  std::string raw;
  while (true)
  {
    if (!ResultsReaderPipeline::poll(file, state.file_size, progressbar, state.progress_time))
    {
      return false;
    }
    if (state.has_pending_line)
    {
      raw = std::move(state.pending_line);
      state.has_pending_line = false;
    }
    else
    {
      if (!ResultsReaderPipeline::read_line(file, raw))
      {
        if (!file.eof())
        {
          PRINT_ERROR("Cannot read DAT results: I/O error or line exceeds 64 KiB.\n");
          return false;
        }
        if (current_read_mode >= 111 && current_read_mode <= 118)
        {
          PRINT_ERROR("Incomplete DAT section output at end of file.\n");
          return false;
        }
        state.finished = true;
        finished = batch.lines.empty();
        return true;
      }
      ++state.line_number;
    }

    if (raw.size() > ResultsReaderPipeline::max_line_bytes)
    {
      PRINT_ERROR("DAT line %zu exceeds the supported line size.\n", state.line_number);
      return false;
    }
    if (is_whitespace(raw))
    {
      continue;
    }
    auto fields = split_line(raw);
    if (fields.empty() || fields.size() > ResultsReaderPipeline::max_fields)
    {
      PRINT_ERROR("Invalid DAT record at line %zu.\n", state.line_number);
      return false;
    }

    const bool numeric = is_number(fields[0]);
    const auto row_bytes = ResultsReaderPipeline::line_storage(fields) + ResultsReaderPipeline::result_row_storage(state.components.size(), 3);
    if (!batch.lines.empty())
    {
      if (!numeric || current_read_mode != 2 || batch.lines.size() >= ResultsReaderPipeline::max_batch_lines || batch_bytes + row_bytes > ResultsReaderPipeline::max_batch_bytes)
      {
        state.pending_line = std::move(raw);
        state.has_pending_line = true;
        return true;
      }
    }

    if (fields.size() == 4 && fields[0] == "S" && fields[1] == "T" && fields[2] == "E" && fields[3] == "P")
    {
      PRINT_ERROR("Incomplete DAT step header at line %zu.\n", state.line_number);
      return false;
    }
    check_step(fields);
    check_mode(fields);

    if (current_read_mode == 1 || current_read_mode == 11)
    {
      const auto set = std::find(fields.begin(), fields.end(), "set");
      if (set == fields.end() || std::distance(fields.begin(), set) < 2 || std::next(set) == fields.end())
      {
        PRINT_ERROR("Invalid DAT result header at line %zu.\n", state.line_number);
        return false;
      }
      if (!ResultsReaderPipeline::ids_fit(result_blocks.size(), 1))
      {
        PRINT_ERROR("DAT result block index overflow.\n");
        return false;
      }
      const bool valid_header = current_read_mode == 11 ? header_emas(fields) : read_header(fields);
      if (!valid_header)
      {
        return false;
      }
      state.data_id = result_blocks.back()[4];
      state.result_type = result_block_type[result_blocks.back()[2]];
      state.components = result_block_components[state.data_id];
      if (state.components.size() > ResultsReaderPipeline::max_fields)
      {
        PRINT_ERROR("DAT result has too many components.\n");
        return false;
      }
    }
    else if (current_read_mode == 2)
    {
      if (state.data_id < 0 || state.components.empty())
      {
        PRINT_ERROR("DAT result row has no header at line %zu.\n", state.line_number);
        return false;
      }
      if (batch.lines.empty())
      {
        batch.data_id = state.data_id;
        batch.first_line = state.line_number;
        batch.result_type = state.result_type;
        batch.components = state.components;
      }
      batch_bytes += row_bytes;
      batch.lines.push_back(std::move(fields));
    }
    else if (current_read_mode == 102)
    {
      if (fields.size() < 2 || buckle_data.empty())
      {
        PRINT_ERROR("Invalid DAT buckling row at line %zu.\n", state.line_number);
        return false;
      }
      if (!read_line_buckle(fields))
      {
        return false;
      }
    }
    else if (current_read_mode == 111)
    {
      if (fields.size() < 8)
      {
        PRINT_ERROR("Incomplete DAT section header at line %zu.\n", state.line_number);
        return false;
      }
      if (!header_section_1(fields))
      {
        return false;
      }
    }
    else if (current_read_mode == 112)
    {
      if (!header_section_2(fields))
      {
        return false;
      }
    }
    else if (current_read_mode == 114)
    {
      if (!header_section_3(fields))
      {
        return false;
      }
    }
    else if (current_read_mode == 116)
    {
      if (!header_section_4(fields))
      {
        return false;
      }
    }
    else if (current_read_mode == 118)
    {
      if (!header_section_5(fields))
      {
        return false;
      }
    }
    else if (current_read_mode == 113 || current_read_mode == 115 || current_read_mode == 117 || current_read_mode == 119)
    {
      const std::size_t expected = current_read_mode == 117 ? 3 : current_read_mode == 119 ? 5 : 6;
      if (result_section_data.empty() || fields.size() != expected)
      {
        PRINT_ERROR("Invalid DAT section row at line %zu.\n", state.line_number);
        return false;
      }
      bool valid = false;
      if (current_read_mode == 113)
      {
        valid = read_line_section_1(fields);
      }
      else if (current_read_mode == 115)
      {
        valid = read_line_section_2(fields);
      }
      else if (current_read_mode == 117)
      {
        valid = read_line_section_3(fields);
      }
      else
      {
        valid = read_line_section_4(fields);
      }
      if (!valid)
      {
        return false;
      }
    }
  }
}

DatParsedBatch CoreResultsDat::parse_boost_batch(const DatReadBatch& batch, const std::atomic<bool>& cancelled)
{
  DatParsedBatch output;
  output.data_id = batch.data_id;
  output.first_line = batch.first_line;

  // Existing conversion and derived-value functions operate only on this
  // private reader; domain interfaces are stateless and need no initialization.
  CoreResultsDat reader;
  reader.result_block_type.push_back(batch.result_type);
  reader.result_block_components.push_back(batch.components);
  reader.result_blocks.push_back({0, 0, 0, 0, 0, 0, 0});
  reader.result_block_data.emplace_back();
  reader.result_block_c1_data.emplace_back();
  reader.progress.assign(1, 0);
  reader.result_block_data[0].reserve(batch.lines.size());
  reader.result_block_c1_data[0].reserve(batch.lines.size());

  const auto component_count = batch.components.size();
  const auto type = reader.erase_whitespace(batch.result_type);
  const bool derived = type == "stresses" || type == "strains";
  if (component_count == 0 || (derived && component_count < 8))
  {
    output.error = "Invalid DAT component layout.";
    return output;
  }
  for (const auto& line : batch.lines)
  {
    if (cancelled.load(std::memory_order_relaxed))
    {
      output.error = "Reading cancelled.";
      return output;
    }
    if (line.empty() || line.back().empty())
    {
      output.error = "Empty DAT result row.";
      return output;
    }
    const bool shell_suffix = line.back()[0] == '_';
    const bool elemental = line.size() == component_count || shell_suffix;
    const auto required = component_count + (elemental ? 0 : 1);
    if (line.size() < required || (shell_suffix && line.size() < component_count + 1))
    {
      output.error = "Incomplete DAT result row.";
      return output;
    }
    if (!reader.read_line(line))
    {
      output.error = "Cannot convert DAT result row.";
      return output;
    }
  }
  if (cancelled.load(std::memory_order_relaxed))
  {
    output.error = "Reading cancelled.";
    return output;
  }
  if (!reader.compute_predefined(0, 0))
  {
    output.error = "Cannot compute DAT derived components.";
    return output;
  }
  output.components = std::move(reader.result_block_components[0]);
  output.values = std::move(reader.result_block_data[0]);
  output.c1_data = std::move(reader.result_block_c1_data[0]);
  output.valid = true;
  return output;
}

bool CoreResultsDat::append_boost_batch(DatParsedBatch& batch)
{
  if (batch.data_id < 0 || static_cast<std::size_t>(batch.data_id) >= result_block_data.size())
  {
    PRINT_ERROR("Invalid DAT result destination.\n");
    return false;
  }
  auto& values = result_block_data[batch.data_id];
  auto& mapping = result_block_c1_data[batch.data_id];
  if (batch.values.size() != batch.c1_data.size() || !ResultsReaderPipeline::ids_fit(values.size(), batch.values.size()))
  {
    PRINT_ERROR("Invalid DAT batch size or result index overflow.\n");
    return false;
  }
  if (!batch.local_rows.empty())
  {
    auto found = pending_local_tables.find(batch.data_id);
    if (found == pending_local_tables.end())
    {
      const auto block = std::find_if(result_blocks.begin(), result_blocks.end(), [&](const std::vector<int>& value)
      {
        return value.size() >= 7 && value[4] == batch.data_id;
      });
      if (block == result_blocks.end() || (*block)[1] < 0 || static_cast<std::size_t>((*block)[1]) >= total_times.size() ||
        (*block)[2] < 0 || static_cast<std::size_t>((*block)[2]) >= result_block_type.size() ||
        (*block)[3] < 0 || static_cast<std::size_t>((*block)[3]) >= result_block_set.size())
      {
        PRINT_ERROR("Invalid DAT local-vector metadata.\n");
        return false;
      }
      const auto type = erase_whitespace(result_block_type[(*block)[2]]);
      DatResultTable table;
      table.key = type == "displacements" ? "U" : (type == "forces" ? "RF" : "VF");
      table.name = result_block_type[(*block)[2]] + " (local coordinates)";
      table.coordinate_system = "Local";
      table.set_name = result_block_set[(*block)[3]];
      table.step = (*block)[5];
      table.increment = (*block)[6];
      table.time = total_times[(*block)[1]];
      table.source_line = batch.first_local_source_line;
      table.columns = {"NodeId"};
      table.columns.insert(table.columns.end(), batch.components.begin(), batch.components.end());
      found = pending_local_tables.emplace(batch.data_id, std::move(table)).first;
    }
    auto& rows = found->second.rows;
    rows.insert(rows.end(), std::make_move_iterator(batch.local_rows.begin()), std::make_move_iterator(batch.local_rows.end()));
  }
  const int offset = static_cast<int>(values.size());
  for (std::size_t i = 0; i < batch.values.size(); ++i)
  {
    batch.c1_data[i][1] += offset;
    values.push_back(std::move(batch.values[i]));
    mapping.push_back(std::move(batch.c1_data[i]));
  }
  result_block_components[batch.data_id] = std::move(batch.components);
  return true;
}

bool CoreResultsDat::finalize_boost_results()
{
  for (auto& entry : pending_local_tables)
  {
    result_tables.push_back(std::move(entry.second));
  }
  pending_local_tables.clear();
  // Empty result collections are valid when no DAT output was requested.
  if (!check_element_sets())
  {
    return false;
  }
  sorted_c1.resize(result_block_c1_data.size());
  sorted_result_block_c1_data_id.resize(result_block_c1_data.size());
  sorted_result_block_c1_data_type.resize(result_block_c1_data.size());
  for (std::size_t i = 0; i < result_block_c1_data.size(); ++i)
  {
    if (ResultsReaderPipeline::interrupted(progressbar))
    {
      return false;
    }
    if (!sort_data(static_cast<int>(i)))
    {
      return false;
    }
  }
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
    if (beginSpace != 0 && line[beginSpace-1] != 'E' && line[beginSpace-1] != 'e' && line[beginSpace-1] != ' ')
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
  // insert whitespaces before "for" if there is none
  beginSpace = line.find("for");
  if ((beginSpace != std::string::npos) && (beginSpace != 0))
  {
    size_t range = 3;
    if (line[beginSpace-1] != ' ')
    {
      line.replace(beginSpace,range, " for");
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
  if (number.empty())
  {
    return false;
  }
  for (size_t i = 0; i < number.length(); i++)
  {
    if (!std::isdigit(static_cast<unsigned char>(number[i])))
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

std::string CoreResultsDat::erase_whitespace(std::string line)
{
  // reduce whitespaces
  size_t beginSpace = line.find_first_of(" \t\r\f\v\n");
  while (beginSpace != std::string::npos)
  {
    size_t endSpace = line.find_first_not_of(" \t\r\f\v\n", beginSpace);
    size_t range = endSpace - beginSpace;

    line.replace(beginSpace,range, "");
    size_t newStart = beginSpace + 1;
    beginSpace = line.find_first_of(" \t\r\f\v\n",newStart);
  }
  return line;
}

bool CoreResultsDat::check_mode(std::vector<std::string> line)
{
  if (line.size() > 1 && line[0] == "B" && line[1] == "U") // Buckling!
  {
    current_read_mode = 10;
    return true;
  } else if (line[0]=="MODE") // still buckling
  {
    current_read_mode = 10;
    return true;
  } else if (line[0]=="FACTOR") // still buckling
  {
    current_read_mode = 101;
    return true;
  }
 
  //check for section print data
  if (line.size() > 4)
  { 
    if ((line[0]=="statistics")&&(line[1]=="for")&&(line[2]=="surface")&&(line[3]=="set"))
    {
      current_read_mode = 111; // read header line for section print
      return  true;
    }
  }
  if (current_read_mode == 111) // read section header 1
  {
    current_read_mode = 112; // read section header 2
    return  true;
  } else if (current_read_mode == 112) // read section header 2
  {
    current_read_mode = 113; // read section line 1
    return  true;
  } else if (current_read_mode == 113) // read section line 1
  {
    current_read_mode = 114; // read section header 3
    return  true;
  } else if (current_read_mode == 114) // read section header 3
  {
    current_read_mode = 115; // read section line 2
    return  true;
  } else if (current_read_mode == 115) // read section line 2
  {
    current_read_mode = 116; // read section header 4
    return  true;
  } else if (current_read_mode == 116) // read section header 4
  {
    current_read_mode = 117; // read section line 3
    return  true;
  } else if (current_read_mode == 117) // read section line 3
  {
    current_read_mode = 118; // read section header 5
    return  true;
  } else if (current_read_mode == 118) // read section header 5
  {
    current_read_mode = 119; // read section line 4
    return  true;
  }
  
  if (this->is_number(line[0]))
  {
    if (current_read_mode == 101) // read buckling data
    {
      current_read_mode = 102; // read buckling data lines
      // so we need to add a new data set that can be filled
      std::vector<std::vector<double>> buckle_data_tmp;
      this->buckle_data.push_back(buckle_data_tmp);
    }
    if ((current_read_mode != 10)&&(current_read_mode != 102)) // skip if should be skipped
    {
      current_read_mode = 2; // read data lines
    } 
  } else if (!this->is_number(line[0]))
  {
    if (line[0]=="mass")
    {
      current_read_mode = 11; // read header line for EMAS
      return  true;
    }
    for (size_t i = 0; i < line.size(); i++)
    {
      if (
        (line[i]=="contact")
      ||(line[i]=="statistics")
      ||(line[i]=="surface")
      ||(line[i]=="center")
      ||(line[i]=="shear")
      ||(line[i]=="total")
      //||(line[i]=="mass")
      ||(line[i]=="variables")
      )
      {
        current_read_mode = 10; // skip
        return  true;
      }
      if (line[i]=="set")
      {
        current_read_mode = 1; // read header line
        return  true;
      }
    } 
    current_read_mode = 10; // read header line
  }

  return true;
}

bool CoreResultsDat::check_step(std::vector<std::string> line)
{
  if (line.size()>4)
  {
    if ((line[0] == "S")&&(line[1] == "T")&&(line[2] == "E")&&(line[3] == "P")) // STEP
    {
      this->current_step = std::stoi(line[4]);

      //std::string log = "STEP " + line[4] + " \n";
      //PRINT_INFO("%s", log.c_str());

      return true;
    }
  }
  
  if (line.size()==2)
  {
    if ((line[0] == "INCREMENT")) // INCREMENT
    {
      this->current_increment = std::stoi(line[1]);
      
      //std::string log = "INCREMENT " + line[1] + " \n";
      //PRINT_INFO("%s", log.c_str());

      return true;
    }
  }

  return false;
}

bool CoreResultsDat::read_header(std::vector<std::string> line)
{
  CalculiXApplicationInterface ccx_application_iface;


  double total_time;
  std::string block_set;
  std::string block_type;
  int result_block_data_id;
  int result_block_type_data_id;
  int result_block_set_data_id;

  int search = -1;
  for (size_t i = 0; i < line.size(); i++)
  {
    if (line[i]=="set")
    {
      search = int(i);
    }
  } 
  if (search < 2 || static_cast<std::size_t>(search + 1) >= line.size())
  {
    PRINT_ERROR("Invalid DAT result header.\n");
    return false;
  }
  block_set = line[search + 1];
  total_time = ccx_application_iface.string_scientific_to_double(line[line.size()-1]);
  total_times.push_back(total_time);
  current_total_time_id = int(total_times.size())-1;

  block_type = "";
  for (size_t i = 0; i < search-1; i++)
  {
    block_type.append(line[i]);
    if (i!=search-2)
    {
      block_type.append(" ");
    }
  }

  std::vector<std::string> tmp_result_block_components;
  // extract components if possible
  size_t beginSpace = block_type.find_first_of("(");
  if (beginSpace != std::string::npos)
  {  
    std::string str_comp = "";
    size_t endSpace = block_type.find_first_of(")", beginSpace);
    size_t range = endSpace - beginSpace + 1;
    str_comp = block_type.substr(beginSpace + 1,range-2);
    block_type.replace(beginSpace,range, "");

    str_comp = this->erase_whitespace(str_comp);

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
  
  if (result_blocks.size()==0)
  {
    current_result_block = 1;
  }else{
    current_result_block = int(result_blocks.size()+1);
  }
  result_block_data_id = int(result_block_data.size())-1;
  result_block_type_data_id = this->get_current_result_block_type(block_type);
  result_block_set_data_id = this->get_current_result_block_set(block_set);
  
  result_blocks.push_back({current_result_block,current_total_time_id,result_block_type_data_id,result_block_set_data_id,result_block_data_id,this->current_step,this->current_increment});
  dat_arrays.push_back({});
 
  return true;
}


bool CoreResultsDat::header_emas(std::vector<std::string> line)
{
  CalculiXApplicationInterface ccx_application_iface;

  double total_time;
  std::string block_set;
  std::string block_type;
  int result_block_data_id;
  int result_block_type_data_id;
  int result_block_set_data_id;

  int search = -1;
  for (size_t i = 0; i < line.size(); i++)
  {
    if (line[i]=="set")
    {
      search = int(i);
    }
  } 
  if (search < 2 || static_cast<std::size_t>(search + 1) >= line.size())
  {
    PRINT_ERROR("Invalid DAT result header.\n");
    return false;
  }
  block_set = line[search + 1];
  total_time = ccx_application_iface.string_scientific_to_double(line[line.size()-1],5);
  total_times.push_back(total_time);
  current_total_time_id = int(total_times.size())-1;

  block_type = "emas";
  std::vector<std::string> tmp_result_block_components;

  tmp_result_block_components.push_back("element");
  tmp_result_block_components.push_back("mass");
  tmp_result_block_components.push_back("xx");
  tmp_result_block_components.push_back("yy");
  tmp_result_block_components.push_back("zz");
  tmp_result_block_components.push_back("xy");
  tmp_result_block_components.push_back("xz");
  tmp_result_block_components.push_back("yz");

  // write new set of component names
  result_block_components.push_back(tmp_result_block_components);

  std::vector<std::vector<double>> tmp_result_block_data;
  std::vector<std::vector<int>> tmp_result_block_c1_data;
  result_block_data.push_back(tmp_result_block_data);
  result_block_c1_data.push_back(tmp_result_block_c1_data);
  
  if (result_blocks.size()==0)
  {
    current_result_block = 1;
  }else{
    current_result_block = int(result_blocks.size()+1);
  }
  result_block_data_id = int(result_block_data.size())-1;
  result_block_type_data_id = this->get_current_result_block_type(block_type);
  result_block_set_data_id = this->get_current_result_block_set(block_set);

  result_blocks.push_back({current_result_block,current_total_time_id,result_block_type_data_id,result_block_set_data_id,result_block_data_id,this->current_step,this->current_increment});
  dat_arrays.push_back({});

  return true;
}

bool CoreResultsDat::header_section_1(std::vector<std::string> line)
{
  CalculiXApplicationInterface ccx_application_iface;

  //std::string log = "header section 1 " + line[4] + " time " + line[7] + " \n";
  //PRINT_INFO("%s", log.c_str());
  if (line.size() > 7)
  {
    std::vector<int> tmp_result_section(2);
    tmp_result_section[0] = this->get_current_result_section_set(line[4]);
    tmp_result_section[1] = int(result_section_data.size());
    result_section.push_back(tmp_result_section);

    std::vector<double> tmp_result_section_data(22);
    tmp_result_section_data[0] = ccx_application_iface.string_scientific_to_double(line[7]);
    tmp_result_section_data[1] = double(current_increment);

    result_section_data.push_back(tmp_result_section_data);
  }

  return true;
}

bool CoreResultsDat::header_section_2(std::vector<std::string> line)
{
  //std::string log = "header section 2 \n";
  //PRINT_INFO("%s", log.c_str());
  
  return true;
}

bool CoreResultsDat::header_section_3(std::vector<std::string> line)
{
  //std::string log = "header section 3 \n";
  //PRINT_INFO("%s", log.c_str());
  
  return true;
}

bool CoreResultsDat::header_section_4(std::vector<std::string> line)
{
  //std::string log = "header section 4 \n";
  //PRINT_INFO("%s", log.c_str());
  
  return true;
}

bool CoreResultsDat::header_section_5(std::vector<std::string> line)
{
  //std::string log = "header section 5 \n";
  //PRINT_INFO("%s", log.c_str());
  
  return true;
}

bool CoreResultsDat::read_line(std::vector<std::string> line)
{
  CalculiXApplicationInterface ccx_application_iface;

  bool bool_node;
  int c1_id;
  int c1_type;
  int n_comp = int(result_block_components[result_block_components.size()-1].size());
  
  if (n_comp == line.size()) // elemental results
  {
    bool_node = false;
    if (n_comp == 2)
    {
      c1_type = 3; // element data with no integration points
    }else if (result_block_type[result_blocks[result_blocks.size()-1][2]]=="emas")
    {
      c1_type = 3; // element data with no integration points
    }else{
      c1_type = 2;
    }
  } else {                   // nodal results
    bool_node = true;
    c1_type = 1;
    
    //check if last component really is a number, shells ... so random
    if (line[line.size()-1][0]=='_')
    {
      bool_node = false;
      c1_type = 2;
    }
  }
  
  std::vector<double> result_comp(n_comp);
  int result_block_c1_data_id = -1;

  c1_id = std::stoi(line[0]);

  for (size_t i = 0; i < n_comp; i++)
  {
    if (bool_node)
    {
      result_comp[i] = ccx_application_iface.string_scientific_to_double(line[i+1]);
    }else{
      result_comp[i] = ccx_application_iface.string_scientific_to_double(line[i]);
    }
  }
      
  result_block_data[result_block_data.size()-1].push_back(result_comp);
  result_block_c1_data_id = int(result_block_data[result_block_data.size()-1].size()-1);
  result_block_c1_data[result_block_c1_data.size()-1].push_back({c1_id,result_block_c1_data_id,c1_type});

  return true;
}

bool CoreResultsDat::read_line_buckle(std::vector<std::string> line)
{
  CalculiXApplicationInterface ccx_application_iface;
 
  std::vector<double> result_comp(2);
  result_comp[0] = ccx_application_iface.string_scientific_to_double(line[0]); // mode no
  result_comp[1] = ccx_application_iface.string_scientific_to_double(line[1]); // buckling factor
  
  this->buckle_data[buckle_data.size()-1].push_back(result_comp);

  return true;
}

bool CoreResultsDat::read_line_section_1(std::vector<std::string> line)
{
  CalculiXApplicationInterface ccx_application_iface;

  //std::string log = "read line section 1 \n";
  //PRINT_INFO("%s", log.c_str());
  if (line.size() == 6){
    int data_id = int(result_section_data.size())-1;
    result_section_data[data_id][2] = ccx_application_iface.string_scientific_to_double(line[0]);
    result_section_data[data_id][3] = ccx_application_iface.string_scientific_to_double(line[1]);
    result_section_data[data_id][4] = ccx_application_iface.string_scientific_to_double(line[2]);
    result_section_data[data_id][5] = ccx_application_iface.string_scientific_to_double(line[3]);
    result_section_data[data_id][6] = ccx_application_iface.string_scientific_to_double(line[4]);
    result_section_data[data_id][7] = ccx_application_iface.string_scientific_to_double(line[5]);
  }

  return true;
}

bool CoreResultsDat::read_line_section_2(std::vector<std::string> line)
{
  CalculiXApplicationInterface ccx_application_iface;

  //std::string log = "read line section 2 \n";
  //PRINT_INFO("%s", log.c_str());
  if (line.size() == 6){
    int data_id = int(result_section_data.size())-1;
    result_section_data[data_id][8] = ccx_application_iface.string_scientific_to_double(line[0]);
    result_section_data[data_id][9] = ccx_application_iface.string_scientific_to_double(line[1]);
    result_section_data[data_id][10] = ccx_application_iface.string_scientific_to_double(line[2]);
    result_section_data[data_id][11] = ccx_application_iface.string_scientific_to_double(line[3]);
    result_section_data[data_id][12] = ccx_application_iface.string_scientific_to_double(line[4]);
    result_section_data[data_id][13] = ccx_application_iface.string_scientific_to_double(line[5]);
  }

  return true;
}

bool CoreResultsDat::read_line_section_3(std::vector<std::string> line)
{
  CalculiXApplicationInterface ccx_application_iface;

  //std::string log = "read line section 3 \n";
  //PRINT_INFO("%s", log.c_str());
  if (line.size() == 3){
    int data_id = int(result_section_data.size())-1;
    result_section_data[data_id][14] = ccx_application_iface.string_scientific_to_double(line[0]);
    result_section_data[data_id][15] = ccx_application_iface.string_scientific_to_double(line[1]);
    result_section_data[data_id][16] = ccx_application_iface.string_scientific_to_double(line[2]);
  }

  return true;
}

bool CoreResultsDat::read_line_section_4(std::vector<std::string> line)
{
  CalculiXApplicationInterface ccx_application_iface;

  //std::string log = "read line section 4 \n";
  //PRINT_INFO("%s", log.c_str());
  if (line.size() == 5){
    int data_id = int(result_section_data.size())-1;
    result_section_data[data_id][17] = ccx_application_iface.string_scientific_to_double(line[0]);
    result_section_data[data_id][18] = ccx_application_iface.string_scientific_to_double(line[1]);
    result_section_data[data_id][19] = ccx_application_iface.string_scientific_to_double(line[2]);
    result_section_data[data_id][20] = ccx_application_iface.string_scientific_to_double(line[3]);
    result_section_data[data_id][21] = ccx_application_iface.string_scientific_to_double(line[4]);
  }

  return true;
}

bool CoreResultsDat::read_lines_thread(int result_block_data_id, int thread_id)
{
  CalculiXApplicationInterface ccx_application_iface;

  for (size_t ib = 0; ib < this->dat_arrays[result_block_data_id].size(); ib++)
  {
    std::vector<std::string> line = dat_arrays[result_block_data_id][ib];

    bool bool_node;
    int c1_id;
    int c1_type;
    int n_comp = int(result_block_components[result_block_data_id].size());
    
    if (n_comp == line.size()) // elemental results
    {
      bool_node = false;
      if (n_comp == 2)
      {
        c1_type = 3; // element data with no integration points
      }else if (result_block_type[result_blocks[result_block_data_id][2]]=="emas")
      {
        c1_type = 3; // element data with no integration points
      }else{
        c1_type = 2;
      }
    } else {                   // nodal results
      bool_node = true;
      c1_type = 1;
      
      //check if last component really is a number, shells ... so random
      if (line[line.size()-1][0]=='_')
      {
        bool_node = false;
        c1_type = 2;
      }
    }
    
    std::vector<double> result_comp(n_comp);
    int result_block_c1_data_id = -1;

    c1_id = std::stoi(line[0]);

    for (size_t i = 0; i < n_comp; i++)
    {
      if (bool_node)
      {
        result_comp[i] = ccx_application_iface.string_scientific_to_double(line[i+1]);
      }else{
        result_comp[i] = ccx_application_iface.string_scientific_to_double(line[i]);
      }
    }
        
    result_block_data[result_block_data_id].push_back(result_comp);
    result_block_c1_data_id = int(result_block_data[result_block_data_id].size()-1);
    result_block_c1_data[result_block_data_id].push_back({c1_id,result_block_c1_data_id,c1_type});
    
    this->progress[thread_id] = this->progress[thread_id] + 1;
  }
  
  
  return true;
}

bool CoreResultsDat::compute_predefined(int result_block_data_id, int thread_id)
{
  CalculiXResultsInterface ccx_results_iface;

  int i = result_block_data_id;

  //stresses
  if (this->erase_whitespace(result_block_type[result_blocks[i][2]])=="stresses") 
  {
    // add components
    result_block_components[result_blocks[i][4]].push_back("mises");

    // compute values
    for (size_t ii = 0; ii < result_block_data[result_blocks[i][4]].size(); ii++)
    {
      result_block_data[result_blocks[i][4]][ii].push_back(ccx_results_iface.compute_von_mises_stress({
      result_block_data[result_blocks[i][4]][ii][2],
      result_block_data[result_blocks[i][4]][ii][3],
      result_block_data[result_blocks[i][4]][ii][4],
      result_block_data[result_blocks[i][4]][ii][5],
      result_block_data[result_blocks[i][4]][ii][6],
      result_block_data[result_blocks[i][4]][ii][7]}));
    }

    // add components
    result_block_components[result_blocks[i][4]].push_back("PS1");
    result_block_components[result_blocks[i][4]].push_back("PS2");
    result_block_components[result_blocks[i][4]].push_back("PS3");
    result_block_components[result_blocks[i][4]].push_back("worstPS");
    result_block_components[result_blocks[i][4]].push_back("maxShear");

    // compute values
    for (size_t ii = 0; ii < result_block_data[result_blocks[i][4]].size(); ii++)
    {
      std::vector<double> ps = ccx_results_iface.compute_principal_stresses({
      result_block_data[result_blocks[i][4]][ii][2],
      result_block_data[result_blocks[i][4]][ii][3],
      result_block_data[result_blocks[i][4]][ii][4],
      result_block_data[result_blocks[i][4]][ii][5],
      result_block_data[result_blocks[i][4]][ii][6],
      result_block_data[result_blocks[i][4]][ii][7]});

      result_block_data[result_blocks[i][4]][ii].push_back(ps[0]);
      result_block_data[result_blocks[i][4]][ii].push_back(ps[1]);
      result_block_data[result_blocks[i][4]][ii].push_back(ps[2]);
      result_block_data[result_blocks[i][4]][ii].push_back(ps[3]);
      result_block_data[result_blocks[i][4]][ii].push_back(0.5 * std::max({ps[0]-ps[2],ps[0]-ps[1],ps[1]-ps[2]}));
    }
    this->progress[thread_id] = this->progress[thread_id] + 1;
  }

  //strains
  if (this->erase_whitespace(result_block_type[result_blocks[i][2]])=="strains") 
  {
    // add components
    result_block_components[result_blocks[i][4]].push_back("mises");

    // compute values      
    for (size_t ii = 0; ii < result_block_data[result_blocks[i][4]].size(); ii++)
    {
      result_block_data[result_blocks[i][4]][ii].push_back(ccx_results_iface.compute_von_mises_strain({
      result_block_data[result_blocks[i][4]][ii][2],
      result_block_data[result_blocks[i][4]][ii][3],
      result_block_data[result_blocks[i][4]][ii][4],
      result_block_data[result_blocks[i][4]][ii][5],
      result_block_data[result_blocks[i][4]][ii][6],
      result_block_data[result_blocks[i][4]][ii][7]}));
    }

    // add components
    result_block_components[result_blocks[i][4]].push_back("PE1");
    result_block_components[result_blocks[i][4]].push_back("PE2");
    result_block_components[result_blocks[i][4]].push_back("PE3");
    result_block_components[result_blocks[i][4]].push_back("worstPE");
    result_block_components[result_blocks[i][4]].push_back("maxShear");

    // compute values
    for (size_t ii = 0; ii < result_block_data[result_blocks[i][4]].size(); ii++)
    {
      std::vector<double> pe = ccx_results_iface.compute_principal_strains({
      result_block_data[result_blocks[i][4]][ii][2],
      result_block_data[result_blocks[i][4]][ii][3],
      result_block_data[result_blocks[i][4]][ii][4],
      result_block_data[result_blocks[i][4]][ii][5],
      result_block_data[result_blocks[i][4]][ii][6],
      result_block_data[result_blocks[i][4]][ii][7]});

      result_block_data[result_blocks[i][4]][ii].push_back(pe[0]);
      result_block_data[result_blocks[i][4]][ii].push_back(pe[1]);
      result_block_data[result_blocks[i][4]][ii].push_back(pe[2]);
      result_block_data[result_blocks[i][4]][ii].push_back(pe[3]);
      result_block_data[result_blocks[i][4]][ii].push_back(0.5 * std::max({pe[0]-pe[2],pe[0]-pe[1],pe[1]-pe[2]}));
    }
    this->progress[thread_id] = this->progress[thread_id] + 1;
  }

  return true;
}

int CoreResultsDat::get_current_result_block_type(std::string result_type)
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

int CoreResultsDat::get_current_result_block_set(std::string result_set)
{
  int data_id = -1;
  for (size_t i = 0; i < result_block_set.size(); i++)
  {
    if (result_block_set[i]==result_set)
    {
      return int(i);
    }
  }
  result_block_set.push_back(result_set);
  data_id = int(result_block_set.size())-1;
  
  return data_id;
}

int CoreResultsDat::get_current_result_section_set(std::string result_set)
{
  int data_id = -1;
  for (size_t i = 0; i < result_section_set.size(); i++)
  {
    if (result_section_set[i]==result_set)
    {
      return int(i);
    }
  }
  result_section_set.push_back(result_set);
  data_id = int(result_section_set.size())-1;
  
  return data_id;
}

bool CoreResultsDat::check_element_sets()
{
  //change set name when there is integration point data
  
  for (size_t i = 0; i < result_blocks.size(); i++)
  {
    if (!result_block_c1_data[result_blocks[i][4]].empty() && result_block_c1_data[result_blocks[i][4]][0][2] == 2)
    {
      result_blocks[i][3] = this->get_current_result_block_set("ip_" + result_block_set[result_blocks[i][3]]);
    }
  }
  
  //delete set name, if they are not used anymore
  bool unused;
  for (size_t i = 0; i < result_block_set.size(); i++)
  {
    unused = true;
    for (size_t ii = 0; ii < result_blocks.size(); ii++)
    {
      if (result_blocks[ii][3] == i)
      {
        unused = false;
      }
    }
    if (unused)
    {
      std::vector<std::string> set = result_block_set;

      result_block_set.erase(result_block_set.begin() + i);
      --i;

      //rewrite the set id in the blocks!!!
      for (size_t ii = 0; ii < result_blocks.size(); ii++)
      {
        result_blocks[ii][3] = this->get_current_result_block_set(set[result_blocks[ii][3]]);
      }
    }
  }
  return true;
}

std::vector<std::string> CoreResultsDat::get_result_block_components_from_result_block_type(std::string result_block_type)
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

int CoreResultsDat::get_result_block_component_id(int result_block_type_id,std::string result_block_component)
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

int CoreResultsDat::get_result_block_type_data_id(std::string result_block_type)
{
  int result_block_type_data_id = -1;

  for (size_t i = 0; i < result_block_type.size(); i++)
  {
    if (this->result_block_type[i]==result_block_type)
    {
      result_block_type_data_id = int(i);
      return result_block_type_data_id;
    }
  }

  return result_block_type_data_id;
}

int CoreResultsDat::get_result_block_set_data_id(std::string result_block_set)
{
  int result_block_set_data_id = -1;

  for (size_t i = 0; i < result_block_set.size(); i++)
  {
    if (this->result_block_set[i]==result_block_set)
    {
      result_block_set_data_id = int(i);
      return result_block_set_data_id;
    }
  }

  return result_block_set_data_id;
}

bool CoreResultsDat::sort_data(int data_id)
{
  std::vector<int> tmp_c1;
  std::vector<int> tmp_result_block_c1_data_id;
  std::vector<int> tmp_result_block_c1_data_type;

  for (size_t i = 0; i < result_block_c1_data[data_id].size(); i++)
  {
    tmp_c1.push_back(result_block_c1_data[data_id][i][0]);
    tmp_result_block_c1_data_id.push_back(result_block_c1_data[data_id][i][1]);
    tmp_result_block_c1_data_type.push_back(result_block_c1_data[data_id][i][2]);
  }  
  auto p = sort_permutation(tmp_c1);
  this->apply_permutation(tmp_c1, p);
  this->apply_permutation(tmp_result_block_c1_data_id, p);
  this->apply_permutation(tmp_result_block_c1_data_type, p);
  sorted_c1[data_id] = tmp_c1;
  sorted_result_block_c1_data_id[data_id] = tmp_result_block_c1_data_id;
  sorted_result_block_c1_data_type[data_id] = tmp_result_block_c1_data_type;

  return true;
}

int CoreResultsDat::get_result_block_lines(const std::vector<std::vector<std::vector<std::string>>>& result_block_lines)
{
  int lines = 0;
  for (size_t i = 0; i < result_block_lines.size(); i++)
  {
    lines = lines + int(result_block_lines[i].size());
  }
  return lines;
}
int CoreResultsDat::get_result_blocks_predefined()
{
  int blocks = 0;
  for (size_t i = 0; i < result_blocks.size(); i++)
  {
    //stresses
    if ((this->erase_whitespace(result_block_type[result_blocks[i][2]])=="stresses")||(this->erase_whitespace(result_block_type[result_blocks[i][2]])=="strains")) 
    {
      ++blocks;
    }
  }
  return blocks;
}

void CoreResultsDat::update_progressbar()
{
  //std::string log;
  const auto t_end = std::chrono::high_resolution_clock::now();
  int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
  if (duration > 500)
  {
    int part = 0;
    for (size_t i = 0; i < progress.size()-1; i++)
    {
      part += progress[i];
      
      //log = "thread " + std::to_string(i) + " lines " + std::to_string(progress[i]) + " \n";
      //PRINT_INFO("%s", log.c_str());
    }

    //log = "max lines " + std::to_string(progress[progress.size()-1]) + " \n";
    //PRINT_INFO("%s", log.c_str());

    progressbar->percent(double(part)/double(progress[progress.size()-1]));
    progressbar->check_interrupt();
    t_start = std::chrono::high_resolution_clock::now();
  }
}

bool CoreResultsDat::print_data()
{
  CalculiXApplicationInterface ccx_application_iface;

  std::string log;
  log = "DAT DATA for Job ID " + std::to_string(job_id) + " \n";

  if (result_blocks.size()>0)
  {
    for (size_t i = 0; i < result_blocks.size(); i++)
    {
      log.append("result block " + std::to_string(result_blocks[i][0]) + " ## ");
      log.append("total time data id " + std::to_string(result_blocks[i][1]) + " ## ");
      log.append("result block type " + std::to_string(result_blocks[i][2]) + " ## ");
      log.append("result set type " + std::to_string(result_blocks[i][3]) + " ## ");
      log.append("result block data id " + std::to_string(result_blocks[i][4]) + " \n");
      
      log.append("total time " + std::to_string(total_times[result_blocks[i][1]]) + " \n");
      log.append("result block type " + result_block_type[result_blocks[i][2]] + " \n");
      log.append("result block set " + result_block_set[result_blocks[i][3]] + " \n");
      log.append("component names ");
      for (size_t ii = 0; ii < result_block_components[i].size(); ii++)
      {
        log.append(result_block_components[i][ii] + " ");
      }
      log.append("\n");

      for (size_t ii = result_block_c1_data[result_blocks[i][4]].size()-1; ii < result_block_c1_data[result_blocks[i][4]].size(); ii++)
      {
        log.append(std::to_string(result_block_c1_data[result_blocks[i][4]][ii][0]) + " ");
        log.append(std::to_string(result_block_c1_data[result_blocks[i][4]][ii][1]) + " ");
        log.append(std::to_string(result_block_c1_data[result_blocks[i][4]][ii][2]) + " ## ");
        for (size_t iii = 0; iii < result_block_components[i].size(); iii++)
        {
          log.append(ccx_application_iface.to_string_scientific(result_block_data[result_blocks[i][4]][ii][iii]) + " ");
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
std::vector<std::size_t> CoreResultsDat::sort_permutation(
    const std::vector<T>& vec)
{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
        [&](std::size_t i, std::size_t j){ return vec[i] < vec[j]; });

    return p;
}

template <typename T> 
void CoreResultsDat::apply_permutation(
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
