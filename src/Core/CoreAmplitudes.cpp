#include "CoreAmplitudes.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"
#include <algorithm>

CoreAmplitudes::CoreAmplitudes()
{}

CoreAmplitudes::~CoreAmplitudes()
{}

bool CoreAmplitudes::init()
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

bool CoreAmplitudes::update()
{ 
  return true;
}

bool CoreAmplitudes::reset()
{
  amplitudes_data.clear();
  name_amplitude_data.clear();
  shiftx_amplitude_data.clear();
  shifty_amplitude_data.clear();
  amplitudevalues_amplitude_data.clear();
  
  init();
  return true;
}

bool CoreAmplitudes::check_initialized()
{
  return is_initialized;
}

bool CoreAmplitudes::create_amplitude(std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  int amplitude_id;
  int amplitude_last;
  int sub_id;
  int sub_last;
  int name_id;
  int shiftx_id;
  int shifty_id;
  int time_type;
  int amplitudevalues_id;
  
  if (amplitudes_data.size()==0)
  {
    amplitude_id = 1;
  }
  else
  {
    amplitude_last = int(amplitudes_data.size()) - 1;
    amplitude_id = amplitudes_data[amplitude_last][0] + 1;
  }

  // name
  if (name_amplitude_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(name_amplitude_data.size()) - 1;
    sub_id = std::stoi(name_amplitude_data[sub_last][0]) + 1;
  }
  name_id = sub_id;
  this->add_name_amplitude(std::to_string(sub_id), options[0]);

  // shiftx
  if (shiftx_amplitude_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(shiftx_amplitude_data.size()) - 1;
    sub_id = std::stoi(shiftx_amplitude_data[sub_last][0]) + 1;
  }
  shiftx_id = sub_id;
  this->add_shiftx_amplitude(std::to_string(sub_id), options[1]);
  
  // shifty
  if (shifty_amplitude_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = int(shifty_amplitude_data.size()) - 1;
    sub_id = std::stoi(shifty_amplitude_data[sub_last][0]) + 1;
  }
  shifty_id = sub_id;
  this->add_shifty_amplitude(std::to_string(sub_id), options[2]);
  
  time_type = std::stoi(options[3]);
  
  if (amplitudevalues_amplitude_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_id = 1;
    for (size_t i = 0; i < amplitudevalues_amplitude_data.size(); i++)
    {
      sub_last = int(amplitudevalues_amplitude_data[i][0]);
      if (sub_id < sub_last)
      {
        sub_id = sub_last;
      }
    }
    sub_id = sub_id + 1;
  }
  amplitudevalues_id = sub_id;
  for (size_t i = 0; i < options2.size(); i++)
  {
    this->add_amplitudevalues_amplitude(double(amplitudevalues_id), std::stod(options2[i][0]), std::stod(options2[i][1]));
  }

  this->add_amplitude(amplitude_id, name_id, shiftx_id, shifty_id, time_type, amplitudevalues_id);

  return true;
}

bool CoreAmplitudes::modify_amplitude(int amplitude_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  int sub_data_id;
  std::vector<int> sub_data_ids;
  int amplitudes_data_id = get_amplitudes_data_id_from_amplitude_id(amplitude_id);
  
  if (amplitudes_data_id == -1)
  {
    return false;
  } else {
    // name
    if (options_marker[0]==1)
    {
      sub_data_id = get_name_amplitude_data_id_from_name_amplitude_id(amplitudes_data[amplitudes_data_id][1]);
      name_amplitude_data[sub_data_id][1] = options[0];
    }
    // shiftx
    if (options_marker[1]==1)
    {
      sub_data_id = get_shiftx_amplitude_data_id_from_shiftx_amplitude_id(amplitudes_data[amplitudes_data_id][2]);
      shiftx_amplitude_data[sub_data_id][1] = options[1];
    }
    // shifty
    if (options_marker[2]==1)
    {
      sub_data_id = get_shifty_amplitude_data_id_from_shifty_amplitude_id(amplitudes_data[amplitudes_data_id][3]);
      shifty_amplitude_data[sub_data_id][1] = options[2];
    }
    // time_type
    if (options_marker[3]==1)
    {
      amplitudes_data[amplitudes_data_id][4] = std::stoi(options[3]);
    }
    // amplitude values
    sub_data_ids = get_amplitudevalues_amplitude_data_ids_from_amplitudevalues_amplitude_id(amplitudes_data[amplitudes_data_id][5]);
    if (options2.size()!=0)
    {
      if (options2.size()==sub_data_ids.size())
      {
        for (size_t i = 0; i < options2.size(); i++)
        {
          amplitudevalues_amplitude_data[sub_data_ids[i]][0] = double(amplitudes_data[amplitudes_data_id][5]);
          amplitudevalues_amplitude_data[sub_data_ids[i]][1] = std::stod(options2[i][0]);
          amplitudevalues_amplitude_data[sub_data_ids[i]][2] = std::stod(options2[i][1]);
        }
      }else{
        // first delete and then make a push back
        // delete from back to begin so that we don't have to care about mismatching id's
        for (size_t i = sub_data_ids.size(); i > 0; i--)
        {
          amplitudevalues_amplitude_data.erase(amplitudevalues_amplitude_data.begin() + sub_data_ids[i-1]);
        }
        
        for (size_t i = 0; i < options2.size(); i++)
        {
          add_amplitudevalues_amplitude(double(amplitudes_data[amplitudes_data_id][5]),std::stod(options2[i][0]),std::stod(options2[i][1]));
        }
      }
    }
    return true;
  }
}

bool CoreAmplitudes::add_amplitude(int amplitude_id, int name_id, int shiftx, int shifty, int time_type, int amplitudevalues_id)
{
  std::vector<int> v = {amplitude_id, name_id, shiftx, shifty, time_type, amplitudevalues_id};
      
  amplitudes_data.push_back(v);

  return true;
}

bool CoreAmplitudes::add_name_amplitude(std::string name_amplitude_id, std::string name)
{
  std::vector<std::string> v = {name_amplitude_id, name};
  
  name_amplitude_data.push_back(v);
  
  return true;
}

bool CoreAmplitudes::add_shiftx_amplitude(std::string shiftx_amplitude_id, std::string shiftx_value)
{
  std::vector<std::string> v = {shiftx_amplitude_id, shiftx_value};
  
  shiftx_amplitude_data.push_back(v);
  
  return true;
}

bool CoreAmplitudes::add_shifty_amplitude(std::string shifty_amplitude_id, std::string shifty_value)
{
  std::vector<std::string> v = {shifty_amplitude_id, shifty_value};
  
  shifty_amplitude_data.push_back(v);
  
  return true;
}

bool CoreAmplitudes::add_amplitudevalues_amplitude(double amplitudevalues_amplitude_id, double amplitudevalues_value1, double amplitudevalues_value2)
{
  std::vector<double> v = {amplitudevalues_amplitude_id, amplitudevalues_value1, amplitudevalues_value2};
  
  amplitudevalues_amplitude_data.push_back(v);
  
  return true;
}

bool CoreAmplitudes::delete_amplitude(int amplitude_id)
{
  int sub_data_id;
  std::vector<int> sub_data_ids;
  int amplitudes_data_id = get_amplitudes_data_id_from_amplitude_id(amplitude_id);
  if (amplitudes_data_id == -1)
  {
    return false;
  } else {
    sub_data_id = get_name_amplitude_data_id_from_name_amplitude_id(amplitudes_data[amplitudes_data_id][1]);
    if (sub_data_id != -1){
      name_amplitude_data.erase(name_amplitude_data.begin() + sub_data_id);  
    }
    sub_data_id = get_shiftx_amplitude_data_id_from_shiftx_amplitude_id(amplitudes_data[amplitudes_data_id][2]);
    if (sub_data_id != -1){
      shiftx_amplitude_data.erase(shiftx_amplitude_data.begin() + sub_data_id);  
    }
    sub_data_id = get_shifty_amplitude_data_id_from_shifty_amplitude_id(amplitudes_data[amplitudes_data_id][3]);
    if (sub_data_id != -1){
      shifty_amplitude_data.erase(shifty_amplitude_data.begin() + sub_data_id);  
    }
    sub_data_ids = get_amplitudevalues_amplitude_data_ids_from_amplitudevalues_amplitude_id(amplitudes_data[amplitudes_data_id][5]);
    for (size_t i = sub_data_ids.size(); i > 0; i--)
    {
      amplitudevalues_amplitude_data.erase(amplitudevalues_amplitude_data.begin() + sub_data_ids[i-1]);
    }
    amplitudes_data.erase(amplitudes_data.begin() + amplitudes_data_id);
    return true;
  }
}

bool CoreAmplitudes::delete_amplitudes(std::vector<int> amplitude_ids)
{
  //only usable to delete amplitudes that were created iterative like in trajectory
  this->update_sorted_vectors();
  auto p = sort_permutation(amplitude_ids);
  this->apply_permutation(amplitude_ids, p);
  //reverse to delete backwards
  std::reverse(amplitude_ids.begin(),amplitude_ids.end()); 

  for (size_t i = 0; i < amplitude_ids.size(); i++)
  {
    int sub_data_id;
    std::vector<int> sub_data_ids;
    int amplitudes_data_id = get_amplitudes_data_id_from_amplitude_id_sorted(amplitude_ids[i]);
    if (amplitudes_data_id == -1)
    {
      return false;
    } else {
      sub_data_id = get_name_amplitude_data_id_from_name_amplitude_id_sorted(amplitudes_data[amplitudes_data_id][1]);
      if (sub_data_id != -1){
        name_amplitude_data.erase(name_amplitude_data.begin() + sub_data_id);  
      }
      sub_data_id = get_shiftx_amplitude_data_id_from_shiftx_amplitude_id_sorted(amplitudes_data[amplitudes_data_id][2]);
      if (sub_data_id != -1){
        shiftx_amplitude_data.erase(shiftx_amplitude_data.begin() + sub_data_id);  
      }
      sub_data_id = get_shifty_amplitude_data_id_from_shifty_amplitude_id_sorted(amplitudes_data[amplitudes_data_id][3]);
      if (sub_data_id != -1){
        shifty_amplitude_data.erase(shifty_amplitude_data.begin() + sub_data_id);  
      }
      sub_data_ids = get_amplitudevalues_amplitude_data_ids_from_amplitudevalues_amplitude_id_sorted(amplitudes_data[amplitudes_data_id][5]);
      for (size_t i = sub_data_ids.size(); i > 0; i--)
      {
        amplitudevalues_amplitude_data.erase(amplitudevalues_amplitude_data.begin() + sub_data_ids[i-1]);
      }
      amplitudes_data.erase(amplitudes_data.begin() + amplitudes_data_id);
    }
  }
  
  return true;
}

int CoreAmplitudes::get_amplitudes_data_id_from_amplitude_id(int amplitude_id)
{ 
  int return_int = -1;
  
  for (size_t i = 0; i < amplitudes_data.size(); i++)
  {
    if (amplitudes_data[i][0]==amplitude_id)
    {
        return_int = int(i);
    }  
  }

  return return_int;
}

int CoreAmplitudes::get_amplitudes_data_id_from_amplitude_id_sorted(int amplitude_id)
{ 
  int return_int = -1;
  
  if (std::binary_search(sorted_amplitudes_ids.begin(), sorted_amplitudes_ids.end(), amplitude_id))
  {
    auto lower = std::lower_bound(sorted_amplitudes_ids.begin(), sorted_amplitudes_ids.end(), amplitude_id);
    return_int = sorted_amplitudes_data_ids[lower-sorted_amplitudes_ids.begin()];
  }
  /*
  for (size_t i = 0; i < amplitudes_data.size(); i++)
  {
    if (amplitudes_data[i][0]==amplitude_id)
    {
        return_int = int(i);
    }  
  }
  */
  return return_int;
}

int CoreAmplitudes::get_name_amplitude_data_id_from_name_amplitude_id(int name_amplitude_id)
{ 
  int return_int = -1;
  
  for (size_t i = 0; i < name_amplitude_data.size(); i++)
  {
    if (name_amplitude_data[i][0]==std::to_string(name_amplitude_id))
    {
        return_int = int(i);
    }  
  }
  
  return return_int;
}

int CoreAmplitudes::get_name_amplitude_data_id_from_name_amplitude_id_sorted(int name_amplitude_id)
{ 
  int return_int = -1;
  if (std::binary_search(sorted_name_ids.begin(), sorted_name_ids.end(), name_amplitude_id))
  {
    auto lower = std::lower_bound(sorted_name_ids.begin(), sorted_name_ids.end(), name_amplitude_id);
    return_int = sorted_name_data_ids[lower-sorted_name_ids.begin()];
  }
  /*
  for (size_t i = 0; i < name_amplitude_data.size(); i++)
  {
    if (name_amplitude_data[i][0]==std::to_string(name_amplitude_id))
    {
        return_int = int(i);
    }  
  }
  */
  return return_int;
}


int CoreAmplitudes::get_shiftx_amplitude_data_id_from_shiftx_amplitude_id(int shiftx_amplitude_id)
{ 
  int return_int = -1;
  
  for (size_t i = 0; i < shiftx_amplitude_data.size(); i++)
  {
    if (shiftx_amplitude_data[i][0]==std::to_string(shiftx_amplitude_id))
    {
        return_int = int(i);
    }  
  }
  
  return return_int;
}

int CoreAmplitudes::get_shiftx_amplitude_data_id_from_shiftx_amplitude_id_sorted(int shiftx_amplitude_id)
{ 
  int return_int = -1;
  if (std::binary_search(sorted_shiftx_ids.begin(), sorted_shiftx_ids.end(), shiftx_amplitude_id))
  {
    auto lower = std::lower_bound(sorted_shiftx_ids.begin(), sorted_shiftx_ids.end(), shiftx_amplitude_id);
    return_int = sorted_shiftx_data_ids[lower-sorted_shiftx_ids.begin()];
  }
  /*
  for (size_t i = 0; i < shiftx_amplitude_data.size(); i++)
  {
    if (shiftx_amplitude_data[i][0]==std::to_string(shiftx_amplitude_id))
    {
        return_int = int(i);
    }  
  }
  */
  return return_int;
}

int CoreAmplitudes::get_shifty_amplitude_data_id_from_shifty_amplitude_id(int shifty_amplitude_id)
{ 
  int return_int = -1;
 
  for (size_t i = 0; i < shifty_amplitude_data.size(); i++)
  {
    if (shifty_amplitude_data[i][0]==std::to_string(shifty_amplitude_id))
    {
        return_int = int(i);
    }  
  }
  
  return return_int;
}

int CoreAmplitudes::get_shifty_amplitude_data_id_from_shifty_amplitude_id_sorted(int shifty_amplitude_id)
{ 
  int return_int = -1;
  if (std::binary_search(sorted_shifty_ids.begin(), sorted_shifty_ids.end(), shifty_amplitude_id))
  {
    auto lower = std::lower_bound(sorted_shifty_ids.begin(), sorted_shifty_ids.end(), shifty_amplitude_id);
    return_int = sorted_shifty_data_ids[lower-sorted_shifty_ids.begin()];
  }
  /*
  for (size_t i = 0; i < shifty_amplitude_data.size(); i++)
  {
    if (shifty_amplitude_data[i][0]==std::to_string(shifty_amplitude_id))
    {
        return_int = int(i);
    }  
  }
  */
  return return_int;
}

std::vector<int> CoreAmplitudes::get_amplitudevalues_amplitude_data_ids_from_amplitudevalues_amplitude_id(int amplitudevalues_amplitude_id)
{ 
  std::vector<int> return_int;

  for (size_t i = 0; i < amplitudevalues_amplitude_data.size(); i++)
  {
    if (amplitudevalues_amplitude_data[i][0]==double(amplitudevalues_amplitude_id))
    {
        return_int.push_back(int(i));
    }  
  }
  
  return return_int;
}

std::vector<int> CoreAmplitudes::get_amplitudevalues_amplitude_data_ids_from_amplitudevalues_amplitude_id_sorted(int amplitudevalues_amplitude_id)
{ 
  std::vector<int> return_int;

  if (std::binary_search(sorted_amplitudevalues_ids.begin(), sorted_amplitudevalues_ids.end(), amplitudevalues_amplitude_id))
  {
    auto lower = std::lower_bound(sorted_amplitudevalues_ids.begin(), sorted_amplitudevalues_ids.end(), amplitudevalues_amplitude_id);
    int first_i = int(lower-sorted_amplitudevalues_ids.begin());
    for (size_t i = first_i; i < sorted_amplitudevalues_ids.size(); i++)
    {
      if (sorted_amplitudevalues_ids[i]==amplitudevalues_amplitude_id)
      {
        return_int.push_back(sorted_amplitudevalues_data_ids[i]);
      }else{
        return return_int;
      } 
    }
  }

  /*
  for (size_t i = 0; i < amplitudevalues_amplitude_data.size(); i++)
  {
    if (amplitudevalues_amplitude_data[i][0]==double(amplitudevalues_amplitude_id))
    {
        return_int.push_back(int(i));
    }  
  }
  */
  return return_int;
}

std::string CoreAmplitudes::get_amplitude_export() // get a list of the CalculiX amplitude exports
{
  std::vector<std::string> amplitudes_export_list;
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
      
      str_temp.append(ccx_iface->to_string_scientific(amplitudevalues_amplitude_data[sub_data_ids[i]][1]));
      str_temp.append(",");
      str_temp.append(ccx_iface->to_string_scientific(amplitudevalues_amplitude_data[sub_data_ids[i]][2]));
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

  return amplitude_export;
}

std::string CoreAmplitudes::print_data()
{
  std::string str_return;
  str_return = "\n CoreAmplitudes amplitudes_data: \n";
  str_return.append("amplitude_id, name_id, shiftx_id, shifty_id, time_type \n");

  for (size_t i = 0; i < amplitudes_data.size(); i++)
  {
    str_return.append(std::to_string(amplitudes_data[i][0]) + " " + std::to_string(amplitudes_data[i][1]) + " " + std::to_string(amplitudes_data[i][2]) + " " + std::to_string(amplitudes_data[i][3]) + " " + std::to_string(amplitudes_data[i][4]) + " \n");
  }

  str_return.append("\n CoreAmplitudes name_amplitude_data: \n");
  str_return.append("name_amplitude_id, name \n");

  for (size_t i = 0; i < name_amplitude_data.size(); i++)
  {
    str_return.append(name_amplitude_data[i][0] + " " + name_amplitude_data[i][1] + " \n");
  }

  str_return.append("\n CoreAmplitudes shiftx_amplitude_data: \n");
  str_return.append("shiftx_amplitude_id, shiftx_value \n");

  for (size_t i = 0; i < shiftx_amplitude_data.size(); i++)
  {
    str_return.append(shiftx_amplitude_data[i][0] + " " + shiftx_amplitude_data[i][1] + " \n");
  }
 
  str_return.append("\n CoreAmplitudes shifty_amplitude_data: \n");
  str_return.append("shifty_amplitude_id, shifty_value \n");

  for (size_t i = 0; i < shifty_amplitude_data.size(); i++)
  {
    str_return.append(shifty_amplitude_data[i][0] + " " + shifty_amplitude_data[i][1] + " \n");
  }

  str_return.append("\n CoreAmplitudes amplitudevalues_amplitude_data: \n");
  str_return.append("amplitudevalues_amplitude_id, amplitudevalues_value1, amplitudevalues_value2 \n");

  for (size_t i = 0; i < amplitudevalues_amplitude_data.size(); i++)
  {
    str_return.append(std::to_string(amplitudevalues_amplitude_data[i][0]) + " " + std::to_string(amplitudevalues_amplitude_data[i][1]) + " " + std::to_string(amplitudevalues_amplitude_data[i][2]) + " \n");
  }

  return str_return;
}


bool CoreAmplitudes::update_sorted_vectors()
{
  // sorting for faster search
  std::vector<int> tmp_sorted_amplitudes_ids; //amplitudes
  std::vector<int> tmp_sorted_amplitudes_data_ids; //amplitudes
  std::vector<int> tmp_sorted_name_ids; //names
  std::vector<int> tmp_sorted_name_data_ids; //names
  std::vector<int> tmp_sorted_shiftx_ids; //shiftx
  std::vector<int> tmp_sorted_shiftx_data_ids; //shiftx
  std::vector<int> tmp_sorted_shifty_ids; //shifty
  std::vector<int> tmp_sorted_shifty_data_ids; //shifty
  std::vector<int> tmp_sorted_amplitudevalues_ids; //amplitude values
  std::vector<int> tmp_sorted_amplitudevalues_data_ids; //amplitude values

  for (size_t i = 0; i < amplitudes_data.size(); i++)
  {
    tmp_sorted_amplitudes_ids.push_back(amplitudes_data[i][0]);
    tmp_sorted_amplitudes_data_ids.push_back(int(i));
  }  
  auto p = sort_permutation(tmp_sorted_amplitudes_ids);
  this->apply_permutation(tmp_sorted_amplitudes_ids, p);
  this->apply_permutation(tmp_sorted_amplitudes_data_ids, p);

  for (size_t i = 0; i < name_amplitude_data.size(); i++)
  {
    tmp_sorted_name_ids.push_back(std::stoi(name_amplitude_data[i][0]));
    tmp_sorted_name_data_ids.push_back(int(i));
  }  
  p = sort_permutation(tmp_sorted_name_ids);
  this->apply_permutation(tmp_sorted_name_ids, p);
  this->apply_permutation(tmp_sorted_name_data_ids, p);

  for (size_t i = 0; i < shiftx_amplitude_data.size(); i++)
  {
    tmp_sorted_shiftx_ids.push_back(std::stoi(shiftx_amplitude_data[i][0]));
    tmp_sorted_shiftx_data_ids.push_back(int(i));
  }  
  p = sort_permutation(tmp_sorted_shiftx_ids);
  this->apply_permutation(tmp_sorted_shiftx_ids, p);
  this->apply_permutation(tmp_sorted_shiftx_data_ids, p);

  for (size_t i = 0; i < shifty_amplitude_data.size(); i++)
  {
    tmp_sorted_shifty_ids.push_back(std::stoi(shifty_amplitude_data[i][0]));
    tmp_sorted_shifty_data_ids.push_back(int(i));
  }  
  p = sort_permutation(tmp_sorted_shifty_ids);
  this->apply_permutation(tmp_sorted_shifty_ids, p);
  this->apply_permutation(tmp_sorted_shifty_data_ids, p);

  for (size_t i = 0; i < amplitudevalues_amplitude_data.size(); i++)
  {
    tmp_sorted_amplitudevalues_ids.push_back(int(amplitudevalues_amplitude_data[i][0]));
    tmp_sorted_amplitudevalues_data_ids.push_back(int(i));
  }  
  p = sort_permutation(tmp_sorted_amplitudevalues_ids);
  this->apply_permutation(tmp_sorted_amplitudevalues_ids, p);
  this->apply_permutation(tmp_sorted_amplitudevalues_data_ids, p);

  sorted_amplitudes_ids = tmp_sorted_amplitudes_ids;
  sorted_amplitudes_data_ids = tmp_sorted_amplitudes_data_ids;
  sorted_name_ids = tmp_sorted_name_ids;
  sorted_name_data_ids = tmp_sorted_name_data_ids;
  sorted_shiftx_ids = tmp_sorted_shiftx_ids;
  sorted_shiftx_data_ids = tmp_sorted_shiftx_data_ids;
  sorted_shifty_ids = tmp_sorted_shifty_ids;
  sorted_shifty_data_ids = tmp_sorted_shifty_data_ids;
  sorted_amplitudevalues_ids = tmp_sorted_amplitudevalues_ids;
  sorted_amplitudevalues_data_ids = tmp_sorted_amplitudevalues_data_ids;

  return true;
}
//sorting of vectors
template <typename T> 
std::vector<std::size_t> CoreAmplitudes::sort_permutation(
    const std::vector<T>& vec)
{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
        [&](std::size_t i, std::size_t j){ return vec[i] < vec[j]; });

    return p;
}

template <typename T> 
void CoreAmplitudes::apply_permutation(
    std::vector<T>& vec,
    const std::vector<std::size_t>& p)
{
    std::vector<bool> done(vec.size());
    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        if (done[i])
        {
            continue;
        }
        done[i] = true;
        std::size_t prev_j = i;
        std::size_t j = p[i];
        while (i != j)
        {
            std::swap(vec[prev_j], vec[j]);
            done[j] = true;
            prev_j = j;
            j = p[j];
        }
    }
}