#include "CoreLoadsFilm.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreLoadsFilm::CoreLoadsFilm()
{}

CoreLoadsFilm::~CoreLoadsFilm()
{}

bool CoreLoadsFilm::init()
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

bool CoreLoadsFilm::update()
{ 
  return true;
}

bool CoreLoadsFilm::reset()
{
  loads_data.clear();
  time_delay_data.clear();
  temperature_data.clear();
  coefficient_data.clear();
  init();
  return true;
}

bool CoreLoadsFilm::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsFilm::create_load(std::vector<std::string> options)
{
  int load_id;
  int load_last;
  int sub_id;
  int sub_last;
  int op_mode;
  int amplitude_id;
  int sideset_id;
  int time_delay_id;
  int temperature_id;
  int coefficient_id;
  
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
  
  // AMPLITUDE
  amplitude_id = std::stoi(options[1]);

  // time_delay
  if (time_delay_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(time_delay_data.size()) - 1;
    sub_id = std::stoi(time_delay_data[sub_last][0]) + 1;
  }
  time_delay_id = sub_id;
  this->add_time_delay(std::to_string(sub_id), options[2]);

  // SIDESET
  sideset_id = std::stoi(options[3]);
  
  // SINK TEMPERATURE
  if (temperature_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(temperature_data.size()) - 1;
    sub_id = std::stoi(temperature_data[sub_last][0]) + 1;
  }
  temperature_id = sub_id;
  this->add_temperature(std::to_string(sub_id), options[4]);

  // COEFFICIENT
  if (coefficient_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(coefficient_data.size()) - 1;
    sub_id = std::stoi(coefficient_data[sub_last][0]) + 1;
  }
  coefficient_id = sub_id;
  this->add_coefficient(std::to_string(sub_id), options[5]);

  this->add_load(load_id, op_mode, amplitude_id, time_delay_id, sideset_id, temperature_id, coefficient_id);
  return true;
}

bool CoreLoadsFilm::add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id, int sideset_id, int temperature_id, int coefficient_id)
{
  std::vector<int> v = {load_id, op_mode, amplitude_id, time_delay_id, sideset_id, temperature_id, coefficient_id};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsFilm::modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker)
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
    // SIDESET
    if (options_marker[3]==1)
    {
      loads_data[loads_data_id][4] = std::stoi(options[3]);
    }
    // TEMPERATURE
    if (options_marker[4]==1)
    {
      sub_data_id = get_temperature_data_id_from_temperature_id(loads_data[loads_data_id][5]);
      temperature_data[sub_data_id][1] = options[4];
    }
    // COEFFICIENt
    if (options_marker[5]==1)
    {
      sub_data_id = get_coefficient_data_id_from_coefficient_id(loads_data[loads_data_id][6]);
      coefficient_data[sub_data_id][1] = options[5];
    }
    return true;
  }
}

bool CoreLoadsFilm::delete_load(int load_id)
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
    // temperature
    sub_data_id = get_temperature_data_id_from_temperature_id(loads_data[loads_data_id][5]);
    if (sub_data_id != -1){
      temperature_data.erase(temperature_data.begin() + sub_data_id);
    }
    // coefficient
    sub_data_id = get_coefficient_data_id_from_coefficient_id(loads_data[loads_data_id][6]);
    if (sub_data_id != -1){
      coefficient_data.erase(coefficient_data.begin() + sub_data_id);
    }
    loads_data.erase(loads_data.begin() + loads_data_id);
    return true;
  }
}

bool CoreLoadsFilm::add_time_delay(std::string time_delay_id, std::string time_delay_value)
{
  std::vector<std::string> v = {time_delay_id, time_delay_value};
      
  time_delay_data.push_back(v);

  return true;
}

bool CoreLoadsFilm::add_temperature(std::string temperature_id, std::string temperature_value)
{
  std::vector<std::string> v = {temperature_id,temperature_value};
      
  temperature_data.push_back(v);

  return true;
}

bool CoreLoadsFilm::add_coefficient(std::string coefficient_id, std::string coefficient_value)
{
  std::vector<std::string> v = {coefficient_id, coefficient_value};
      
  coefficient_data.push_back(v);

  return true;
}

int CoreLoadsFilm::get_loads_data_id_from_load_id(int load_id)
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

int CoreLoadsFilm::get_time_delay_data_id_from_time_delay_id(int time_delay_id)
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

int CoreLoadsFilm::get_temperature_data_id_from_temperature_id(int temperature_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < temperature_data.size(); i++)
  {
    if (temperature_data[i][0]==std::to_string(temperature_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsFilm::get_coefficient_data_id_from_coefficient_id(int coefficient_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < coefficient_data.size(); i++)
  {
    if (coefficient_data[i][0]==std::to_string(coefficient_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

std::string CoreLoadsFilm::get_load_export(int load_id)
{
  int load_data_id;
  int sub_data_id;
  std::string str_temp = "*FILM";
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
    str_temp.append(",FILM AMPLITUDE=" + ccx_iface->get_amplitude_name(loads_data[load_data_id][2]));
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(loads_data[load_data_id][3]);
  if (time_delay_data[sub_data_id][1]!="")
  {
    str_temp.append(",FILM TIME DELAY=" + time_delay_data[sub_data_id][1]);
  }
  str_temp.append("\n");
  
  // second line
  str_temp.append(ccx_iface->get_sideset_name(loads_data[load_data_id][4]) + ",GRAV,");
  
  sub_data_id = get_temperature_data_id_from_temperature_id(loads_data[load_data_id][5]);
  str_temp.append(temperature_data[sub_data_id][1] + ",");
  
  sub_data_id = get_coefficient_data_id_from_coefficient_id(loads_data[load_data_id][6]);
  str_temp.append(coefficient_data[sub_data_id][1]);

  return str_temp;
}

std::string CoreLoadsFilm::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsFilm loads_data: \n";
  str_return.append("load_id, OP MODE, amplitude_id, time_delay_id, block_id, direction_id, magnitude_id \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " " + std::to_string(loads_data[i][1]) + " " + std::to_string(loads_data[i][2]) + " " + std::to_string(loads_data[i][3]) + " " + std::to_string(loads_data[i][4]) + " " + std::to_string(loads_data[i][5]) + " " + std::to_string(loads_data[i][6]) + " \n");
  }

  str_return.append("\n CoreLoadsFilm time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value \n");

  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(time_delay_data[i][0] + " " + time_delay_data[i][1] + " \n");
  }

  str_return.append("\n CoreLoadsFilm temperature_data: \n");
  str_return.append("temperature_id,temperature_value  \n");

  for (size_t i = 0; i < temperature_data.size(); i++)
  {
    str_return.append(temperature_data[i][0] + " " + temperature_data[i][1] + " \n");
  }
  
  str_return.append("\n CoreLoadsFilm coefficient_data: \n");
  str_return.append("coefficient_id, coefficient_value \n");

  for (size_t i = 0; i < coefficient_data.size(); i++)
  {
    str_return.append(coefficient_data[i][0] + " " + coefficient_data[i][1] + " \n");
  }

  return str_return;
}