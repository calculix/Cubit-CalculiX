#include "CoreResultsFrd.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

#include <cmath>
#include <fstream>
#include <iostream>
#include "loadUserOptions.hpp"

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
  steps.clear();
  steps_time.clear();
  result_disp.clear();
  result_disp_nodes.clear();
  return true;
}

bool CoreResultsFrd::check_initialized()
{
  return is_initialized;
}


bool CoreResultsFrd::read()
{
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
    log = "";
    while (frd)
    {
      std::getline(frd,frdline);

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
      } else if (current_read_mode == 101) // DISP
      {
        this->read_disp(frd_array);
      } else if (current_read_mode == 9999)
      {
        break;
      }

      for (size_t i = 0; i < frd_array.size(); i++)
      {
        //log.append(std::to_string(current_read_mode) + "#");
        log.append(frd_array[i] + " ");
      }
      log.append("\n");
    }
  }
  frd.close();
  PRINT_INFO("%s", log.c_str());
  print_data();

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
    if ((frdline[beginSpace-1] != 'E') && (beginSpace !=0))
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
  }else if ((current_read_mode == 3)||(current_read_mode > 9)) // switch to parameter mode
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
    size_t name_pos = line[1].find_first_not_of("0123456789");
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

    node_coords[0] = std::stod(line[2]);
    node_coords[1] = std::stod(line[3]);
    node_coords[2] = std::stod(line[4]);

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
  int step_id;
  double step_time;
  int result_type_data_id = -1;
  
  if (line[0] == "1PSTEP")
  {
    step_id = std::stoi(line[2]);
    if (!check_current_step_id_exists(step_id))
    {
      current_step_id = step_id;
    }
  } else if (line[0] == "100CL")
  {
    step_id = std::stoi(line[6]);
    step_time = std::stod(line[2]);
    if (!check_current_step_id_exists(step_id))
    {
      steps_time.push_back(step_time);
      current_step_time_id = steps_time.size()-1;
    }
  } else if (line[0] == "-4") // choosing result type
  {
    if (line[1] == "DISP")
    {
      std::vector<std::vector<double>> tmp_disp;
      std::vector<std::vector<int>> tmp_disp_nodes;
      result_disp.push_back(tmp_disp);
      result_disp_nodes.push_back(tmp_disp_nodes);
      
      result_type_data_id = result_disp.size()-1;
      steps.push_back({current_step_id,current_step_time_id,1,result_type_data_id});
      current_read_mode = 101;
    }
  }
  return true;
}

bool CoreResultsFrd::check_current_step_id_exists(int step_id)
{
  for (size_t i = 0; i < steps.size(); i++)
  {
    if (steps[i][0]==step_id)
    {
      return true;
    }
  }
  return false;
}

bool CoreResultsFrd::read_disp(std::vector<std::string> line)
{
  int node_id;
  std::vector<double> disp_comp(4);
  int result_disp_node_data_id = -1;
  
  if (line[0] == "-1")
  {
    node_id = std::stoi(line[1]);

    disp_comp[0] = std::stod(line[2]);
    disp_comp[1] = std::stod(line[3]);
    disp_comp[2] = std::stod(line[4]);
    // compute ALL
    disp_comp[3] = std::sqrt(disp_comp[0]*disp_comp[0]+disp_comp[1]*disp_comp[1]+disp_comp[2]*disp_comp[2]);

    result_disp[result_disp.size()-1].push_back(disp_comp);

    result_disp_node_data_id = result_disp[result_disp.size()-1].size()-1;

    result_disp_nodes[result_disp_nodes.size()-1].push_back({node_id,result_disp_node_data_id});
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
    for (size_t i = 0; i < nodes.size(); i++)
    {
      log.append(std::to_string(nodes[i][0]) + " ## " + std::to_string(nodes[i][1]) + " ## x ");
      log.append(std::to_string(nodes_coords[nodes[i][1]][0]) + " ## y " + std::to_string(nodes_coords[nodes[i][1]][1]) + " ## z " + std::to_string(nodes_coords[nodes[i][1]][2]));

      log.append("\n");
    }
  }

  if (elements.size()>0)
  {
    for (size_t i = 0; i < elements.size(); i++)
    {
      log.append(std::to_string(elements[i][0]) + " ## " + std::to_string(elements[i][1]) + " ## " + std::to_string(elements[i][2]) + " #---# ");
      
      for (size_t ii = 0; ii < elements_connectivity[elements[i][2]].size(); ii++)
      {
        log.append(std::to_string(elements_connectivity[elements[i][2]][ii]) + " ");
      }

      log.append("\n");
    }
  }
  if (steps.size()>0)
  {
    for (size_t i = 0; i < steps.size(); i++)
    {
      log.append("step " + std::to_string(steps[i][0]) + " ## " + std::to_string(steps[i][1]) + " ## " + std::to_string(steps[i][2]) + " ## " + std::to_string(steps[i][3]) + " \n");
      log.append("steptime " + std::to_string(steps_time[steps[i][1]]) + " \n");
      log.append("\n");
      if (steps[i][2] == 1)
      {
        for (size_t ii = 0; ii < result_disp_nodes[steps[i][3]].size(); ii++)
        {
          log.append(std::to_string(result_disp_nodes[steps[i][3]][ii][0]) + " ");
          log.append(ccx_iface->to_string_scientific(result_disp[steps[i][3]][ii][0]) + " ");
          log.append(ccx_iface->to_string_scientific(result_disp[steps[i][3]][ii][1]) + " ");
          log.append(ccx_iface->to_string_scientific(result_disp[steps[i][3]][ii][2]) + " ");
          log.append(ccx_iface->to_string_scientific(result_disp[steps[i][3]][ii][3]));
          log.append("\n");
        }
      }
    }
  }

  PRINT_INFO("%s", log.c_str());
  return true;
}