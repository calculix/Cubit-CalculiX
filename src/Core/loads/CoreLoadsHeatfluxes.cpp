#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include <string>
#include <vector>
#include "Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
CoreLoadsHeatfluxes::CoreLoadsHeatfluxes()
{}

CoreLoadsHeatfluxes::~CoreLoadsHeatfluxes()
{}

bool CoreLoadsHeatfluxes::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    is_initialized = true;  
    return true;
  }
}

bool CoreLoadsHeatfluxes::update()
{
  return true;
}

bool CoreLoadsHeatfluxes::reset()
{
  loads_data.clear();
  time_delay_data.clear();
  magnitude_data.clear();
  name_data.clear();
  init();
  return true;
}

bool CoreLoadsHeatfluxes::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsHeatfluxes::create_load(std::vector<std::string> options, std::vector<double> options2)
{
  // options: op mode, amplitude, time delay, sideset, name
  // options2: magnitude
  if (options.size() != 5 || options2.size() != 1)
  {
    return false;
  }

  int op_mode = std::stoi(options[0]);
  int amplitude_id = std::stoi(options[1]);
  int sideset_id = std::stoi(options[3]);
  int load_id = 1;
  int time_delay_id = 1;
  int magnitude_id = 1;
  int name_id = 1;

  if (!loads_data.empty())
  {
    load_id = loads_data.back()[0] + 1;
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
  return add_load(load_id, op_mode, amplitude_id, time_delay_id, sideset_id, magnitude_id, name_id);
}

bool CoreLoadsHeatfluxes::add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id, int sideset_id, int magnitude_id, int name_id)
{
  std::vector<int> v = {load_id, op_mode, amplitude_id, time_delay_id, sideset_id, magnitude_id, name_id};
  loads_data.push_back(v);
  return true;
}

bool CoreLoadsHeatfluxes::modify_load(int load_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  // markers: op, amplitude, delay, sideset, magnitude, name
  if (options.size() != 5 || options2.size() != 1 || options_marker.size() != 6)
  {
    return false;
  }

  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return false;
  }

  int time_delay_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[loads_data_id][3]);
  int magnitude_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][5]);
  int name_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][6]);
  if (time_delay_data_id == -1 || magnitude_data_id == -1 || name_data_id == -1)
  {
    return false;
  }

  // Parse all requested integer changes before modifying stored data.
  int op_mode = loads_data[loads_data_id][1];
  int amplitude_id = loads_data[loads_data_id][2];
  int sideset_id = loads_data[loads_data_id][4];
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
    sideset_id = std::stoi(options[3]);
  }

  loads_data[loads_data_id][1] = op_mode;
  loads_data[loads_data_id][2] = amplitude_id;
  loads_data[loads_data_id][4] = sideset_id;
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

bool CoreLoadsHeatfluxes::delete_load(int load_id)
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
  sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][5]);
  if (sub_data_id != -1)
  {
    magnitude_data.erase(magnitude_data.begin() + sub_data_id);
  }
  sub_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][6]);
  if (sub_data_id != -1)
  {
    name_data.erase(name_data.begin() + sub_data_id);
  }

  loads_data.erase(loads_data.begin() + loads_data_id);
  return true;
}

bool CoreLoadsHeatfluxes::add_magnitude(double magnitude_id, double magnitude_value)
{
  std::vector<double> v = {magnitude_id, magnitude_value};
  magnitude_data.push_back(v);
  return true;
}

bool CoreLoadsHeatfluxes::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  name_data.push_back(v);
  return true;
}

bool CoreLoadsHeatfluxes::add_time_delay(std::string time_delay_id, std::string time_delay_value)
{
  std::vector<std::string> v = {time_delay_id, time_delay_value};
      
  time_delay_data.push_back(v);

  return true;
}

int CoreLoadsHeatfluxes::get_loads_data_id_from_load_id(int load_id)
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

int CoreLoadsHeatfluxes::get_time_delay_data_id_from_time_delay_id(int time_delay_id)
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

int CoreLoadsHeatfluxes::get_magnitude_data_id_from_magnitude_id(int magnitude_id)
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

int CoreLoadsHeatfluxes::get_name_data_id_from_name_id(int name_id)
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

std::string CoreLoadsHeatfluxes::get_load_parameter_export(int load_id)
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

std::string CoreLoadsHeatfluxes::get_load_export(int load_id)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return "";
  }

  int magnitude_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][5]);
  int time_delay_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[loads_data_id][3]);
  if (magnitude_data_id == -1 || time_delay_data_id == -1)
  {
    return "";
  }

  int sideset_id = loads_data[loads_data_id][4];
  if (!ccx_mesh_iface.check_sideset_exists(sideset_id))
  {
    return "";
  }

  std::vector<std::vector<std::string>> faces = ccx_mesh_iface.get_sideset_face(sideset_id);
  if (faces.empty())
  {
    return "";
  }

  std::string str_temp = "*DFLUX";
  str_temp.append(get_load_parameter_export(load_id));
  str_temp.append("\n");

  for (size_t i = 0; i < faces.size(); i++)
  {
    if (faces[i].size() < 4)
    {
      return "";
    }

    int side = std::stoi(faces[i][2]);
    if (side < 1 || side > 6)
    {
      return "";
    }

    str_temp.append(
        faces[i][1] + ",S" + std::to_string(side) + "," +
        ccx_application_iface.to_string_scientific(magnitude_data[magnitude_data_id][1]) + "\n");
  }

  return str_temp;
}

std::string CoreLoadsHeatfluxes::print_data()
{
  std::string str_return = "\n CoreLoadsHeatfluxes loads_data: \n";
  str_return.append("load_id, OP MODE, amplitude_id, time_delay_id, sideset_id, magnitude_id, name_id\n");
  for (size_t i = 0; i < loads_data.size(); i++)
  {
    for (size_t ii = 0; ii < loads_data[i].size(); ii++)
    {
      str_return.append(std::to_string(loads_data[i][ii]) + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreLoadsHeatfluxes time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value\n");
  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(time_delay_data[i][0] + " " + time_delay_data[i][1] + "\n");
  }

  str_return.append("\n CoreLoadsHeatfluxes magnitude_data: \n");
  str_return.append("magnitude_id, magnitude_value\n");
  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    for (size_t ii = 0; ii < magnitude_data[i].size(); ii++)
    {
      str_return.append(std::to_string(magnitude_data[i][ii]) + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreLoadsHeatfluxes name_data: \n");
  str_return.append("name_id, name\n");
  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + "\n");
  }
  return str_return;
}