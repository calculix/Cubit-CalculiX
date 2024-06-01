#include "CoreLoadsHeatfluxes.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

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
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    is_initialized = true;  
    return true;
  }
}

bool CoreLoadsHeatfluxes::update()
{ 
  // Get the list of loads
  std::vector<int> load_ids;
  int load_id;
  int sub_id;
  int sub_last;
  int time_delay_id;
  load_ids = CubitInterface::get_bc_id_list(CI_BCTYPE_HEATFLUX);	
  
  int loads_data_id;
  bool erase_load;

  // check for new loads!

  for (size_t i = 0; i < load_ids.size(); i++)
  {
    load_id = load_ids[i];
    loads_data_id = get_loads_data_id_from_load_id(load_id);
    if (loads_data_id == -1)
    {
      // time delay
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
      this->add_time_delay(std::to_string(sub_id), "");

      add_load(load_id, 0 , -1, time_delay_id);
    }
  }

  // check if a load changes id or is removed, this means we have erase all loads in our loads_data that cannot be connected anymore

  for (size_t i = loads_data.size(); i > 0; i--)
  {
    erase_load = true;
    for (size_t ii = 0; ii < load_ids.size(); ii++)
    {
      load_id = load_ids[ii];
      if (load_id == loads_data[i-1][0])
      {
        erase_load = false;
        break;
      }
    }
    if (erase_load)
    {
      delete_load(loads_data[i-1][0]);
    }
  }

  return true;
}

bool CoreLoadsHeatfluxes::reset()
{
  loads_data.clear();
  time_delay_data.clear();
  init();
  return true;
}

bool CoreLoadsHeatfluxes::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsHeatfluxes::add_load(int load_id, int op_mode, int amplitude_id, int time_delay_id)
{
  std::vector<int> v = {load_id, op_mode, amplitude_id, time_delay_id};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsHeatfluxes::modify_load(int load_id, std::vector<std::string> options, std::vector<int> options_marker)
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
    return true;
  }
}

bool CoreLoadsHeatfluxes::delete_load(int load_id)
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
    loads_data.erase(loads_data.begin() + loads_data_id);
    return true;
  }
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


std::string CoreLoadsHeatfluxes::get_load_parameter_export(int load_id)
{
  int load_data_id;
  int sub_data_id;
  std::string str_temp = "";
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
  return str_temp;
}

std::string CoreLoadsHeatfluxes::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsHeatfluxes loads_data: \n";
  str_return.append("load_id, OP MODE, amplitude_id, time_delay_id \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " " + std::to_string(loads_data[i][1]) + " " + std::to_string(loads_data[i][2]) + " " + std::to_string(loads_data[i][3]) + " \n");
  }

  str_return.append("\n CoreLoadsHeatfluxes time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value \n");

  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(time_delay_data[i][0] + " " + time_delay_data[i][1] + " \n");
  }
  
  return str_return;
}