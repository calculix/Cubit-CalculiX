#include "CoreLoadsTrajectory.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CoreLoadsTrajectoryHeatflux.hpp"
#include "CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"
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
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
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
  
  this-add_load(load_id,load_type_int,sub_id);

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
  int sub_data_id;
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
  int sub_data_id;
  std::vector<int> sub_data_ids;
  int loads_data_id = get_loads_data_id_from_load_id(load_id);
  if (loads_data_id == -1)
  {
    return false;
  } else {
    //HEATFLUX
    bool deleted = false;
    if (loads_data[loads_data_id][1]==1)
    {
      deleted = heatflux->delete_load(loads_data[loads_data_id][2]);
    }
    //BODYHEATFLUX SPHERE
    else if (loads_data[loads_data_id][1]==2)
    {
      deleted = bodyheatfluxsphere->delete_load(loads_data[loads_data_id][2]);
    }else{
     return false;
    }

    if (deleted)
    {
      loads_data.erase(loads_data.begin() + loads_data_id);
      return true;
    }else{
      return false;
    }
  }
  return false;
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
  bodyheatfluxsphere->prepare_export();
  heatflux->prepare_export();
  
  return true;
}

bool CoreLoadsTrajectory::clean_export()
{
  bodyheatfluxsphere->clean_export();
  heatflux->clean_export();
  
  return true;
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