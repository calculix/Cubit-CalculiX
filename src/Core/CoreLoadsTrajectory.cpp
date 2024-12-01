#include "CoreLoadsTrajectory.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreLoadsTrajectory::CoreLoadsTrajectory()
{}

CoreLoadsTrajectory::~CoreLoadsTrajectory()
{}

bool CoreLoadsTrajectory::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
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
  fire_ray_surface_data.clear();
  time_data.clear();
  direction_data.clear();
  magnitude_data.clear();
  init();
  return true;
}

bool CoreLoadsTrajectory::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsTrajectory::create_load(std::vector<std::string> options, std::vector<int> options2)
{
  int load_id;
  int load_last;
  int sub_id;
  int sub_last;
  int op_mode;
  int curve_id;
  int vertex_id;
  int fire_ray_surface_id;
  int direction_id;
  int magnitude_id;
  int time_id;

  if (loads_data.size()==0)
  {
    load_id = 1;
  }
  else
  {
    load_last = int(loads_data.size()) - 1;
    load_id = loads_data[load_last][0] + 1;
  }

  // OP MODE
  op_mode = std::stoi(options[0]);
  
  // curve
  curve_id = std::stoi(options[1]);

  // vertex
  vertex_id = std::stoi(options[2]);

  // fire ray
  fire_ray_surface_id = load_id;
  for (size_t i = 0; i < options2.size(); i++)
  {
    this->add_fire_ray_surface(fire_ray_surface_id, options2[i]);
  }
  
  // direction
  if (direction_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(direction_data.size()) - 1;
    sub_id = std::stoi(direction_data[sub_last][0]) + 1;
  }
  direction_id = sub_id;
  this->add_direction(std::to_string(sub_id), options[3], options[4], options[5]);

  // magnitude
  if (magnitude_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(magnitude_data.size()) - 1;
    sub_id = std::stoi(magnitude_data[sub_last][0]) + 1;
  }
  magnitude_id = sub_id;
  this->add_magnitude(std::to_string(sub_id), options[6]);

  // time
  if (time_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(time_data.size()) - 1;
    sub_id = std::stoi(time_data[sub_last][0]) + 1;
  }
  time_id = sub_id;
  this->add_time(std::to_string(sub_id), options[7], options[8]);

  this->add_load(load_id, op_mode, curve_id, vertex_id, fire_ray_surface_id, direction_id, magnitude_id, time_id);
  return true;
}

bool CoreLoadsTrajectory::add_load(int load_id, int op_mode, int curve_id, int vertex_id, int fire_ray_surface_id, int direction_id, int magnitude_id, int time_id)
{
  std::vector<int> v = {load_id, op_mode, curve_id, vertex_id, fire_ray_surface_id, direction_id, magnitude_id, time_id};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectory::modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2)
{
  int sub_data_id;
  std::vector<int> sub_data_ids;
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  
  if (loads_data_id == -1)
  {
    return false;
  } else {
    // OP MODE
    if (options_marker[0]==1)
    {
      loads_data[loads_data_id][1] = std::stoi(options[0]);
    }
    // curve
    if (options_marker[1]==1)
    {
      loads_data[loads_data_id][2] = std::stoi(options[1]);
    }
    // vertex
    if (options_marker[2]==1)
    {
      loads_data[loads_data_id][3] = std::stoi(options[2]);
    }
    // fire ray
    if (options_marker[3]==1)
    {
      // surface ids
      sub_data_ids = get_fire_ray_surface_data_ids_from_fire_ray_surface_id(loads_data[loads_data_id][4]);
      if (options2.size()!=0)
      {
        if (options2.size()==sub_data_ids.size())
        {
          for (size_t i = 0; i < options2.size(); i++)
          {
            fire_ray_surface_data[sub_data_ids[i]][0] = loads_data[loads_data_id][4];
            fire_ray_surface_data[sub_data_ids[i]][1] = options2[i];
          }
        }else{
          // first delete and then make a push back
          // delete from back to begin so that we don't have to care about mismatching id's
          for (size_t i = sub_data_ids.size(); i > 0; i--)
          {
            fire_ray_surface_data.erase(fire_ray_surface_data.begin() + sub_data_ids[i-1]);
          }
          
          for (size_t i = 0; i < options2.size(); i++)
          {
            add_fire_ray_surface(loads_data[loads_data_id][4],options2[i]);
          }
        }
      }

    }
    // direction
    if (options_marker[4]==1)
    {
      sub_data_id = get_direction_data_id_from_direction_id(loads_data[loads_data_id][5]);
      direction_data[sub_data_id][1] = options[4];
    }
    if (options_marker[5]==1)
    {
      sub_data_id = get_direction_data_id_from_direction_id(loads_data[loads_data_id][5]);
      direction_data[sub_data_id][2] = options[5];
    }
    if (options_marker[6]==1)
    {
      sub_data_id = get_direction_data_id_from_direction_id(loads_data[loads_data_id][5]);
      direction_data[sub_data_id][3] = options[6];
    }
    // magnitude
    if (options_marker[7]==1)
    {
      sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][6]);
      magnitude_data[sub_data_id][1] = options[7];
    }
    // time
    if (options_marker[8]==1)
    {
      sub_data_id = get_time_data_id_from_time_id(loads_data[loads_data_id][7]);
      time_data[sub_data_id][1] = options[8];
    }
    if (options_marker[9]==1)
    {
      sub_data_id = get_time_data_id_from_time_id(loads_data[loads_data_id][7]);
      time_data[sub_data_id][1] = options[9];
    }
    return true;
  }
}

bool CoreLoadsTrajectory::delete_load(int load_id)
{
  int sub_data_id;
  std::vector<int> sub_data_ids;
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return false;
  } else {
    // time
    sub_data_id = get_time_data_id_from_time_id(loads_data[loads_data_id][7]);
    if (sub_data_id != -1){
      time_data.erase(time_data.begin() + sub_data_id);
    }
    // direction
    sub_data_id = get_direction_data_id_from_direction_id(loads_data[loads_data_id][5]);
    if (sub_data_id != -1){
      direction_data.erase(direction_data.begin() + sub_data_id);
    }
    // magnitude
    sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][6]);
    if (sub_data_id != -1){
      magnitude_data.erase(magnitude_data.begin() + sub_data_id);
    }
    // fire ray surfaces
    sub_data_ids = get_fire_ray_surface_data_ids_from_fire_ray_surface_id(loads_data[loads_data_id][4]);
    for (size_t i = sub_data_ids.size(); i > 0; i--)
    {
      fire_ray_surface_data.erase(fire_ray_surface_data.begin() + sub_data_ids[i-1]);
    }
    loads_data.erase(loads_data.begin() + loads_data_id);
    return true;
  }
}

bool CoreLoadsTrajectory::add_time(std::string time_id, std::string t_begin, std::string t_end)
{
  std::vector<std::string> v = {time_id, t_begin, t_end};
      
  time_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectory::add_direction(std::string direction_id, std::string x, std::string y, std::string z)
{
  std::vector<std::string> v = {direction_id, x, y, z};
      
  direction_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectory::add_magnitude(std::string magnitude_id, std::string magnitude_value)
{
  std::vector<std::string> v = {magnitude_id, magnitude_value};
      
  magnitude_data.push_back(v);

  return true;
}

bool CoreLoadsTrajectory::add_fire_ray_surface(int fire_ray_surface_id, int surface_id)
{
  std::vector<int> v = {fire_ray_surface_id, surface_id};
      
  fire_ray_surface_data.push_back(v);

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

int CoreLoadsTrajectory::get_time_data_id_from_time_id(int time_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < time_data.size(); i++)
  {
    if (time_data[i][0]==std::to_string(time_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsTrajectory::get_direction_data_id_from_direction_id(int direction_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < direction_data.size(); i++)
  {
    if (direction_data[i][0]==std::to_string(direction_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsTrajectory::get_magnitude_data_id_from_magnitude_id(int magnitude_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    if (magnitude_data[i][0]==std::to_string(magnitude_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

std::vector<int> CoreLoadsTrajectory::get_fire_ray_surface_data_ids_from_fire_ray_surface_id(int fire_surface_id)
{ 
  std::vector<int> return_int;
  for (size_t i = 0; i < fire_ray_surface_data.size(); i++)
  {
    if (fire_ray_surface_data[i][0]==fire_surface_id)
    {
        return_int.push_back(int(i));
    }  
  }
  return return_int;
}

std::string CoreLoadsTrajectory::get_load_export(int load_id)
{
  
  int load_data_id;
  int sub_data_id;
  std::string str_temp = "**Trajectory";
  
  /*
  load_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data[load_data_id][1]==0)
  {
    //str_temp.append(",OP=MOD");
  }else if (loads_data[load_data_id][1]==1)
  {
    str_temp.append(",OP=NEW");
  }
  if (loads_data[load_data_id][2]!=-1)
  {
    str_temp.append(",AMPLITUDE=" + ccx_iface->get_amplitude_name(loads_data[load_data_id][2]));
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[load_data_id][3]);
  if (time_delay_data[sub_data_id][1]!="")
  {
    str_temp.append(",TIME DELAY=" + time_delay_data[sub_data_id][1]);
  }
  str_temp.append("\n");
  
  // second line
  str_temp.append(ccx_iface->get_block_name(loads_data[load_data_id][4]) + ",CENTRIF,");
  
  sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[load_data_id][6]);
  str_temp.append(magnitude_data[sub_data_id][1] + ",");
  
  sub_data_id = get_coordinate_data_id_from_coordinate_id(loads_data[load_data_id][7]);
  str_temp.append(coordinate_data[sub_data_id][1] + "," + coordinate_data[sub_data_id][2] + "," + coordinate_data[sub_data_id][3] + ",");

  sub_data_id = get_direction_data_id_from_direction_id(loads_data[load_data_id][5]);
  str_temp.append(direction_data[sub_data_id][1] + "," + direction_data[sub_data_id][2] + "," + direction_data[sub_data_id][3]);

*/
  return str_temp;
}

std::string CoreLoadsTrajectory::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsTrajectory loads_data: \n";
  str_return.append("load_id, OP MODE, curve_id, vertex_id, fire_ray_surface_id, direction_id, magnitude_id, time_id \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " " + std::to_string(loads_data[i][1]) + " " + std::to_string(loads_data[i][2]) + " " + std::to_string(loads_data[i][3]) + " " + std::to_string(loads_data[i][4]) + " " + std::to_string(loads_data[i][5]) + " " + std::to_string(loads_data[i][6]) + " " + std::to_string(loads_data[i][7]) + " \n");
  }

  str_return.append("\n CoreLoadsTrajectory time_data: \n");
  str_return.append("time_id, t_begin, t_end \n");

  for (size_t i = 0; i < time_data.size(); i++)
  {
    str_return.append(time_data[i][0] + " " + time_data[i][1] + " " + time_data[i][2] + " \n");
  }

  str_return.append("\n CoreLoadsTrajectory direction_data: \n");
  str_return.append("direction_id, x, y, z \n");

  for (size_t i = 0; i < direction_data.size(); i++)
  {
    str_return.append(direction_data[i][0] + " " + direction_data[i][1] + " " + direction_data[i][2] + " " + direction_data[i][3] + " \n");
  }
  
  str_return.append("\n CoreLoadsTrajectory magnitude_data: \n");
  str_return.append("magnitude_id, magnitude_value \n");

  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    str_return.append(magnitude_data[i][0] + " " + magnitude_data[i][1] + " \n");
  }

  str_return.append("\n CoreLoadsTrajectory fire_ray_surface_data: \n");
  str_return.append("fire_ray_surface_id, surface_id \n");

  for (size_t i = 0; i < fire_ray_surface_data.size(); i++)
  {
    str_return.append(std::to_string(fire_ray_surface_data[i][0]) + " " + std::to_string(fire_ray_surface_data[i][1]) + " \n");
  }

  return str_return;
}