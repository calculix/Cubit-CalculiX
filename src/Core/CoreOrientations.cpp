#include "CoreOrientations.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreOrientations::CoreOrientations()
{}

CoreOrientations::~CoreOrientations()
{}

bool CoreOrientations::init()
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

bool CoreOrientations::update()
{ 
  return true;
}

bool CoreOrientations::reset()
{
  orientations_data.clear();
  name_data.clear();
  a_data.clear();
  b_data.clear();
  rotation_data.clear();

  init();
  return true;
}

bool CoreOrientations::check_initialized()
{
  return is_initialized;
}

bool CoreOrientations::create_orientation(std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  int orientation_id;
  int orientation_last;
  int sub_id;
  int sub_last;
  int name_id;
  int system_type;
  int distribution_id;
  int a_id;
  int b_id;
  int local_axis;
  int rotation_id;
  
  if (orientations_data.size()==0)
  {
    orientation_id = 1;
  }
  else
  {
    orientation_last = orientations_data.size() - 1;
    orientation_id = orientations_data[orientation_last][0] + 1;
  }

  // name
  if (name_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = name_data.size() - 1;
    sub_id = std::stoi(name_data[sub_last][0]) + 1;
  }
  name_id = sub_id;
  this->add_name(std::to_string(sub_id), options[0]);

  system_type = std::stoi(options[1]);
  distribution_id = std::stoi(options[2]);

  //a
  if (a_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_id = 1;
    for (size_t i = 0; i < a_data.size(); i++)
    {
      sub_last = std::stoi(a_data[i][0]);
      if (sub_id < sub_last)
      {
        sub_id = sub_last;
      }
    }
    sub_id = sub_id + 1;
  }
  a_id = sub_id;
  this->add_a(std::to_string(a_id), options2[3][0], options2[3][1], options2[3][2]);
  
  //b
  if (b_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_id = 1;
    for (size_t i = 0; i < b_data.size(); i++)
    {
      sub_last = std::stoi(b_data[i][0]);
      if (sub_id < sub_last)
      {
        sub_id = sub_last;
      }
    }
    sub_id = sub_id + 1;
  }
  b_id = sub_id;
  this->add_b(std::to_string(b_id), options2[4][0], options2[4][1], options2[4][2]);

  local_axis = std::stoi(options[5]);

  //rotation_id
  if (rotation_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_id = 1;
    for (size_t i = 0; i < rotation_data.size(); i++)
    {
      sub_last = std::stoi(rotation_data[i][0]);
      if (sub_id < sub_last)
      {
        sub_id = sub_last;
      }
    }
    sub_id = sub_id + 1;
  }
  rotation_id = sub_id;
  this->add_rotation(std::to_string(rotation_id), options2[5][0]);

  this->add_orientation(orientation_id, name_id, system_type, distribution_id, a_id, b_id, local_axis, rotation_id);
  return true;
}

bool CoreOrientations::modify_orientation(int orientation_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  int sub_data_id;
  int orientations_data_id = get_orientations_data_id_from_orientation_id(orientation_id);
  
  if (orientations_data_id == -1)
  {
    return false;
  } else {
    // name
    if (options_marker[0]==1)
    {
      sub_data_id = get_name_data_id_from_name_id(orientations_data[orientations_data_id][1]);
      name_data[sub_data_id][1] = options[0];
    }
    // system_type
    if (options_marker[1]==1)
    {
      orientations_data[orientations_data_id][2] = std::stoi(options[1]);
    }
    // distribution_id
    if (options_marker[2]==1)
    {
      orientations_data[orientations_data_id][3] = std::stoi(options[2]);
    }
    // a values
    sub_data_id = get_a_data_id_from_a_id(orientations_data[orientations_data_id][4]);
    if (options_marker[3]==1)
    {
      a_data[sub_data_id][1] = options2[3][0];
      a_data[sub_data_id][2] = options2[3][1];
      a_data[sub_data_id][3] = options2[3][2];
    }
    // b values
    sub_data_id = get_b_data_id_from_b_id(orientations_data[orientations_data_id][5]);
    if (options_marker[4]==1)
    {
      b_data[sub_data_id][1] = options2[4][0];
      b_data[sub_data_id][2] = options2[4][1];
      b_data[sub_data_id][3] = options2[4][2];
    }
    // local_axis
    if (options_marker[5]==1)
    {
      orientations_data[orientations_data_id][6] = std::stoi(options[5]);
    }
    // rotation angle
    sub_data_id = get_rotation_data_id_from_rotation_id(orientations_data[orientations_data_id][7]);
    if (options_marker[6]==1)
    {
      rotation_data[sub_data_id][1] = options2[6][0];
    }

    return true;
  }
}

bool CoreOrientations::add_orientation(int orientation_id, int name_id, int system_type, int distribution_id, int a_id, int b_id, int local_axis, int rotation_id)
{
  std::vector<int> v = {orientation_id, name_id, system_type, distribution_id, a_id, b_id, local_axis, rotation_id};
      
  orientations_data.push_back(v);

  return true;
}

bool CoreOrientations::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  
  name_data.push_back(v);
  
  return true;
}

bool CoreOrientations::add_a(std::string a_id, std::string x, std::string y, std::string z)
{
  std::vector<std::string> v = {a_id, x, y, z};
  
  a_data.push_back(v);
  
  return true;
}

bool CoreOrientations::add_b(std::string b_id, std::string x, std::string y, std::string z)
{
  std::vector<std::string> v = {b_id, x, y, z};
  
  b_data.push_back(v);
  
  return true;
}

bool CoreOrientations::add_rotation(std::string rotation_id, std::string angle)
{
  std::vector<std::string> v = {rotation_id, angle};
  
  rotation_data.push_back(v);
  
  return true;
}

bool CoreOrientations::delete_orientation(int orientation_id)
{
  int sub_data_id;
  std::vector<int> sub_data_ids;
  int orientations_data_id = get_orientations_data_id_from_orientation_id(orientation_id);
  if (orientations_data_id == -1)
  {
    return false;
  } else {
    sub_data_id = get_name_data_id_from_name_id(orientations_data[orientations_data_id][1]);
    if (sub_data_id != -1){
      name_data.erase(name_data.begin() + sub_data_id);  
    }
    sub_data_id = get_a_data_id_from_a_id(orientations_data[orientations_data_id][4]);
    if (sub_data_id != -1){
      a_data.erase(a_data.begin() + sub_data_id);  
    }
    sub_data_id = get_b_data_id_from_b_id(orientations_data[orientations_data_id][5]);
    if (sub_data_id != -1){
      b_data.erase(b_data.begin() + sub_data_id);  
    }
    sub_data_id = get_rotation_data_id_from_rotation_id(orientations_data[orientations_data_id][7]);
    if (sub_data_id != -1){
      rotation_data.erase(rotation_data.begin() + sub_data_id);  
    }
    orientations_data.erase(orientations_data.begin() + orientations_data_id);
    return true;
  }
}

int CoreOrientations::get_orientations_data_id_from_orientation_id(int orientation_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < orientations_data.size(); i++)
  {
    if (orientations_data[i][0]==orientation_id)
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreOrientations::get_name_data_id_from_name_id(int name_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < name_data.size(); i++)
  {
    if (name_data[i][0]==std::to_string(name_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreOrientations::get_a_data_id_from_a_id(int a_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < a_data.size(); i++)
  {
    if (a_data[i][0]==std::to_string(a_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreOrientations::get_b_data_id_from_b_id(int b_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < b_data.size(); i++)
  {
    if (b_data[i][0]==std::to_string(b_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreOrientations::get_rotation_data_id_from_rotation_id(int rotation_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < rotation_data.size(); i++)
  {
    if (rotation_data[i][0]==std::to_string(rotation_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

std::string CoreOrientations::get_orientation_export() // get a list of the CalculiX orienation exports
{
  std::vector<std::string> amplitudes_export_list;
  /*
  amplitudes_export_list.push_back("********************************** A M P L I T U D E S ****************************");
  std::string str_temp;
  int sub_data_id;
  std::vector<int> sub_data_ids;
  
  //loop over all amplitudes
  for (size_t i = 0; i < amplitudes_data.size(); i++)
  { 
    // CUSTOMLINE START
    std::vector<std::string> customline = ccx_iface->get_customline_data("BEFORE","AMPLITUDE",amplitudes_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      amplitudes_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END

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

    // CUSTOMLINE START
    customline = ccx_iface->get_customline_data("AFTER","AMPLITUDE",amplitudes_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      amplitudes_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END
  }
  
  std::string amplitude_export;

  for (size_t i = 0; i < amplitudes_export_list.size(); i++)
  {
    amplitude_export.append(amplitudes_export_list[i] + "\n");
  }
  */
  return "**Orienations Export";
}

std::string CoreOrientations::print_data()
{
  std::string str_return;
  str_return = "\n CoreOrientations orientations_data: \n";
  str_return.append("orientation_id, name_id, system_type, distribution_id, a_id, b_id, local_axis, rotation_id \n");

  for (size_t i = 0; i < orientations_data.size(); i++)
  {
    str_return.append(std::to_string(orientations_data[i][0]) + " " + std::to_string(orientations_data[i][1]) + " " + std::to_string(orientations_data[i][2]) + " " + std::to_string(orientations_data[i][3]) + " " + std::to_string(orientations_data[i][4]) + " " + std::to_string(orientations_data[i][5]) + " " + std::to_string(orientations_data[i][6]) + " " + std::to_string(orientations_data[i][7]) + " \n");
  }

  str_return.append("\n CoreOrientations name_data: \n");
  str_return.append("name_id, name \n");

  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + " \n");
  }

  str_return.append("\n CoreOrientations a_data: \n");
  str_return.append("a_id, x, y, z \n");

  for (size_t i = 0; i < a_data.size(); i++)
  {
    str_return.append(a_data[i][0] + " " + a_data[i][1] + " " + a_data[i][2] + " " + a_data[i][3] + " \n");
  }

  str_return.append("\n CoreOrientations b_data: \n");
  str_return.append("b_id, x, y, z \n");

  for (size_t i = 0; i < b_data.size(); i++)
  {
    str_return.append(b_data[i][0] + " " + b_data[i][1] + " " + b_data[i][2] + " " + b_data[i][3] + " \n");
  }

  str_return.append("\n CoreOrientations rotation_data: \n");
  str_return.append("rotation_id, angle \n");

  for (size_t i = 0; i < rotation_data.size(); i++)
  {
    str_return.append(rotation_data[i][0] + " " + rotation_data[i][1] + " \n");
  }

  return str_return;
}