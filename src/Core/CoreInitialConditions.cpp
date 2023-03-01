#include "CoreInitialConditions.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreInitialConditions::CoreInitialConditions()
{}

CoreInitialConditions::~CoreInitialConditions()
{}

bool CoreInitialConditions::init()
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

bool CoreInitialConditions::update()
{ 
  return true;
}

bool CoreInitialConditions::reset()
{
  initialconditions_data.clear();
  displacement_data.clear();
  temperature_data.clear();
  
  init();
  return true;
}

bool CoreInitialConditions::check_initialized()
{
  return is_initialized;
}

bool CoreInitialConditions::create_initialcondition(std::vector<std::string> options)
{
  int initialcondition_id;
  int initialcondition_last;
  int sub_id;
  int sub_last;
  int initialcondition_type;
  
  if (initialconditions_data.size()==0)
  {
    initialcondition_id = 1;
  }
  else
  {
    initialcondition_last = initialconditions_data.size() - 1;
    initialcondition_id = initialconditions_data[initialcondition_last][0] + 1;
  }
  
  // initialcondition_type
  initialcondition_type = std::stoi(options[0]);
  
  if (initialcondition_type == 1)
  {  // displacement
    if (displacement_data.size()==0)
    {
      sub_id = 1;
    }
    else
    {
      sub_last = displacement_data.size() - 1;
      sub_id = std::stoi(displacement_data[sub_last][0]) + 1;
    }
    this->add_displacement(std::to_string(sub_id));
  }else if (initialcondition_type == 2)
  { // temperature
    if (temperature_data.size()==0)
    {
      sub_id = 1;
    }
    else
    {
      sub_last = temperature_data.size() - 1;
      sub_id = std::stoi(temperature_data[sub_last][0]) + 1;
    }
    this->add_temperature(std::to_string(sub_id));
  }
  
  this->add_initialcondition(initialcondition_id, initialcondition_type, sub_id);
  return true;
}

bool CoreInitialConditions::modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  int sub_data_id;
  int initialconditions_data_id = get_initialconditions_data_id_from_initialcondition_id(initialcondition_id);

  if (initialconditions_data_id == -1)
  {
    return false;
  } else {
    // displacement
    if ((initialconditions_data[initialconditions_data_id][1]==1) && (initialconditions_data[initialconditions_data_id][1]==modify_type))
    {
      sub_data_id = get_displacement_data_id_from_displacement_id(initialconditions_data[initialconditions_data_id][2]);
      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          displacement_data[sub_data_id][i+1] = options[i];
        }
      }
    } else if ((initialconditions_data[initialconditions_data_id][1]==2) && (initialconditions_data[initialconditions_data_id][1]==modify_type))
    {
      sub_data_id = get_temperature_data_id_from_temperature_id(initialconditions_data[initialconditions_data_id][2]);
      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          temperature_data[sub_data_id][i+1] = options[i];
        }
      }
    }
    return true;
  }
}

bool CoreInitialConditions::add_initialcondition(int initialcondition_id, int initialcondition_type, int initialcondition_type_id)
{
  std::vector<int> v = {initialcondition_id, initialcondition_type, initialcondition_type_id};
      
  initialconditions_data.push_back(v);

  return true;
}

bool CoreInitialConditions::add_displacement(std::string displacement_id)
{
  std::vector<std::string> v = {displacement_id,""};
  
  displacement_data.push_back(v);
  
  return true;
}

bool CoreInitialConditions::add_temperature(std::string temperature_id)
{
  std::vector<std::string> v = {temperature_id,""};
  
  temperature_data.push_back(v);
  
  return true;
}

bool CoreInitialConditions::delete_initialcondition(int initialcondition_id)
{
  int sub_data_id;
  int initialconditions_data_id = get_initialconditions_data_id_from_initialcondition_id(initialcondition_id);
  if (initialconditions_data_id == -1)
  {
    return false;
  } else {
    if (initialconditions_data[initialconditions_data_id][1]==1)
    {
      sub_data_id = get_displacement_data_id_from_displacement_id(initialconditions_data[initialconditions_data_id][2]);
      if (sub_data_id != -1){
        displacement_data.erase(displacement_data.begin() + sub_data_id);  
      }
    }else if (initialconditions_data[initialconditions_data_id][1]==2)
    {
      sub_data_id = get_temperature_data_id_from_temperature_id(initialconditions_data[initialconditions_data_id][2]);
      if (sub_data_id != -1){
        temperature_data.erase(temperature_data.begin() + sub_data_id);  
      }
    }
    initialconditions_data.erase(initialconditions_data.begin() + initialconditions_data_id);
    return true;
  }
}

int CoreInitialConditions::get_initialconditions_data_id_from_initialcondition_id(int initialcondition_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < initialconditions_data.size(); i++)
  {
    if (initialconditions_data[i][0]==initialcondition_id)
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreInitialConditions::get_displacement_data_id_from_displacement_id(int displacement_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < displacement_data.size(); i++)
  {
    if (displacement_data[i][0]==std::to_string(displacement_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreInitialConditions::get_temperature_data_id_from_temperature_id(int temperature_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < temperature_data.size(); i++)
  {
    if (temperature_data[i][0]==std::to_string(temperature_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

std::string CoreInitialConditions::get_initialcondition_export() // get a list of the CalculiX amplitude exports
{
  std::vector<std::string> initialconditions_export_list;
  initialconditions_export_list.push_back("********************************** I N I T I A L C O N D I T I O N S ****************************");
  std::string str_temp;
  int sub_data_id;
  std::vector<int> sub_data_ids;
  /*
  //loop over all amplitudes
  for (size_t i = 0; i < amplitudes_data.size(); i++)
  { 
    str_temp = "*AMPLITUDE, NAME=";
    sub_data_id = get_name_amplitude_data_id_from_name_amplitude_id(amplitudes_data[i][1]);
    str_temp.append(name_amplitude_data[sub_data_id][1]);
    //shiftx
    sub_data_id = get_shiftx_amplitude_data_id_from_shiftx_amplitude_id(amplitudes_data[i][2]);
    if(shiftx_amplitude_data[sub_data_id][1]!="")
    {
      str_temp.append(", SHIFTX=");
      str_temp.append(shiftx_amplitude_data[sub_data_id][1]);
    }
    //shifty
    sub_data_id = get_shifty_amplitude_data_id_from_shifty_amplitude_id(amplitudes_data[i][3]);
    if(shifty_amplitude_data[sub_data_id][1]!="")
    {
      str_temp.append(", SHIFTY=");
      str_temp.append(shifty_amplitude_data[sub_data_id][1]);
    }
    // time_type
    if (amplitudes_data[i][4]==1)
    {
      str_temp.append(", TIME=TOTAL TIME"); 
    }
    
    amplitudes_export_list.push_back(str_temp);

    // second line
    // time_amplitude

    sub_data_ids = get_amplitudevalues_amplitude_data_ids_from_amplitudevalues_amplitude_id(amplitudes_data[i][5]);
    // second lines
    str_temp = "";
    int ii = 0;
    for (size_t i = 0; i < sub_data_ids.size(); i++)
    {
      if ((i != 0) && (ii!=0))
      {
        str_temp.append(",");
      }
      ii = ii + 1;
      
      str_temp.append(amplitudevalues_amplitude_data[sub_data_ids[i]][1]);
      str_temp.append(",");
      str_temp.append(amplitudevalues_amplitude_data[sub_data_ids[i]][2]);
      if (ii == 4)
      {
        ii = 0;
        str_temp.append(",");
        amplitudes_export_list.push_back(str_temp);
        str_temp = "";
      }
    }
    amplitudes_export_list.push_back(str_temp);
  }
*/  
  std::string initialcondition_export;

  for (size_t i = 0; i < initialconditions_export_list.size(); i++)
  {
    initialcondition_export.append(initialconditions_export_list[i] + "\n");
  }

  return initialcondition_export;
}

std::string CoreInitialConditions::print_data()
{
  std::string str_return;
  str_return = "\n CoreInitialConditions initialconditions_data: \n";
  str_return.append("initialcondition_id, initialcondition_type, initialcondition_type_id \n");

  for (size_t i = 0; i < initialconditions_data.size(); i++)
  {
    str_return.append(std::to_string(initialconditions_data[i][0]) + " " + std::to_string(initialconditions_data[i][1]) + " " + std::to_string(initialconditions_data[i][2]) + " \n");
  }

  str_return.append("\n CoreInitialConditions displacement_data: \n");
  str_return.append("displacement_id, bc_displacement_id \n");

  for (size_t i = 0; i < displacement_data.size(); i++)
  {
    str_return.append(displacement_data[i][0] + " " + displacement_data[i][1] + " \n");
  }
  
  str_return.append("\n CoreInitialConditions temperature_data: \n");
  str_return.append("temperature_id, bc_temperature_id \n");

  for (size_t i = 0; i < temperature_data.size(); i++)
  {
    str_return.append(temperature_data[i][0] + " " + temperature_data[i][1] + " \n");
  }

  return str_return;
}