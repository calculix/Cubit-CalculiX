#include "CoreBCsTemperatures.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreBCsTemperatures::CoreBCsTemperatures()
{}

CoreBCsTemperatures::~CoreBCsTemperatures()
{}

bool CoreBCsTemperatures::init()
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

bool CoreBCsTemperatures::update()
{ 
  // Get the list of bcs
  std::vector<int> bc_ids;
  int bc_id;
  int sub_id;
  int sub_last;
  int time_delay_id;
  bc_ids = CubitInterface::get_bc_id_list(CI_BCTYPE_TEMPERATURE);	
  
  int bcs_data_id;
  bool erase_bc;

  // check for new bcs!

  for (size_t i = 0; i < bc_ids.size(); i++)
  {
    bc_id = bc_ids[i];
    bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
    if (bcs_data_id == -1)
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

      add_bc(bc_id, 0 , -1, time_delay_id, 0);
    }
  }

  // check if a bc changes id or is removed, this means we have erase all bcs in our bcs_data that cannot be connected anymore

  for (size_t i = bcs_data.size(); i > 0; i--)
  {
    erase_bc = true;
    for (size_t ii = 0; ii < bc_ids.size(); ii++)
    {
      bc_id = bc_ids[ii];
      if (bc_id == bcs_data[i-1][0])
      {
        erase_bc = false;
        break;
      }
    }
    if (erase_bc)
    {
      delete_bc(bcs_data[i-1][0]);
    }
  }

  return true;
}

bool CoreBCsTemperatures::reset()
{
  bcs_data.clear();
  time_delay_data.clear();
  init();
  return true;
}

bool CoreBCsTemperatures::check_initialized()
{
  return is_initialized;
}

bool CoreBCsTemperatures::add_bc(int bc_id, int op_mode, int amplitude_id, int time_delay_id, int keyword_type)
{
  std::vector<int> v = {bc_id, op_mode, amplitude_id, time_delay_id, keyword_type};
      
  bcs_data.push_back(v);

  return true;
}

bool CoreBCsTemperatures::modify_bc(int bc_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  int sub_data_id;
  std::vector<int> sub_data_ids;
  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  
  if (bcs_data_id == -1)
  {
    return false;
  } else {
    // OP MODE
    if (options_marker[0]==1)
    {
      bcs_data[bcs_data_id][1] = std::stoi(options[0]);
    }
    // AMPLITUDE
    if (options_marker[1]==1)
    {
      bcs_data[bcs_data_id][2] = std::stoi(options[1]);
    }
    // time delay
    if (options_marker[2]==1)
    {
      sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bcs_data_id][3]);
      time_delay_data[sub_data_id][1] = options[2];
    }
    // keyword_type
    if (options_marker[3]==1)
    {
      bcs_data[bcs_data_id][4] = std::stoi(options[3]);
    }
    return true;
  }
}

bool CoreBCsTemperatures::delete_bc(int bc_id)
{
  int sub_data_id;
  int bcs_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data_id == -1)
  {
    return false;
  } else {
    // time delay
    sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bcs_data_id][3]);
    if (sub_data_id != -1){
      time_delay_data.erase(time_delay_data.begin() + sub_data_id);
    }
    bcs_data.erase(bcs_data.begin() + bcs_data_id);
    return true;
  }
}

bool CoreBCsTemperatures::add_time_delay(std::string time_delay_id, std::string time_delay_value)
{
  std::vector<std::string> v = {time_delay_id, time_delay_value};
      
  time_delay_data.push_back(v);

  return true;
}

int CoreBCsTemperatures::get_bcs_data_id_from_bc_id(int bc_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < bcs_data.size(); i++)
  {
    if (bcs_data[i][0]==bc_id)
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreBCsTemperatures::get_time_delay_data_id_from_time_delay_id(int time_delay_id)
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

std::string CoreBCsTemperatures::get_bc_parameter_export(int bc_id)
{
  int bc_data_id;
  int sub_data_id;
  std::string str_temp = "";
  bc_data_id = get_bcs_data_id_from_bc_id(bc_id);
  if (bcs_data[bc_data_id][1]==0)
  {
    //str_temp.append(",OP=MOD");
  }else if (bcs_data[bc_data_id][1]==1)
  {
    str_temp.append(",OP=NEW");
  }
  if (bcs_data[bc_data_id][2]!=-1)
  {
    str_temp.append(",AMPLITUDE=" + ccx_iface->get_amplitude_name(bcs_data[bc_data_id][2]));
  }
  sub_data_id = get_time_delay_data_id_from_time_delay_id(bcs_data[bc_data_id][3]);
  if (time_delay_data[sub_data_id][1]!="")
  {
    str_temp.append(",TIME DELAY=" + time_delay_data[sub_data_id][1]);
  }
  return str_temp;
}

int CoreBCsTemperatures::get_bc_keyword_type(int bc_id)
{
  int bc_data_id;
  bc_data_id = get_bcs_data_id_from_bc_id(bc_id);
  return bcs_data[bc_data_id][4];
}

std::string CoreBCsTemperatures::print_data()
{
  std::string str_return;
  str_return = "\n CoreBCsTemperatures bcs_data: \n";
  str_return.append("bc_id, OP MODE, amplitude_id, time_delay_id, keyword_type \n");

  for (size_t i = 0; i < bcs_data.size(); i++)
  {
    str_return.append(std::to_string(bcs_data[i][0]) + " " + std::to_string(bcs_data[i][1]) + " " + std::to_string(bcs_data[i][2]) + " " + std::to_string(bcs_data[i][3]) + " " + std::to_string(bcs_data[i][4]) + " \n");
  }

  str_return.append("\n CoreBCsTemperatures time_delay_data: \n");
  str_return.append("time_delay_id, time_delay_value \n");

  for (size_t i = 0; i < time_delay_data.size(); i++)
  {
    str_return.append(time_delay_data[i][0] + " " + time_delay_data[i][1] + " \n");
  }
  
  return str_return;
}