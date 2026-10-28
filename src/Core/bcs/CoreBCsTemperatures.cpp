#include "Core/bcs/CoreBCsTemperatures.hpp"
#include <string>
#include <vector>
#include <cmath>
#include "Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
CoreBCsTemperatures::CoreBCsTemperatures()
{}

CoreBCsTemperatures::~CoreBCsTemperatures()
{}

bool CoreBCsTemperatures::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    is_initialized = true;  
    return true;
  }
}

bool CoreBCsTemperatures::update()
{
  return true;
}

bool CoreBCsTemperatures::reset()
{
  bcs_data.clear();
  time_delay_data.clear();
  magnitude_data.clear();
  name_data.clear();
  init();
  return true;
}

bool CoreBCsTemperatures::check_initialized()
{
  return is_initialized;
}

bool CoreBCsTemperatures::create_bc(std::vector<std::string> options, std::vector<double> options2)
{
  // options: op mode, amplitude, time delay, nodeset, name, keyword type
  // options2: magnitude
  if (options.size() != 6 || options2.size() != 1)
  {
    return false;
  }

  int keyword_type = std::stoi(options[5]);
  if (keyword_type < 0 || keyword_type > 1 || !std::isfinite(options2[0]))
  {
    return false;
  }

  int op_mode = std::stoi(options[0]);
  int amplitude_id = std::stoi(options[1]);
  int nodeset_id = std::stoi(options[3]);
  int bc_id = 1;
  int time_delay_id = 1;
  int magnitude_id = 1;
  int name_id = 1;

  if (!bcs_data.empty())
  {
    bc_id = bcs_data.back()[0] + 1;
  }
  if (!time_delay_data.empty())
  {
    time_delay_id = std::stoi(time_delay_data.back()[0]) + 1;
  }
  if (!magnitude_data.empty())
  {
    magnitude_id = int(magnitude_data.back()[0]) + 1;
  }
  if (!name_data.empty())
  {
    name_id = std::stoi(name_data.back()[0]) + 1;
  }

  add_time_delay(std::to_string(time_delay_id), options[2]);
  add_magnitude(double(magnitude_id), options2[0]);
  add_name(std::to_string(name_id), options[4]);
  return add_bc(bc_id, op_mode, amplitude_id, time_delay_id, nodeset_id, magnitude_id, name_id, keyword_type);
}

bool CoreBCsTemperatures::add_bc(int bc_id, int op_mode, int amplitude_id, int time_delay_id, int nodeset_id, int magnitude_id, int name_id, int keyword_type)
{
  std::vector<int> v = {bc_id, op_mode, amplitude_id, time_delay_id, nodeset_id, magnitude_id, name_id, keyword_type};
  bcs_data.push_back(v);
  return true;
}

bool CoreBCsTemperatures::modify_bc(int bc_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  // markers: op, amplitude, delay, nodeset, magnitude, name, keyword type
  if (options.size() != 6 || options2.size() != 1 || options_marker.size() != 7)
  {
    return false;
  }

  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1)
  {
    return false;
  }

  int time_delay_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bcs_data_id][3]);
  int magnitude_data_id = get_magnitude_data_id_from_magnitude_id(bcs_data[bcs_data_id][5]);
  int name_data_id = get_name_data_id_from_name_id(bcs_data[bcs_data_id][6]);
  if (time_delay_data_id == -1 || magnitude_data_id == -1 || name_data_id == -1)
  {
    return false;
  }

  // Parse all requested integer changes before modifying stored data.
  int op_mode = bcs_data[bcs_data_id][1];
  int amplitude_id = bcs_data[bcs_data_id][2];
  int nodeset_id = bcs_data[bcs_data_id][4];
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

  int keyword_type = bcs_data[bcs_data_id][7];
  if (options_marker[6] == 1)
  {
    keyword_type = std::stoi(options[5]);
  }
  if (keyword_type < 0 || keyword_type > 1)
  {
    return false;
  }
  if (options_marker[4] == 1 && !std::isfinite(options2[0]))
  {
    return false;
  }

  bcs_data[bcs_data_id][7] = keyword_type;
  bcs_data[bcs_data_id][1] = op_mode;
  bcs_data[bcs_data_id][2] = amplitude_id;
  bcs_data[bcs_data_id][4] = nodeset_id;
  if (options_marker[2] == 1)
  {
    time_delay_data[time_delay_data_id][1] = options[2];
  }
  if (options_marker[4] == 1)
  {
    magnitude_data[magnitude_data_id][1] = options2[0];
  }
  if (options_marker[5] == 1)
  {
    name_data[name_data_id][1] = options[4];
  }
  return true;
}

bool CoreBCsTemperatures::delete_bc(int bc_id)
{
  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1)
  {
    return false;
  }

  int sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bcs_data_id][3]);
  if (sub_data_id != -1)
  {
    time_delay_data.erase(time_delay_data.begin() + sub_data_id);
  }
  sub_data_id = get_magnitude_data_id_from_magnitude_id(bcs_data[bcs_data_id][5]);
  if (sub_data_id != -1)
  {
    magnitude_data.erase(magnitude_data.begin() + sub_data_id);
  }
  sub_data_id = get_name_data_id_from_name_id(bcs_data[bcs_data_id][6]);
  if (sub_data_id != -1)
  {
    name_data.erase(name_data.begin() + sub_data_id);
  }

  bcs_data.erase(bcs_data.begin() + bcs_data_id);
  return true;
}

bool CoreBCsTemperatures::add_magnitude(double magnitude_id, double magnitude_value)
{
  std::vector<double> v = {magnitude_id, magnitude_value};
  magnitude_data.push_back(v);
  return true;
}

bool CoreBCsTemperatures::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  name_data.push_back(v);
  return true;
}

bool CoreBCsTemperatures::add_time_delay(std::string time_delay_id, std::string time_delay_value)
{
  std::vector<std::string> v = {time_delay_id, time_delay_value};
      
  time_delay_data.push_back(v);

  return true;
}

int CoreBCsTemperatures::get_bcs_data_id_from_bc_id(int bc_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < bcs_data.size(); i++)
  {
    if (bcs_data[i][0]==bc_id)
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreBCsTemperatures::get_time_delay_data_id_from_time_delay_id(int time_delay_id)
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

int CoreBCsTemperatures::get_magnitude_data_id_from_magnitude_id(int magnitude_id)
{
  int return_int = -1;
  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    if (magnitude_data[i][0] == double(magnitude_id))
    {
      return_int = int(i);
    }
  }
  return return_int;
}

int CoreBCsTemperatures::get_name_data_id_from_name_id(int name_id)
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

std::string CoreBCsTemperatures::get_bc_parameter_export(int bc_id)
{
  CalculiXAmplitudesInterface ccx_amplitudes_iface;

  int bc_data_id;
  int sub_data_id;
  std::string str_temp = "";
  bc_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bc_data_id == -1)
  {
    return "";
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bc_data_id][3]);
  if (sub_data_id == -1)
  {
    return "";
  }
  if (bcs_data[bc_data_id][1]==0)
  {
    //str_temp.append(",OP=MOD");
  }else if (bcs_data[bc_data_id][1]==1)
  {
    str_temp.append(",OP=NEW");
  }
  if (bcs_data[bc_data_id][2]!=-1)
  {
    str_temp.append(",AMPLITUDE=" + ccx_amplitudes_iface.get_amplitude_name(bcs_data[bc_data_id][2]));
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bc_data_id][3]);
  if (bcs_data[bc_data_id][2] != -1 &&
      time_delay_data[sub_data_id][1] != "")
  {
    str_temp.append(",TIME DELAY=" + time_delay_data[sub_data_id][1]);
  }
  return str_temp;
}

int CoreBCsTemperatures::get_bc_keyword_type(int bc_id)
{
  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1)
  {
    return -1;
  }
  return bcs_data[bcs_data_id][7];
}

std::string CoreBCsTemperatures::get_bc_export(int bc_id)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1 || bcs_data[bcs_data_id].size() < 8)
  {
    return "";
  }

  int magnitude_data_id = get_magnitude_data_id_from_magnitude_id(bcs_data[bcs_data_id][5]);
  int time_delay_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bcs_data_id][3]);
  int nodeset_id = bcs_data[bcs_data_id][4];
  int keyword_type = bcs_data[bcs_data_id][7];
  if (magnitude_data_id == -1 ||
      time_delay_data_id == -1 ||
      !ccx_mesh_iface.check_nodeset_exists(nodeset_id))
  {
    return "";
  }
  if (magnitude_data[magnitude_data_id].size() < 2 ||
      time_delay_data[time_delay_data_id].size() < 2 ||
      keyword_type < 0 ||
      keyword_type > 1)
  {
    return "";
  }

  double magnitude = magnitude_data[magnitude_data_id][1];
  if (!std::isfinite(magnitude))
  {
    return "";
  }

  std::string nodeset_name = ccx_mesh_iface.get_nodeset_name(nodeset_id);
  if (nodeset_name.empty())
  {
    return "";
  }

  std::string str_temp;
  if (keyword_type == 0)
  {
    str_temp = "*BOUNDARY";
  }
  else
  {
    str_temp = "*TEMPERATURE";
  }
  str_temp.append(get_bc_parameter_export(bc_id));
  str_temp.append("\n");
  str_temp.append(nodeset_name);
  if (keyword_type == 0)
  {
    str_temp.append(",11,11");
  }
  str_temp.append("," + ccx_application_iface.to_string_scientific(magnitude) + "\n");
  return str_temp;
}

std::string CoreBCsTemperatures::get_hbc_export(int bc_id)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1 || bcs_data[bcs_data_id].size() < 8)
  {
    return "";
  }

  int time_delay_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bcs_data_id][3]);
  int nodeset_id = bcs_data[bcs_data_id][4];
  if (time_delay_data_id == -1 ||
      !ccx_mesh_iface.check_nodeset_exists(nodeset_id))
  {
    return "";
  }
  if (time_delay_data[time_delay_data_id].size() < 2)
  {
    return "";
  }

  std::string nodeset_name = ccx_mesh_iface.get_nodeset_name(nodeset_id);
  if (nodeset_name.empty())
  {
    return "";
  }

  std::string str_temp = "*BOUNDARY";
  str_temp.append(get_bc_parameter_export(bc_id));
  str_temp.append("\n");
  str_temp.append(nodeset_name + ",11,11\n");
  return str_temp;
}

std::string CoreBCsTemperatures::get_initial_temperature_export(int bc_id)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1 || bcs_data[bcs_data_id].size() < 8)
  {
    return "";
  }

  int magnitude_data_id = get_magnitude_data_id_from_magnitude_id(bcs_data[bcs_data_id][5]);
  int nodeset_id = bcs_data[bcs_data_id][4];
  if (magnitude_data_id == -1 ||
      !ccx_mesh_iface.check_nodeset_exists(nodeset_id))
  {
    return "";
  }
  if (magnitude_data[magnitude_data_id].size() < 2)
  {
    return "";
  }

  double magnitude = magnitude_data[magnitude_data_id][1];
  if (!std::isfinite(magnitude))
  {
    return "";
  }

  std::string nodeset_name = ccx_mesh_iface.get_nodeset_name(nodeset_id);
  if (nodeset_name.empty())
  {
    return "";
  }

  return "*INITIAL CONDITIONS,TYPE=TEMPERATURE\n" + nodeset_name + "," +
    ccx_application_iface.to_string_scientific(magnitude) + "\n";
}

std::string CoreBCsTemperatures::print_data()
{
  std::string str_return = "\n CoreBCsTemperatures bcs_data: \n";
  str_return.append("bc_id, OP MODE, amplitude_id, time_delay_id, nodeset_id, magnitude_id, name_id, keyword_type\n");
  for (size_t i = 0; i < bcs_data.size(); i++)
  {
    for (size_t ii = 0; ii < bcs_data[i].size(); ii++)
    {
      str_return.append(std::to_string(bcs_data[i][ii]) + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreBCsTemperatures time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value\n");
  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(time_delay_data[i][0] + " " + time_delay_data[i][1] + "\n");
  }

  str_return.append("\n CoreBCsTemperatures magnitude_data: \n");
  str_return.append("magnitude_id, magnitude_value\n");
  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    for (size_t ii = 0; ii < magnitude_data[i].size(); ii++)
    {
      str_return.append(std::to_string(magnitude_data[i][ii]) + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreBCsTemperatures name_data: \n");
  str_return.append("name_id, name\n");
  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + "\n");
  }
  return str_return;
}