#include "CoreLoadsGravity.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreLoadsGravity::CoreLoadsGravity()
{}

CoreLoadsGravity::~CoreLoadsGravity()
{}

bool CoreLoadsGravity::init()
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

bool CoreLoadsGravity::update()
{ 
  return true;
}

bool CoreLoadsGravity::reset()
{
  loads_data.clear();
  time_delay_data.clear();
  direction_data.clear();
  init();
  return true;
}

bool CoreLoadsGravity::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsGravity::create_load(std::vector<std::string> options)
{
  int load_id;
  int load_last;
  int sub_id;
  int sub_last;
  int op_mode;
  int amplitude_id;
  int block_id;
  int time_delay_id;
  int direction_id;
  int magnitude_id;
  
  if (loads_data.size()==0)
  {
    load_id = 1;
  }
  else
  {
    load_last = loads_data.size() - 1;
    load_id = loads_data[load_last][0] + 1;
  }

  // OP MODE
  op_mode = std::stoi(options[0]);
  
  // AMPLITUDE
  amplitude_id = std::stoi(options[1]);

  // time_delay
  if (time_delay_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = time_delay_data.size() - 1;
    sub_id = std::stoi(time_delay_data[sub_last][0]) + 1;
  }
  time_delay_id = sub_id;
  this->add_time_delay(std::to_string(sub_id), options[2]);

  // BLOCK
  block_id = std::stoi(options[3]);
  
  // direction
  if (direction_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = direction_data.size() - 1;
    sub_id = std::stoi(direction_data[sub_last][0]) + 1;
  }
  direction_id = sub_id;
  this->add_direction(std::to_string(sub_id), options[4], options[5], options[6]);

  // magnitude
  if (magnitude_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = magnitude_data.size() - 1;
    sub_id = std::stoi(magnitude_data[sub_last][0]) + 1;
  }
  magnitude_id = sub_id;
  this->add_magnitude(std::to_string(sub_id), options[7]);

  this->add_load(load_id, op_mode, amplitude_id, time_delay_id, block_id, direction_id, magnitude_id);
  return true;
}

bool CoreLoadsGravity::add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id, int block_id, int direction_id, int magnitude_id)
{
  std::vector<int> v = {load_id, op_mode, amplitude_id, time_delay_id, block_id, direction_id, magnitude_id};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsGravity::modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker)
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
    // AMPLITUDE
    if (options_marker[1]==1)
    {
      loads_data[loads_data_id][2] = std::stoi(options[1]);
    }
    // time delay
    if (options_marker[2]==1)
    {
      sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[loads_data_id][3]);
      time_delay_data[sub_data_id][1] = options[2];
    }
    // BLOCK
    if (options_marker[3]==1)
    {
      loads_data[loads_data_id][4] = std::stoi(options[3]);
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
    return true;
  }
}

bool CoreLoadsGravity::delete_load(int load_id)
{
  int sub_data_id;
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return false;
  } else {
    // time delay
    sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[loads_data_id][3]);
    if (sub_data_id != -1){
      time_delay_data.erase(time_delay_data.begin() + sub_data_id);
    }
    // direction
    sub_data_id = get_direction_data_id_from_direction_id(loads_data[loads_data_id][5]);
    if (sub_data_id != -1){
      direction_data.erase(direction_data.begin() + sub_data_id);
    }
    // magnitude
    sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][5]);
    if (sub_data_id != -1){
      magnitude_data.erase(magnitude_data.begin() + sub_data_id);
    }
    loads_data.erase(loads_data.begin() + loads_data_id);
    return true;
  }
}

bool CoreLoadsGravity::add_time_delay(std::string time_delay_id, std::string time_delay_value)
{
  std::vector<std::string> v = {time_delay_id, time_delay_value};
      
  time_delay_data.push_back(v);

  return true;
}

bool CoreLoadsGravity::add_direction(std::string direction_id, std::string x, std::string y, std::string z)
{
  std::vector<std::string> v = {direction_id, x, y, z};
      
  direction_data.push_back(v);

  return true;
}

bool CoreLoadsGravity::add_magnitude(std::string magnitude_id, std::string magnitude_value)
{
  std::vector<std::string> v = {magnitude_id, magnitude_value};
      
  magnitude_data.push_back(v);

  return true;
}

int CoreLoadsGravity::get_loads_data_id_from_load_id(int load_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < loads_data.size(); i++)
  {
    if (loads_data[i][0]==load_id)
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreLoadsGravity::get_time_delay_data_id_from_time_delay_id(int time_delay_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    if (time_delay_data[i][0]==std::to_string(time_delay_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreLoadsGravity::get_direction_data_id_from_direction_id(int direction_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < direction_data.size(); i++)
  {
    if (direction_data[i][0]==std::to_string(direction_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreLoadsGravity::get_magnitude_data_id_from_magnitude_id(int magnitude_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    if (magnitude_data[i][0]==std::to_string(magnitude_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

std::string CoreLoadsGravity::get_load_export(int load_id)
{
  int load_data_id;
  int sub_data_id;
  std::string str_temp = "*DLOAD";
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
  str_temp.append(ccx_iface->get_block_name(loads_data[load_data_id][4]) + ",GRAV,");
  
  sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[load_data_id][6]);
  str_temp.append(magnitude_data[sub_data_id][1] + ",");
  
  sub_data_id = get_direction_data_id_from_direction_id(loads_data[load_data_id][5]);
  str_temp.append(direction_data[sub_data_id][1] + "," + direction_data[sub_data_id][2] + "," + direction_data[sub_data_id][3]);

  return str_temp;
}

std::string CoreLoadsGravity::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsGravity loads_data: \n";
  str_return.append("load_id, OP MODE, amplitude_id, time_delay_id, block_id, direction_id, magnitude_id \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " " + std::to_string(loads_data[i][1]) + " " + std::to_string(loads_data[i][2]) + " " + std::to_string(loads_data[i][3]) + " " + std::to_string(loads_data[i][4]) + " " + std::to_string(loads_data[i][5]) + " " + std::to_string(loads_data[i][6]) + " \n");
  }

  str_return.append("\n CoreLoadsGravity time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value \n");

  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(time_delay_data[i][0] + " " + time_delay_data[i][1] + " \n");
  }

  str_return.append("\n CoreLoadsGravity direction_data: \n");
  str_return.append("direction_id, x, y, z \n");

  for (size_t i = 0; i < direction_data.size(); i++)
  {
    str_return.append(direction_data[i][0] + " " + direction_data[i][1] + " " + direction_data[i][2] + " " + direction_data[i][3] + " \n");
  }
  
  str_return.append("\n CoreLoadsGravity magnitude_data: \n");
  str_return.append("magnitude_id, magnitude_value \n");

  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    str_return.append(magnitude_data[i][0] + " " + magnitude_data[i][1] + " \n");
  }

  return str_return;
}