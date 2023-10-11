#include "CoreResultsFrd.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"

#include <chrono>
#include <cmath>
#include <fstream>
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

  return true;
}

bool CoreResultsFrd::check_initialized()
{
  return is_initialized;
}


bool CoreResultsFrd::read()
{
  int maxlines = 0;
  int currentline = 0;
  ProgressTool progressbar;
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
    progressbar.start(0,100,"Scanning Results FRD");
    while (std::getline(frd,frdline))
    { 
      ++maxlines;
      if(frd.eof()){
        break;
      }
    }
    progressbar.end();
    frd.close();
    frd.open(this->filepath);

    progressbar.start(0,100,"Reading Results FRD");
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
        progressbar.percent(double(currentline)/double(maxlines));
        progressbar.check_interrupt();
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
  progressbar.end();
  //PRINT_INFO("%s", log.c_str());
  //print_data();

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
    int nodes_coords_data_id = nodes_coords.size()-1;

    nodes.push_back({node_id,nodes_coords_data_id});
  }

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

      elements_connectivity_data_id = elements_connectivity.size() - 1;
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
    current_total_time_id = total_times.size()-1;

    total_increment = ccx_iface->string_scientific_to_double(line[5]);
    current_total_increment = total_increment;

  } else if (line[0] == "-4") // getting result type
  { 
    std::vector<std::vector<double>> tmp_result_block_data;
    std::vector<std::vector<int>> tmp_result_block_node_data;
    result_block_data.push_back(tmp_result_block_data);
    result_block_node_data.push_back(tmp_result_block_node_data);
      
    result_block_data_id = result_block_data.size()-1;
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
      return i;
    }
  }
  result_block_type.push_back(result_type);
  int data_id = result_block_type.size()-1;
  
  return data_id;
}

bool CoreResultsFrd::read_nodal_result_block(std::vector<std::string> line)
{
  
  if (line[0] == "-1")
  {
    int node_id;
    int n_comp = result_block_components[result_block_components.size()-1].size();
    std::vector<double> result_comp(n_comp);
    int result_block_node_data_id = -1;

    node_id = std::stoi(line[1]);

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
    }else{
      for (size_t i = 0; i < n_comp; i++)
      {
        result_comp[i] = ccx_iface->string_scientific_to_double(line[i+2]);
      }
    }
        
    result_block_data[result_block_data.size()-1].push_back(result_comp);
    result_block_node_data_id = result_block_data[result_block_data.size()-1].size()-1;
    result_block_node_data[result_block_node_data.size()-1].push_back({node_id,result_block_node_data_id});
  }else if (line[0] == "-5")
  {
    result_block_components[result_block_components.size()-1].push_back(line[1]);
  }

  return true;
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