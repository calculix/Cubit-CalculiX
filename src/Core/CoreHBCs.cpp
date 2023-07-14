#include "CoreHBCs.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreHBCs::CoreHBCs()
{}

CoreHBCs::~CoreHBCs()
{}

bool CoreHBCs::init()
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

bool CoreHBCs::update()
{ 
  return true;
}

bool CoreHBCs::reset()
{
  bcs_data.clear();
  
  init();
  return true;
}

bool CoreHBCs::check_initialized()
{
  return is_initialized;
}



bool CoreHBCs::add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
    
  for (size_t i = 0; i < bc_ids.size(); i++)
  {
    if (get_bc_data_id(bcs_id, bc_type, bc_ids[i])==-1)
    {
      add_bc(bcs_id, bc_type, bc_ids[i]);
    }
  }
  return true;
  
}

bool CoreHBCs::remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  int bcs_data_id;
      
  for (size_t i = bc_ids.size(); i > 0; i--)
  {
    bcs_data_id = get_bc_data_id(bcs_id, bc_type, bc_ids[i-1]);
    if (bcs_data_id!=-1)
    {
      bcs_data.erase(bcs_data.begin() + bcs_data_id);
    }
  }
  return true;
  
}

bool CoreHBCs::add_bc(int bcs_id, int bc_type, int bc_id)
{
  std::vector<int> v = {bcs_id, bc_type, bc_id};
  
  bcs_data.push_back(v);
  
  return true;
}


int CoreHBCs::get_bc_data_id(int bcs_id, int bc_type,int bc_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < bcs_data.size(); i++)
  {
    if ((bcs_data[i][0]==bcs_id) && (bcs_data[i][1]==bc_type) && (bcs_data[i][2]==bc_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

std::vector<int> CoreHBCs::get_bc_data_ids_from_bcs_id(int bcs_id)
{ 
  std::vector<int> return_int;
  for (size_t i = 0; i < bcs_data.size(); i++)
  {
    if (bcs_data[i][0]==bcs_id)
    {
        return_int.push_back(i);
    }  
  }
  return return_int;
}

std::string CoreHBCs::get_hbc_export()
{
  std::vector<std::string> steps_export_list;
  std::string str_temp;

  std::string step_export;

  for (size_t i = 0; i < steps_export_list.size(); i++)
  {
    if (i==steps_export_list.size()-1)
    {
      step_export.append(steps_export_list[i]);
    }else{
      step_export.append(steps_export_list[i] + "\n");
    }
  }

  return step_export;
}

std::string CoreHBCs::print_data()
{
  std::string str_return;
  str_return = "\n CoreHBCs bcs_data: \n";
  
  str_return.append("bcs_id, bc_type, bc_id \n");

  for (size_t i = 0; i < bcs_data.size(); i++)
  {
    str_return.append(std::to_string(bcs_data[i][0]) + " " + std::to_string(bcs_data[i][1]) + " " + std::to_string(bcs_data[i][2]) + " \n");
  }

  return str_return;
}