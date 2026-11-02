#include "Core/application/SettingsRead.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include <string>
#include <vector>
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitInterface.hpp"
#include "Core/loads/CoreLoadsTrajectoryHeatflux.hpp"
#include "Core/loads/CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <algorithm>
#include "StopWatch.hpp"

CoreLoadsTrajectory::CoreLoadsTrajectory()
{
  this->heatflux = new CoreLoadsTrajectoryHeatflux();
  this->bodyheatfluxsphere = new CoreLoadsTrajectoryBodyHeatfluxSphere();
}

CoreLoadsTrajectory::~CoreLoadsTrajectory()
{}

bool CoreLoadsTrajectory::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    heatflux->init();
    bodyheatfluxsphere->init();
    is_initialized = true;  
    return true;
  }
}

bool CoreLoadsTrajectory::update()
{ 
  return true;
}

bool CoreLoadsTrajectory::reset()
{
  loads_data.clear();
  heatflux->reset();
  bodyheatfluxsphere->reset();
  init();
  heatflux->init();
  bodyheatfluxsphere->init();
  return true;
}

bool CoreLoadsTrajectory::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsTrajectory::create_load(std::string load_type, std::vector<std::string> options, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  int load_id;
  int load_last;
  int load_type_int;
  int sub_id;
  int sub_last;

  if (loads_data.size()==0)
  {
    load_id = 1;
  }
  else
  {
    load_last = int(loads_data.size()) - 1;
    load_id = loads_data[load_last][0] + 1;
  }

  if (load_type == "HEATFLUX")
  {
    heatflux->create_load(options,options2,options3);
    load_type_int = 1;
    sub_last = int(heatflux->loads_data.size()) - 1;
    sub_id = heatflux->loads_data[sub_last][0];
  }else if (load_type == "BODYHEATFLUXSPHERE")
  {
    bodyheatfluxsphere->create_load(options,options2,options3);
    load_type_int = 2;
    sub_last = int(bodyheatfluxsphere->loads_data.size()) - 1;
    sub_id = bodyheatfluxsphere->loads_data[sub_last][0];
  }else{
    // no matching load type -> break
    std::string output = "no matching load type !!! " + load_type + " !!! -> break!\n";
    PRINT_INFO(output.c_str());
    return false;
  }
  
  this->add_load(load_id,load_type_int,sub_id);

  return true;
}

bool CoreLoadsTrajectory::add_load(int load_id, int load_type, int sub_load_id)
{
  std::vector<int> v = {load_id, load_type, sub_load_id};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectory::modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  //int sub_data_id;
  std::vector<int> sub_data_ids;
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  
  if (loads_data_id == -1)
  {
    return false;
  } else {
    //HEATFLUX
    if (loads_data[loads_data_id][1]==1)
    {
      return heatflux->modify_load(loads_data[loads_data_id][2],options,options_marker,options2,options3);
    }
    //BODYHEATFLUX SPHERE
    else if (loads_data[loads_data_id][1]==2)
    {
      return bodyheatfluxsphere->modify_load(loads_data[loads_data_id][2],options,options_marker,options2,options3);
    }else{
     return false;
    }    
  }
  return false;
}

bool CoreLoadsTrajectory::delete_load(int load_id)
{
  const int index = get_loads_data_id_from_load_id(load_id);

  if (index == -1)
  {
    return false;
  }

  const int type = loads_data[index][1];
  const int subload_id = loads_data[index][2];
  bool deleted = false;

  if (type == 1)
  {
    deleted = heatflux->get_loads_data_id_from_load_id(subload_id) == -1 || heatflux->delete_load(subload_id);
  }
  else if (type == 2)
  {
    deleted = bodyheatfluxsphere->get_loads_data_id_from_load_id(subload_id) == -1 || bodyheatfluxsphere->delete_load(subload_id);
  }
  else
  {
    return false;
  }

  if (!deleted)
  {
    return false;
  }

  loads_data.erase(loads_data.begin() + index);
  return true;
}

int CoreLoadsTrajectory::get_loads_data_id_from_load_id(int load_id)
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

std::string CoreLoadsTrajectory::get_load_type(int load_id)
{ 
  std::string return_str = "";
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  
  if (loads_data_id == -1)
  {
    return return_str;
  } else {
    if (loads_data[loads_data_id][1]==1)
    {
      return_str = "HEATFLUX";
    }else if (loads_data[loads_data_id][1]==2)
    {
      return_str = "BODYHEATFLUXSPHERE";
    }
  }
  
  return return_str;
}

int CoreLoadsTrajectory::get_subload_id(int load_id)
{ 
  int return_int = -1;
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  
  if (loads_data_id == -1)
  {
    return return_int;
  } else {
    return_int = loads_data[loads_data_id][2];
  }
  
  return return_int;
}

std::string CoreLoadsTrajectory::get_name_from_load_id(int load_id)
{ 
  std::string return_str = "";
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  
  if (loads_data_id == -1)
  {
    return return_str;
  } else {
    if (loads_data[loads_data_id][1]==1) //HEATFLUX
    {
      int name_data_id = heatflux->get_name_data_id_from_name_id(loads_data[loads_data_id][2]);
      return_str = heatflux->name_data[name_data_id][1];
    }else if (loads_data[loads_data_id][1]==2) //BODYHEATFLUX SPHERE
    {
      int name_data_id = bodyheatfluxsphere->get_name_data_id_from_name_id(loads_data[loads_data_id][2]);
      return_str = bodyheatfluxsphere->name_data[name_data_id][1];
    }
  }
  
  return return_str;
}

bool CoreLoadsTrajectory::prepare_export()
{
  CalculiXStepsInterface ccx_steps_iface;

  if (!ccx_steps_iface.step_utility_save_backup())
  {
    return false;
  }

  if (!bodyheatfluxsphere->prepare_export())
  {
    return false;
  }

  return heatflux->prepare_export();
}

bool CoreLoadsTrajectory::clean_export()
{
  CalculiXStepsInterface ccx_steps_iface;

  const bool sphere_ok = bodyheatfluxsphere->clean_export();
  const bool heatflux_ok = heatflux->clean_export();
  const bool restore_ok = ccx_steps_iface.step_utility_load_backup();

  return sphere_ok && heatflux_ok && restore_ok;
}  

std::string CoreLoadsTrajectory::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsTrajectory loads_data: \n";
  str_return.append("load_id, load_type, sub_load_id \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " " + std::to_string(loads_data[i][1]) + " " + std::to_string(loads_data[i][2]) + " \n");
  }

  return str_return;
}

std::vector<int> CoreLoadsTrajectory::get_load_ids() const
{
  std::vector<int> ids;
  ids.reserve(loads_data.size());

  for (const auto& load : loads_data)
  {
    ids.push_back(load[0]);
  }

  return ids;
}

std::vector<std::vector<std::vector<int>>> CoreLoadsTrajectory::get_bodyheatfluxsphere_draw_element_ids(int trajectory_id)
{
  return bodyheatfluxsphere->get_draw_element_ids(get_subload_id(trajectory_id, 2));
}

bool CoreLoadsTrajectory::prepare_export_if_needed()
{
  if (loads_data.empty())
  {
    return true;
  }

  return prepare_export();
}

bool CoreLoadsTrajectory::clean_export_if_needed()
{
  if (loads_data.empty())
  {
    return true;
  }

  return clean_export();
}

int CoreLoadsTrajectory::get_subload_id(int trajectory_id, int expected_type)
{
  const int index = get_loads_data_id_from_load_id(trajectory_id);

  if (index == -1)
  {
    return -1;
  }

  if (loads_data[index][1] != expected_type)
  {
    return -1;
  }

  return loads_data[index][2];
}

std::vector<int> CoreLoadsTrajectory::get_heatflux_node_ids(int trajectory_id)
{
  return heatflux->get_node_ids(get_subload_id(trajectory_id, 1));
}

std::vector<int> CoreLoadsTrajectory::get_heatflux_edge_ids(int trajectory_id)
{
  return heatflux->get_edge_ids(get_subload_id(trajectory_id, 1));
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_heatflux_hit_coordinates(int trajectory_id)
{
  return heatflux->get_hit_coordinates(get_subload_id(trajectory_id, 1));
}

std::vector<std::vector<std::vector<int>>> CoreLoadsTrajectory::get_heatflux_face_ids(int trajectory_id)
{
  return heatflux->get_face_ids(get_subload_id(trajectory_id, 1));
}

std::vector<std::vector<std::vector<int>>> CoreLoadsTrajectory::get_heatflux_draw_face_ids(int trajectory_id)
{
  return heatflux->get_draw_face_ids(get_subload_id(trajectory_id, 1));
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_heatflux_times(int trajectory_id)
{
  return heatflux->get_times(get_subload_id(trajectory_id, 1));
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_heatflux_radius(int trajectory_id)
{
  return heatflux->get_radius(get_subload_id(trajectory_id, 1));
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_heatflux_magnitude(int trajectory_id)
{
  return heatflux->get_magnitude(get_subload_id(trajectory_id, 1));
}

std::vector<int> CoreLoadsTrajectory::get_bodyheatfluxsphere_node_ids(int trajectory_id)
{
  return bodyheatfluxsphere->get_node_ids(get_subload_id(trajectory_id, 2));
}

std::vector<int> CoreLoadsTrajectory::get_bodyheatfluxsphere_edge_ids(int trajectory_id)
{
  return bodyheatfluxsphere->get_edge_ids(get_subload_id(trajectory_id, 2));
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_bodyheatfluxsphere_hit_coordinates(int trajectory_id)
{
  return bodyheatfluxsphere->get_hit_coordinates(get_subload_id(trajectory_id, 2));
}

std::vector<std::vector<std::vector<int>>> CoreLoadsTrajectory::get_bodyheatfluxsphere_element_ids(int trajectory_id)
{
  return bodyheatfluxsphere->get_element_ids(get_subload_id(trajectory_id, 2));
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_bodyheatfluxsphere_times(int trajectory_id)
{
  return bodyheatfluxsphere->get_times(get_subload_id(trajectory_id, 2));
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_bodyheatfluxsphere_radius(int trajectory_id)
{
  return bodyheatfluxsphere->get_radius(get_subload_id(trajectory_id, 2));
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_bodyheatfluxsphere_depth(int trajectory_id)
{
  return bodyheatfluxsphere->get_depth(get_subload_id(trajectory_id, 2));
}

std::vector<std::vector<double>> CoreLoadsTrajectory::get_bodyheatfluxsphere_magnitude(int trajectory_id)
{
  return bodyheatfluxsphere->get_magnitude(get_subload_id(trajectory_id, 2));
}


bool CoreLoadsTrajectory::get_settings(int id, LoadsTrajectorySettings& settings, std::string& error) const
{
  settings = {};
  error.clear();
  const auto* record = SettingsRead::row(loads_data, id, 3, error);

  if (record == nullptr)
  {
    return false;
  }

  settings.id = id;
  settings.type = (*record)[1];
  if (settings.type == 1 && heatflux != nullptr)
  {
    const auto* sub = SettingsRead::row(heatflux->loads_data, (*record)[2], 10, error);
    if (sub == nullptr)
    {
      return false;
    }
    settings.name = SettingsRead::field(heatflux->name_data, (*sub)[9], 1, error);
    settings.loadType = "Heatflux";
    settings.operation = (*sub)[1] == 1 ? "new" : "mod";
    settings.curve = SettingsRead::optional_id((*sub)[2]);
    settings.vertex = SettingsRead::optional_id((*sub)[3]);
    settings.direction = SettingsRead::field(heatflux->direction_data, (*sub)[5], 1, error) + " " + SettingsRead::field(heatflux->direction_data, (*sub)[5], 2, error) + " " + SettingsRead::field(heatflux->direction_data, (*sub)[5], 3, error);
    settings.timeBegin = SettingsRead::field(heatflux->time_data, (*sub)[7], 1, error);
    settings.timeEnd = SettingsRead::field(heatflux->time_data, (*sub)[7], 2, error);
    for (const auto& entry : heatflux->fire_ray_surface_data)
    {
      if (entry.size() >= 2 && entry[0] == (*sub)[4])
      {
        if (!settings.surfaces.empty())
        {
          settings.surfaces += " ";
        }
        settings.surfaces += SettingsRead::text(entry[1]);
      }
    }
    const auto radii = SettingsRead::table(heatflux->radius_data, (*sub)[8], 1, error);
    const auto magnitudes = SettingsRead::table(heatflux->magnitude_data, (*sub)[6], 1, error);
    if (radii.size() != magnitudes.size())
    {
      error = "Trajectory sample arrays have inconsistent lengths.";
      return false;
    }
    for (std::size_t index = 0; index < radii.size(); ++index)
    {
      settings.samples.push_back({radii[index][0], magnitudes[index][0]});
    }
  }
  else if (settings.type == 2 && bodyheatfluxsphere != nullptr)
  {
    const auto* sub = SettingsRead::row(bodyheatfluxsphere->loads_data, (*record)[2], 12, error);
    if (sub == nullptr)
    {
      return false;
    }
    settings.name = SettingsRead::field(bodyheatfluxsphere->name_data, (*sub)[10], 1, error);
    settings.loadType = "BodyHeatfluxSphere";
    settings.modelChange = (*sub)[11] == 1 ? "add" : ((*sub)[11] == 2 ? "remove" : "off");
    settings.operation = (*sub)[1] == 1 ? "new" : "mod";
    settings.curve = SettingsRead::optional_id((*sub)[2]);
    settings.vertex = SettingsRead::optional_id((*sub)[3]);
    settings.direction = SettingsRead::field(bodyheatfluxsphere->direction_data, (*sub)[5], 1, error) + " " + SettingsRead::field(bodyheatfluxsphere->direction_data, (*sub)[5], 2, error) + " " + SettingsRead::field(bodyheatfluxsphere->direction_data, (*sub)[5], 3, error);
    settings.timeBegin = SettingsRead::field(bodyheatfluxsphere->time_data, (*sub)[7], 1, error);
    settings.timeEnd = SettingsRead::field(bodyheatfluxsphere->time_data, (*sub)[7], 2, error);
    for (const auto& entry : bodyheatfluxsphere->fire_ray_surface_data)
    {
      if (entry.size() >= 2 && entry[0] == (*sub)[4])
      {
        if (!settings.surfaces.empty())
        {
          settings.surfaces += " ";
        }
        settings.surfaces += SettingsRead::text(entry[1]);
      }
    }
    const auto radii = SettingsRead::table(bodyheatfluxsphere->radius_data, (*sub)[8], 1, error);
    const auto magnitudes = SettingsRead::table(bodyheatfluxsphere->magnitude_data, (*sub)[6], 1, error);
    const auto depths = SettingsRead::table(bodyheatfluxsphere->depth_data, (*sub)[9], 1, error);
    if (radii.size() != magnitudes.size() || radii.size() != depths.size())
    {
      error = "Trajectory sample arrays have inconsistent lengths.";
      return false;
    }
    for (std::size_t index = 0; index < radii.size(); ++index)
    {
      settings.samples.push_back({radii[index][0], depths[index][0], magnitudes[index][0]});
    }
  }
  else
  {
    error = "Trajectory subtype data is unavailable.";
    return false;
  }

  if (!error.empty())
  {
    settings = {};
    return false;
  }

  return true;
}
