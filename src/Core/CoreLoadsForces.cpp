#include "CoreLoadsForces.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

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
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    is_initialized = true;  
    return true;
  }
}

bool CoreLoadsForces::update()
{ 
  // Get the list of loads
  std::vector<int> load_ids;
  int load_id;
  load_ids = CubitInterface::get_bc_id_list(CI_BCTYPE_FORCE);	
  
  int loads_data_id;
  bool erase_load;

  // check for new loads and changes of the cubit element type!

  for (size_t i = 0; i < load_ids.size(); i++)
  {
    load_id = load_ids[i];
    loads_data_id = get_loads_data_id_from_load_id(load_id);
    if (loads_data_id == -1)
    {
      add_load(load_id);
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

bool CoreLoadsForces::reset()
{
  loads_data.clear();
  init();
  return true;
}

bool CoreLoadsForces::check_initialized()
{
  return is_initialized;
}

bool CoreLoadsForces::add_load(int load_id)
{
  std::vector<int> v = {load_id};
      
  loads_data.push_back(v);

  return true;
}

bool CoreLoadsForces::delete_load(int load_id)
{
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return false;
  } else {
    loads_data.erase(loads_data.begin() + loads_data_id);
    return true;
  }
}

bool CoreLoadsForces::(std::string time_delay_id, std::string time_delay_value)
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
        return_int = i;
    }  
  }
  return return_int;
}

std::string CoreLoadsForces::print_data()
{
  std::string str_return;
  str_return = "\n CoreLoadsForces loads_data: \n";
  str_return.append("load_id \n");

  for (size_t i = 0; i < loads_data.size(); i++)
  {
    str_return.append(std::to_string(loads_data[i][0]) + " \n");
  }
  
  return str_return;
}