#include "Core/loads/CoreLoadsForces.hpp"
#include <string>
#include <vector>
#include "Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
CoreLoadsForces::CoreLoadsForces()
{}

CoreLoadsForces::~CoreLoadsForces()
{}

bool CoreLoadsForces::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    is_initialized = true;  
    return true;
  }
}

bool CoreLoadsForces::update()
{
  return true;
}

bool CoreLoadsForces::reset()
{
  loads_data.clear();
  time_delay_data.clear();
  force_data.clear();
  name_data.clear();
  init();
  return true;
}

bool CoreLoadsForces::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsForces::create_load(std::vector<std::string> options, std::vector<double> options2)
{
  // options: op mode, amplitude, time delay, nodeset, name
  // options2: force/moment components for dofs 1..6
  if (options.size() != 5 || options2.size() != 6)
  {
    return false;
  }

  int op_mode = std::stoi(options[0]);
  int amplitude_id = std::stoi(options[1]);
  int nodeset_id = std::stoi(options[3]);
  int load_id = 1;
  int time_delay_id = 1;
  int force_id = 1;
  int name_id = 1;

  if (!loads_data.empty())
  {
    load_id = loads_data.back()[0] + 1;
  }
  if (!time_delay_data.empty())
  {
    time_delay_id = std::stoi(time_delay_data.back()[0]) + 1;
  }
  if (!force_data.empty())
  {
    force_id = int(force_data.back()[0]) + 1;
  }
  if (!name_data.empty())
  {
    name_id = std::stoi(name_data.back()[0]) + 1;
  }

  add_time_delay(std::to_string(time_delay_id), options[2]);
  add_force(double(force_id), options2[0], options2[1], options2[2], options2[3], options2[4], options2[5]);
  add_name(std::to_string(name_id), options[4]);
  return add_load(load_id, op_mode, amplitude_id, time_delay_id, nodeset_id, force_id, name_id);
}

bool CoreLoadsForces::add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id, int nodeset_id, int force_id, int name_id)
{
  std::vector<int> v = {load_id, op_mode, amplitude_id, time_delay_id, nodeset_id, force_id, name_id};
  loads_data.push_back(v);
  return true;
}

bool CoreLoadsForces::modify_load(int load_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  // markers: op, amplitude, delay, nodeset, dofs 1..6, name
  if (options.size() != 5 || options2.size() != 6 || options_marker.size() != 11)
  {
    return false;
  }

  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return false;
  }

  int time_delay_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[loads_data_id][3]);
  int force_data_id = get_force_data_id_from_force_id(loads_data[loads_data_id][5]);
  int name_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][6]);
  if (time_delay_data_id == -1 || force_data_id == -1 || name_data_id == -1)
  {
    return false;
  }

  // Parse all requested integer changes before modifying stored data.
  int op_mode = loads_data[loads_data_id][1];
  int amplitude_id = loads_data[loads_data_id][2];
  int nodeset_id = loads_data[loads_data_id][4];
  if (options_marker[0] == 1)
  {
    op_mode = std::stoi(options[0]);
  }
  if (options_marker[1] == 1)
  {
    amplitude_id = std::stoi(options[1]);
  }
  if (options_marker[3] == 1)
  {
    nodeset_id = std::stoi(options[3]);
  }

  loads_data[loads_data_id][1] = op_mode;
  loads_data[loads_data_id][2] = amplitude_id;
  loads_data[loads_data_id][4] = nodeset_id;
  if (options_marker[2] == 1)
  {
    time_delay_data[time_delay_data_id][1] = options[2];
  }
  for (size_t i = 0; i < 6; i++)
  {
    if (options_marker[i + 4] == 1)
    {
      force_data[force_data_id][i + 1] = options2[i];
    }
  }
  if (options_marker[10] == 1)
  {
    name_data[name_data_id][1] = options[4];
  }
  return true;
}

bool CoreLoadsForces::delete_load(int load_id)
{
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return false;
  }

  int sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[loads_data_id][3]);
  if (sub_data_id != -1)
  {
    time_delay_data.erase(time_delay_data.begin() + sub_data_id);
  }
  sub_data_id = get_force_data_id_from_force_id(loads_data[loads_data_id][5]);
  if (sub_data_id != -1)
  {
    force_data.erase(force_data.begin() + sub_data_id);
  }
  sub_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][6]);
  if (sub_data_id != -1)
  {
    name_data.erase(name_data.begin() + sub_data_id);
  }

  loads_data.erase(loads_data.begin() + loads_data_id);
  return true;
}

bool CoreLoadsForces::add_force(double force_id, double dof_1_value, double dof_2_value, double dof_3_value, double dof_4_value, double dof_5_value, double dof_6_value)
{
  std::vector<double> v = {force_id, dof_1_value, dof_2_value, dof_3_value, dof_4_value, dof_5_value, dof_6_value};
  force_data.push_back(v);
  return true;
}

bool CoreLoadsForces::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  name_data.push_back(v);
  return true;
}

bool CoreLoadsForces::add_time_delay(std::string time_delay_id, std::string time_delay_value)
{
  std::vector<std::string> v = {time_delay_id, time_delay_value};
      
  time_delay_data.push_back(v);

  return true;
}

int CoreLoadsForces::get_loads_data_id_from_load_id(int load_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < loads_data.size(); i++)
  {
    if (loads_data[i][0]==load_id)
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsForces::get_time_delay_data_id_from_time_delay_id(int time_delay_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    if (time_delay_data[i][0]==std::to_string(time_delay_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsForces::get_force_data_id_from_force_id(int force_id)
{
  int return_int = -1;
  for (size_t i = 0; i < force_data.size(); i++)
  {
    if (force_data[i][0] == double(force_id))
    {
      return_int = int(i);
    }
  }
  return return_int;
}

int CoreLoadsForces::get_name_data_id_from_name_id(int name_id)
{
  int return_int = -1;
  for (size_t i = 0; i < name_data.size(); i++)
  {
    if (name_data[i][0] == std::to_string(name_id))
    {
      return_int = int(i);
    }
  }
  return return_int;
}

std::string CoreLoadsForces::get_load_parameter_export(int load_id)
{
  CalculiXAmplitudesInterface ccx_amplitudes_iface;

  int load_data_id;
  int sub_data_id;
  std::string str_temp = "";
  load_data_id = get_loads_data_id_from_load_id(load_id);
  if (load_data_id == -1)
  {
    return "";
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[load_data_id][3]);
  if (sub_data_id == -1)
  {
    return "";
  }
  if (loads_data[load_data_id][1]==0)
  {
    //str_temp.append(",OP=MOD");
  }else if (loads_data[load_data_id][1]==1)
  {
    str_temp.append(",OP=NEW");
  }
  if (loads_data[load_data_id][2]!=-1)
  {
    str_temp.append(",AMPLITUDE=" + ccx_amplitudes_iface.get_amplitude_name(loads_data[load_data_id][2]));
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[load_data_id][3]);
  if (loads_data[load_data_id][2] != -1 &&
      time_delay_data[sub_data_id][1] != "")
  {
    str_temp.append(",TIME DELAY=" + time_delay_data[sub_data_id][1]);
  }
  return str_temp;
}

std::string CoreLoadsForces::get_load_export(int load_id)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return "";
  }

  int force_data_id = get_force_data_id_from_force_id(loads_data[loads_data_id][5]);
  int time_delay_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[loads_data_id][3]);
  if (force_data_id == -1 || time_delay_data_id == -1)
  {
    return "";
  }

  int nodeset_id = loads_data[loads_data_id][4];
  if (!ccx_mesh_iface.check_nodeset_exists(nodeset_id))
  {
    return "";
  }

  std::string nodeset_name = ccx_mesh_iface.get_nodeset_name(nodeset_id);
  if (nodeset_name.empty())
  {
    return "";
  }

  std::string str_temp = "*CLOAD";
  str_temp.append(get_load_parameter_export(load_id));
  str_temp.append("\n");
  for (size_t i = 1; i <= 6; i++)
  {
    if (force_data[force_data_id][i] != 0.0)
    {
      str_temp.append(nodeset_name + "," + std::to_string(i) + "," + ccx_application_iface.to_string_scientific(force_data[force_data_id][i]) + "\n");
    }
  }
  return str_temp;
}

std::string CoreLoadsForces::print_data()
{
  std::string str_return = "\n CoreLoadsForces loads_data: \n";
  str_return.append("load_id, OP MODE, amplitude_id, time_delay_id, nodeset_id, force_id, name_id\n");
  for (size_t i = 0; i < loads_data.size(); i++)
  {
    for (size_t ii = 0; ii < loads_data[i].size(); ii++)
    {
      str_return.append(std::to_string(loads_data[i][ii]) + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreLoadsForces time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value\n");
  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(time_delay_data[i][0] + " " + time_delay_data[i][1] + "\n");
  }

  str_return.append("\n CoreLoadsForces force_data: \n");
  str_return.append("force_id, dof_1, dof_2, dof_3, dof_4, dof_5, dof_6\n");
  for (size_t i = 0; i < force_data.size(); i++)
  {
    for (size_t ii = 0; ii < force_data[i].size(); ii++)
    {
      str_return.append(std::to_string(force_data[i][ii]) + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreLoadsForces name_data: \n");
  str_return.append("name_id, name\n");
  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + "\n");
  }
  return str_return;
}
