#include "CoreLoadsBodyHeatflux.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreLoadsBodyHeatflux::CoreLoadsBodyHeatflux()
{}

CoreLoadsBodyHeatflux::~CoreLoadsBodyHeatflux()
{}

bool CoreLoadsBodyHeatflux::init()
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

bool CoreLoadsBodyHeatflux::update()
{ 
  return true;
}

bool CoreLoadsBodyHeatflux::reset()
{
  loads_data.clear();
  time_delay_data.clear();
  element_data.clear();
  magnitude_data.clear();
  name_data.clear();
  init();
  return true;
}

bool CoreLoadsBodyHeatflux::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsBodyHeatflux::create_load(std::vector<std::string> options, std::vector<int> options2)
{
  int load_id;
  int load_last;
  int sub_id;
  int sub_last;
  int op_mode;
  int amplitude_id;
  int time_delay_id;
  int element_id;
  int element_type;
  int magnitude_id;
  int name_id;
  
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
    sub_id = int(time_delay_data[sub_last][0]) + 1;
  }
  time_delay_id = sub_id;
  this->add_time_delay(sub_id, std::stod(options[2]));

  // element
  if (element_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(element_data.size()) - 1;
    sub_id = int(element_data[sub_last][0]) + 1;
  }
  element_id = sub_id;
  this->add_element(sub_id, options2);

  // element type
  element_type = std::stoi(options[3]);

  // magnitude
  if (magnitude_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(magnitude_data.size()) - 1;
    sub_id = int(magnitude_data[sub_last][0]) + 1;
  }
  magnitude_id = sub_id;
  this->add_magnitude(sub_id, std::stod(options[4]));

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
  this->add_name(std::to_string(sub_id), options[5]);

  this->add_load(load_id, op_mode, amplitude_id, time_delay_id, element_id, element_type, magnitude_id, name_id);
  return true;
}

bool CoreLoadsBodyHeatflux::add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id, int element_id, int element_type, int magnitude_id, int name_id)
{
  std::vector<int> v = {load_id, op_mode, amplitude_id, time_delay_id, element_id, element_type, magnitude_id, name_id};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsBodyHeatflux::modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2)
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
      time_delay_data[sub_data_id][1] = std::stod(options[2]);
    }
    // element
    if (options_marker[3]==1)
    {
      sub_data_id = get_element_data_id_from_element_id(loads_data[loads_data_id][4]);
      std::vector<int> e;
      element_data[sub_data_id] = e;
      element_data[sub_data_id].push_back(loads_data[loads_data_id][4]);
      for (size_t i = 0; i < options2.size(); i++)
      {
        element_data[sub_data_id].push_back(options2[i]);
      }
    }
    // element type
    if (options_marker[4]==1)
    {
      loads_data[loads_data_id][5] = std::stoi(options[3]);
    }
    // magnitude
    if (options_marker[5]==1)
    {
      sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][6]);
      magnitude_data[sub_data_id][1] = std::stod(options[4]);
    }
    // name
    if (options_marker[6]==1)
    {
      sub_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][7]);
      name_data[sub_data_id][1] = options[5];
    }
    return true;
  }
}

bool CoreLoadsBodyHeatflux::delete_load(int load_id)
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
    // element
    sub_data_id = get_element_data_id_from_element_id(loads_data[loads_data_id][4]);
    if (sub_data_id != -1){
      element_data.erase(element_data.begin() + sub_data_id);
    }
    // magnitude
    sub_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[loads_data_id][6]);
    if (sub_data_id != -1){
      magnitude_data.erase(magnitude_data.begin() + sub_data_id);
    }
    // name
    sub_data_id = get_name_data_id_from_name_id(loads_data[loads_data_id][7]);
    if (sub_data_id != -1){
      name_data.erase(name_data.begin() + sub_data_id);
    }
    loads_data.erase(loads_data.begin() + loads_data_id);
    return true;
  }
}

bool CoreLoadsBodyHeatflux::add_time_delay(double time_delay_id, double time_delay_value)
{
  std::vector<double> v = {time_delay_id, time_delay_value};
      
  time_delay_data.push_back(v);

  return true;
}

bool CoreLoadsBodyHeatflux::add_element(int element_id, std::vector<int> elements)
{
  std::vector<int> v = {element_id};
  for (size_t i = 0; i < elements.size(); i++)
  {
    v.push_back(elements[i]);
  }
      
  element_data.push_back(v);

  return true;
}

bool CoreLoadsBodyHeatflux::add_magnitude(double magnitude_id, double magnitude_value)
{
  std::vector<double> v = {magnitude_id, magnitude_value};
      
  magnitude_data.push_back(v);

  return true;
}

bool CoreLoadsBodyHeatflux::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  
  name_data.push_back(v);
  
  return true;
}

int CoreLoadsBodyHeatflux::get_loads_data_id_from_load_id(int load_id)
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

int CoreLoadsBodyHeatflux::get_time_delay_data_id_from_time_delay_id(int time_delay_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    if (time_delay_data[i][0]==double(time_delay_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsBodyHeatflux::get_element_data_id_from_element_id(int element_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < element_data.size(); i++)
  {
    if (element_data[i][0]==element_id)
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsBodyHeatflux::get_magnitude_data_id_from_magnitude_id(int magnitude_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    if (magnitude_data[i][0]==double(magnitude_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreLoadsBodyHeatflux::get_name_data_id_from_name_id(int name_id)
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

std::string CoreLoadsBodyHeatflux::get_load_export(int load_id)
{
  int load_data_id;
  int sub_data_id;
  int magnitude_data_id;

  std::string str_temp = "*DFLUX";
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
  if (time_delay_data[sub_data_id][1]==0.)
  {
    str_temp.append(",TIME DELAY=" + std::to_string(time_delay_data[sub_data_id][1]));
  }
  str_temp.append("\n");
  
  // second line
  sub_data_id = get_element_data_id_from_element_id(loads_data[load_data_id][4]);
  magnitude_data_id = get_magnitude_data_id_from_magnitude_id(loads_data[load_data_id][6]);
    
  for (size_t i = 1; i < element_data[sub_data_id].size(); i++)
  {
    if (loads_data[load_data_id][5]==1) // BLOCK
    {  
      str_temp.append(ccx_iface->get_block_name(element_data[sub_data_id][i]) + ",BF,"); 
    }else if (loads_data[load_data_id][5]==2) // ELEMENTS
    {
      str_temp.append(std::to_string(element_data[sub_data_id][i]) + ",BF,"); 
    }
    str_temp.append(ccx_iface->to_string_scientific(magnitude_data[sub_data_id][1]));
    str_temp.append("\n");  
  }
  
  return str_temp;
}

std::string CoreLoadsBodyHeatflux::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsBodyHeatflux loads_data: \n";
  str_return.append("load_id, OP MODE, amplitude_id, time_delay_id, element_id, element_type, magnitude_id, name_id \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " " + std::to_string(loads_data[i][1]) + " " + std::to_string(loads_data[i][2]) + " " + std::to_string(loads_data[i][3]) + " " + std::to_string(loads_data[i][4]) + " " + std::to_string(loads_data[i][5]) + " " + std::to_string(loads_data[i][6]) + " " + std::to_string(loads_data[i][7]) + " \n");
  }

  str_return.append("\n CoreLoadsBodyHeatflux time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value \n");

  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(std::to_string(time_delay_data[i][0]) + " " + std::to_string(time_delay_data[i][1]) + " \n");
  }

  str_return.append("\n CoreLoadsBodyHeatflux element_data: \n");
  str_return.append("element_id, element ids.... \n");

  for (size_t i = 0; i < element_data.size(); i++)
  {
    for (size_t ii = 0; ii < element_data[i].size(); ii++)
    {
      str_return.append(std::to_string(element_data[i][ii]) + " ");
    }
    str_return.append(" \n");
  }
  
  str_return.append("\n CoreLoadsBodyHeatflux magnitude_data: \n");
  str_return.append("magnitude_id, magnitude_value \n");

  for (size_t i = 0; i < magnitude_data.size(); i++)
  {
    str_return.append(std::to_string(magnitude_data[i][0]) + " " + std::to_string(magnitude_data[i][1]) + " \n");
  }

  str_return.append("\n CoreLoadsBodyHeatflux name_data: \n");
  str_return.append("name_id, name \n");

  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + " \n");
  }

  return str_return;
}