#include "CoreLoadsRadiation.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreLoadsRadiation::CoreLoadsRadiation()
{}

CoreLoadsRadiation::~CoreLoadsRadiation()
{}

bool CoreLoadsRadiation::init()
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

bool CoreLoadsRadiation::update()
{ 
  return true;
}

bool CoreLoadsRadiation::reset()
{
  loads_data.clear();
  time_delay_data.clear();
  radiation_time_delay_data.clear();
  temperature_data.clear();
  emissivity_data.clear();
  name_data.clear();
  cavity_data.clear();
  init();
  return true;
}

bool CoreLoadsRadiation::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsRadiation::create_load(std::vector<std::string> options)
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
  int emissivity_id;
  int radiation_amplitude_id;
  int radiation_time_delay_id;
  int name_id;
  int cavity_id;
  int cavityradiation;
  
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

  // emissivity
  if (emissivity_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(emissivity_data.size()) - 1;
    sub_id = std::stoi(emissivity_data[sub_last][0]) + 1;
  }
  emissivity_id = sub_id;
  this->add_emissivity(std::to_string(sub_id), options[5]);

  // RADIATION AMPLITUDE
  radiation_amplitude_id = std::stoi(options[6]);

  // radiation time_delay
  if (radiation_time_delay_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(radiation_time_delay_data.size()) - 1;
    sub_id = std::stoi(radiation_time_delay_data[sub_last][0]) + 1;
  }
  radiation_time_delay_id = sub_id;
  this->add_radiation_time_delay(std::to_string(sub_id), options[7]);

  // name
  if (name_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(name_data.size()) - 1;
    sub_id = std::stoi(name_data[sub_last][0]) + 1;
  }
  name_id = sub_id;
  this->add_name(std::to_string(sub_id), options[8]);

  // cavity
  if (cavity_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(cavity_data.size()) - 1;
    sub_id = std::stoi(cavity_data[sub_last][0]) + 1;
  }
  cavity_id = sub_id;
  this->add_cavity(std::to_string(sub_id), options[9]);

  // CAVITY RADIATION
  cavityradiation = std::stoi(options[10]);

  this->add_load(load_id, op_mode, amplitude_id, time_delay_id, sideset_id, temperature_id, emissivity_id,radiation_amplitude_id, radiation_time_delay_id, name_id, cavity_id, cavityradiation);
  return true;
}

bool CoreLoadsRadiation::add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id, int sideset_id, int temperature_id, int emissivity_id,int radiation_amplitude_id, int radiation_time_delay_id, int name_id, int cavity_id, int cavityradiation)
{
  std::vector<int> v = {load_id, op_mode, amplitude_id, time_delay_id, sideset_id, temperature_id, emissivity_id, radiation_amplitude_id, radiation_time_delay_id, name_id, cavity_id, cavityradiation};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsRadiation::modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker)
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
    // emissivity
    if (options_marker[5]==1)
    {
      sub_data_id = get_emissivity_data_id_from_emissivity_id(loads_data[loads_data_id][6]);
      emissivity_data[sub_data_id][1] = options[5];
    }
    // RADIATION AMPLITUDE
    if (options_marker[6]==1)
    {
      loads_data[loads_data_id][7] = std::stoi(options[6]);
    }
    // radiation time delay
    if (options_marker[7]==1)
    {
      sub_data_id = get_radiation_time_delay_data_id_from_radiation_time_delay_id(loads_data[loads_data_id][8]);
      radiation_time_delay_data[sub_data_id][1] = options[7];
    }
    // name
    if (options_marker[8]==1)
    {
      sub_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][9]);
      name_data[sub_data_id][1] = options[8];
    }
    // cavity
    if (options_marker[9]==1)
    {
      sub_data_id = get_cavity_data_id_from_cavity_id(loads_data[loads_data_id][10]);
      cavity_data[sub_data_id][1] = options[9];
    }
    // Cavity Radiation
    if (options_marker[10]==1)
    {
      loads_data[loads_data_id][11] = std::stoi(options[10]);
    }
    return true;
  }
}

bool CoreLoadsRadiation::delete_load(int load_id)
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
    // emissivity
    sub_data_id = get_emissivity_data_id_from_emissivity_id(loads_data[loads_data_id][6]);
    if (sub_data_id != -1){
      emissivity_data.erase(emissivity_data.begin() + sub_data_id);
    }
    // radiation time delay
    sub_data_id = get_radiation_time_delay_data_id_from_radiation_time_delay_id(loads_data[loads_data_id][8]);
    if (sub_data_id != -1){
      radiation_time_delay_data.erase(radiation_time_delay_data.begin() + sub_data_id);
    }
    // name
    sub_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][9]);
    if (sub_data_id != -1){
      name_data.erase(name_data.begin() + sub_data_id);
    }
    // cavity
    sub_data_id = get_cavity_data_id_from_cavity_id(loads_data[loads_data_id][10]);
    if (sub_data_id != -1){
      cavity_data.erase(cavity_data.begin() + sub_data_id);
    }

    loads_data.erase(loads_data.begin() + loads_data_id);
    return true;
  }
}

bool CoreLoadsRadiation::add_time_delay(std::string time_delay_id, std::string time_delay_value)
{
  std::vector<std::string> v = {time_delay_id, time_delay_value};
      
  time_delay_data.push_back(v);

  return true;
}

bool CoreLoadsRadiation::add_temperature(std::string temperature_id, std::string temperature_value)
{
  std::vector<std::string> v = {temperature_id,temperature_value};
      
  temperature_data.push_back(v);

  return true;
}

bool CoreLoadsRadiation::add_emissivity(std::string emissivity_id, std::string emissivity_value)
{
  std::vector<std::string> v = {emissivity_id, emissivity_value};
      
  emissivity_data.push_back(v);

  return true;
}

bool CoreLoadsRadiation::add_radiation_time_delay(std::string radiation_time_delay_id, std::string radiation_time_delay_value)
{
  std::vector<std::string> v = {radiation_time_delay_id, radiation_time_delay_value};
      
  radiation_time_delay_data.push_back(v);

  return true;
}

bool CoreLoadsRadiation::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  
  name_data.push_back(v);
  
  return true;
}

bool CoreLoadsRadiation::add_cavity(std::string cavity_id, std::string cavity)
{
  std::vector<std::string> v = {cavity_id, cavity};
  
  cavity_data.push_back(v);
  
  return true;
}

int CoreLoadsRadiation::get_loads_data_id_from_load_id(int load_id)
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

int CoreLoadsRadiation::get_time_delay_data_id_from_time_delay_id(int time_delay_id)
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

int CoreLoadsRadiation::get_temperature_data_id_from_temperature_id(int temperature_id)
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

int CoreLoadsRadiation::get_emissivity_data_id_from_emissivity_id(int emissivity_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < emissivity_data.size(); i++)
  {
    if (emissivity_data[i][0]==std::to_string(emissivity_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsRadiation::get_radiation_time_delay_data_id_from_radiation_time_delay_id(int radiation_time_delay_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < radiation_time_delay_data.size(); i++)
  {
    if (radiation_time_delay_data[i][0]==std::to_string(radiation_time_delay_id))
    {
      return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsRadiation::get_name_data_id_from_name_id(int name_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < name_data.size(); i++)
  {
    if (name_data[i][0]==std::to_string(name_id))
    {
      return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsRadiation::get_cavity_data_id_from_cavity_id(int cavity_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < cavity_data.size(); i++)
  {
    if (cavity_data[i][0]==std::to_string(cavity_id))
    {
      return_int = int(i);
    }  
  }
  return return_int;
}

std::string CoreLoadsRadiation::get_load_export(int load_id)
{
  int load_data_id;
  int sub_data_id;
  std::string str_temp = "*RADIATE";
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
  if (loads_data[load_data_id][7]!=-1)
  {
    str_temp.append(",RADIATION AMPLITUDE=" + ccx_iface->get_amplitude_name(loads_data[load_data_id][7]));
  }
  sub_data_id = get_radiation_time_delay_data_id_from_radiation_time_delay_id(loads_data[load_data_id][8]);
  if (radiation_time_delay_data[sub_data_id][1]!="")
  {
    str_temp.append(",RADIATION TIME DELAY=" + radiation_time_delay_data[sub_data_id][1]);
  }
  sub_data_id = get_cavity_data_id_from_cavity_id(loads_data[load_data_id][10]);
  if (cavity_data[sub_data_id][1]!="")
  {
    str_temp.append(",CAVITY=" + cavity_data[sub_data_id][1]);
  }
  //str_temp.append("\n");
  

  return str_temp;
}

std::string CoreLoadsRadiation::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsRadiation loads_data: \n";
  str_return.append("load_id, OP MODE, amplitude_id, time_delay_id, sideset_id, direction_id, magnitude_id,radiation_amplitude_id, radiation_time_delay_id,name, cavity_id, cavity_radiation \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " " + std::to_string(loads_data[i][1]) + " " + std::to_string(loads_data[i][2]) + " " + std::to_string(loads_data[i][3]) + " " + std::to_string(loads_data[i][4]) + " " + std::to_string(loads_data[i][5]) + " " + std::to_string(loads_data[i][6])  + " " + std::to_string(loads_data[i][7])  + " " + std::to_string(loads_data[i][8])  + " " + std::to_string(loads_data[i][9])  + " " + std::to_string(loads_data[i][10])  + " " + std::to_string(loads_data[i][11]) + " \n");
  }

  str_return.append("\n CoreLoadsRadiation time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value \n");

  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(time_delay_data[i][0] + " " + time_delay_data[i][1] + " \n");
  }

  str_return.append("\n CoreLoadsRadiation temperature_data: \n");
  str_return.append("temperature_id,temperature_value  \n");

  for (size_t i = 0; i < temperature_data.size(); i++)
  {
    str_return.append(temperature_data[i][0] + " " + temperature_data[i][1] + " \n");
  }
  
  str_return.append("\n CoreLoadsRadiation emissivity_data: \n");
  str_return.append("emissivity_id, emissivity_value \n");

  for (size_t i = 0; i < emissivity_data.size(); i++)
  {
    str_return.append(emissivity_data[i][0] + " " + emissivity_data[i][1] + " \n");
  }

  str_return.append("\n CoreLoadsRadiation radiation_time_delay_data: \n");
  str_return.append("radiation_time_delay_id, radiation_time_delay_value \n");

  for (size_t i = 0; i < radiation_time_delay_data.size(); i++)
  {
    str_return.append(radiation_time_delay_data[i][0] + " " + radiation_time_delay_data[i][1] + " \n");
  }

  str_return.append("\n CoreLoadsRadiation name_data: \n");
  str_return.append("name_id, name \n");

  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + " \n");
  }

  return str_return;
}