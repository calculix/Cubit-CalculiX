#include "CoreFieldOutputs.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreFieldOutputs::CoreFieldOutputs()
{
  node_keys.push_back("CP");
  node_keys.push_back("DEPF");
  node_keys.push_back("DEPT");
  node_keys.push_back("DTF");
  node_keys.push_back("HCRI");
  node_keys.push_back("KEQ");
  node_keys.push_back("MACH");
  node_keys.push_back("MAXU");
  node_keys.push_back("MF");
  node_keys.push_back("NT");
  node_keys.push_back("PNT");
  node_keys.push_back("POT");
  node_keys.push_back("PRF");
  node_keys.push_back("PS");
  node_keys.push_back("PSF");
  node_keys.push_back("PT");
  node_keys.push_back("PTF");
  node_keys.push_back("PU");
  node_keys.push_back("RF");
  node_keys.push_back("RFL");
  node_keys.push_back("SEN");
  node_keys.push_back("TS");
  node_keys.push_back("TSF");
  node_keys.push_back("TT");
  node_keys.push_back("TTF");
  node_keys.push_back("TURB");
  node_keys.push_back("U");
  node_keys.push_back("V");
  node_keys.push_back("VF");
 
  element_keys.push_back("CEEQ");
  element_keys.push_back("E");
  element_keys.push_back("ECD");
  element_keys.push_back("EMFB");
  element_keys.push_back("EMFE");
  element_keys.push_back("ENER");
  element_keys.push_back("ERR");
  element_keys.push_back("HER");
  element_keys.push_back("HFL");
  element_keys.push_back("HFLF");
  element_keys.push_back("MAXE");
  element_keys.push_back("MAXS");
  element_keys.push_back("ME");
  element_keys.push_back("PEEQ");
  element_keys.push_back("PHS");
  element_keys.push_back("S");
  element_keys.push_back("SF");
  element_keys.push_back("SMID");
  element_keys.push_back("SNEG");
  element_keys.push_back("SPOS");
  element_keys.push_back("SVF");
  element_keys.push_back("SDV");
  element_keys.push_back("THE");
  element_keys.push_back("ZZS");

  contact_keys.push_back("CDIS");
  contact_keys.push_back("CSTR");
  contact_keys.push_back("CELS");
  contact_keys.push_back("PCON");
}

CoreFieldOutputs::~CoreFieldOutputs()
{}

bool CoreFieldOutputs::init()
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

bool CoreFieldOutputs::update()
{ 
  return true;
}

bool CoreFieldOutputs::reset()
{
  outputs_data.clear();
  name_data.clear();
  node_data.clear();
  element_data.clear();
  contact_data.clear();
  
  init();
  return true;
}

bool CoreFieldOutputs::check_initialized()
{
  return is_initialized;
}

bool CoreFieldOutputs::create_output(std::vector<std::string> options)
{
  int output_id;
  int output_last;
  int sub_id;
  int sub_last;
  int name_id;
  int output_type;
  //int node_id;
  //int element_id;
  //int contact_id;
  
  if (outputs_data.size()==0)
  {
    output_id = 1;
  }
  else
  {
    output_last = outputs_data.size() - 1;
    output_id = outputs_data[output_last][0] + 1;
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
  
  // output_type
  output_type = std::stoi(options[1]);
  
  if (output_type == 1)
  {  // node
    if (node_data.size()==0)
    {
      sub_id = 1;
    }
    else
    {
      sub_last = node_data.size() - 1;
      sub_id = std::stoi(node_data[sub_last][0]) + 1;
    }
    //node_id = sub_id;
    this->add_node(std::to_string(sub_id));
  }else if (output_type == 2)
  { // element
    if (element_data.size()==0)
    {
      sub_id = 1;
    }
    else
    {
      sub_last = element_data.size() - 1;
      sub_id = std::stoi(element_data[sub_last][0]) + 1;
    }
    //element_id = sub_id;
    this->add_element(std::to_string(sub_id));
  }else if (output_type == 3)
  { // contact
    if (contact_data.size()==0)
    {
      sub_id = 1;
    }
    else
    {
      sub_last = contact_data.size() - 1;
      sub_id = std::stoi(contact_data[sub_last][0]) + 1;
    }
    //contact_id = sub_id;
    this->add_contact(std::to_string(sub_id));
  }
  
  this->add_output(output_id, name_id, output_type, sub_id);
  return true;
}

bool CoreFieldOutputs::modify_output(int output_id,int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  int sub_data_id;
  int outputs_data_id = get_outputs_data_id_from_output_id(output_id);
  //modify_type = 0 for changing name

  if (outputs_data_id == -1)
  {
    return false;
  } else {
    // name
    if (modify_type==0)
    {
      if (options_marker[0]==1)
      {
        sub_data_id = get_name_data_id_from_name_id(outputs_data[outputs_data_id][1]);
        name_data[sub_data_id][1] = options[0];
      }
    }
    // node
    if ((outputs_data[outputs_data_id][2]==1) && (outputs_data[outputs_data_id][2]==modify_type))
    {
      sub_data_id = get_node_data_id_from_node_id(outputs_data[outputs_data_id][3]);
      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          node_data[sub_data_id][i+1] = options[i];
        }
      }
    } else if ((outputs_data[outputs_data_id][2]==2) && (outputs_data[outputs_data_id][2]==modify_type))
    { // element
      sub_data_id = get_element_data_id_from_element_id(outputs_data[outputs_data_id][3]);
      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          element_data[sub_data_id][i+1] = options[i];
        }
      }
    } else if ((outputs_data[outputs_data_id][2]==3) && (outputs_data[outputs_data_id][2]==modify_type))
    { // contact 
      sub_data_id = get_contact_data_id_from_contact_id(outputs_data[outputs_data_id][3]);
      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          contact_data[sub_data_id][i+1] = options[i];
        }
      }
    }
    return true;
  }
}

bool CoreFieldOutputs::add_output(int output_id, int name_id, int output_type, int output_type_id)
{
  std::vector<int> v = {output_id, name_id, output_type, output_type_id};
      
  outputs_data.push_back(v);

  return true;
}

bool CoreFieldOutputs::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  
  name_data.push_back(v);
  
  return true;
}

bool CoreFieldOutputs::add_node(std::string node_id)
{
  std::vector<std::string> v = {node_id};
  for (size_t i = 1; i < 40; i++)
  {
    v.push_back("");
  }
  
  node_data.push_back(v);
  
  return true;
}

bool CoreFieldOutputs::add_element(std::string element_id)
{
  std::vector<std::string> v = {element_id};
  for (size_t i = 1; i < 35; i++)
  {
    v.push_back("");
  }

  element_data.push_back(v);
  
  return true;
}

bool CoreFieldOutputs::add_contact(std::string contact_id)
{
  std::vector<std::string> v = {contact_id};

  for (size_t i = 1; i < 9; i++)
  {
    v.push_back("");
  }

  contact_data.push_back(v);
  
  return true;
}

bool CoreFieldOutputs::delete_output(int output_id)
{
  int sub_data_id;
  int outputs_data_id = get_outputs_data_id_from_output_id(output_id);
  if (outputs_data_id == -1)
  {
    return false;
  } else {
    sub_data_id = get_name_data_id_from_name_id(outputs_data[outputs_data_id][1]);
    if (sub_data_id != -1){
      name_data.erase(name_data.begin() + sub_data_id);  
    }
    if (outputs_data[outputs_data_id][2]==1)
    {
      sub_data_id = get_node_data_id_from_node_id(outputs_data[outputs_data_id][3]);
      if (sub_data_id != -1){
        node_data.erase(node_data.begin() + sub_data_id);  
      }
    }else if (outputs_data[outputs_data_id][2]==2)
    {
      sub_data_id = get_element_data_id_from_element_id(outputs_data[outputs_data_id][3]);
      if (sub_data_id != -1){
        element_data.erase(element_data.begin() + sub_data_id);  
      }
    }else if (outputs_data[outputs_data_id][2]==3)
    {
      sub_data_id = get_contact_data_id_from_contact_id(outputs_data[outputs_data_id][3]);
      if (sub_data_id != -1){
        contact_data.erase(contact_data.begin() + sub_data_id);  
      }
    }
    outputs_data.erase(outputs_data.begin() + outputs_data_id);
    return true;
  }
}

int CoreFieldOutputs::get_outputs_data_id_from_output_id(int output_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < outputs_data.size(); i++)
  {
    if (outputs_data[i][0]==output_id)
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreFieldOutputs::get_name_data_id_from_name_id(int name_id)
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

int CoreFieldOutputs::get_node_data_id_from_node_id(int node_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < node_data.size(); i++)
  {
    if (node_data[i][0]==std::to_string(node_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreFieldOutputs::get_element_data_id_from_element_id(int element_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < element_data.size(); i++)
  {
    if (element_data[i][0]==std::to_string(element_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreFieldOutputs::get_contact_data_id_from_contact_id(int contact_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < contact_data.size(); i++)
  {
    if (contact_data[i][0]==std::to_string(contact_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

std::string CoreFieldOutputs::get_output_export(std::vector<int> output_ids) // get a list of the CalculiX output exports
{
  std::vector<std::string> outputs_export_list;
  outputs_export_list.push_back("********************************** H I S T O R Y O U T P U T S ****************************");
  std::string str_temp;
  int output_data_id;
  int sub_data_id;
  std::vector<int> sub_data_ids;
  /*
  //loop over all outputs
  for (size_t i = 0; i < output_ids.size(); i++)
  { 
    output_data_id = get_outputs_data_id_from_output_id(output_ids[i]);

    str_temp = "*AMPLITUDE, NAME=";
    sub_data_id = get_name_amplitude_data_id_from_name_amplitude_id(outputs_data[i][1]);
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
  std::string output_export;

  for (size_t i = 0; i < outputs_export_list.size(); i++)
  {
    output_export.append(outputs_export_list[i] + "\n");
  }

  return output_export;
}

std::string CoreFieldOutputs::print_data()
{
  std::string str_return;
  str_return = "\n CoreFieldOutputs outputs_data: \n";
  str_return.append("output_id, name_id, output_type, output_type_id \n");

  for (size_t i = 0; i < outputs_data.size(); i++)
  {
    str_return.append(std::to_string(outputs_data[i][0]) + " " + std::to_string(outputs_data[i][1]) + " " + std::to_string(outputs_data[i][2]) + " " + std::to_string(outputs_data[i][3]) + " \n");
  }

  str_return.append("\n CoreFieldOutputs name_data: \n");
  str_return.append("name_id, name \n");

  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + " \n");
  }

  str_return.append("\n CoreFieldOutputs node_data: \n");
  str_return.append("node_id, nodeset_id, FREQUENCY, FREQUENCYF, TOTALS, GLOBAL, OUTPUT, OUTPUT ALL, TIME POINTS, LAST ITERATIONS, CONTACT ELEMENTS");

  for (size_t i = 0; i < node_keys.size(); i++)
  {
    str_return.append(", " + node_keys[i]);
  }
  str_return.append("\n");

  for (size_t i = 0; i < node_data.size(); i++)
  {
    for (size_t ii = 0; ii < 40; ii++)
    {
      str_return.append(node_data[i][ii] + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreFieldOutputs element_data: \n");
  str_return.append("element_id, elementset_id, FREQUENCY, FREQUENCYF, GLOBAL, OUTPUT, OUTPUT ALL, SECTION FORCES, TIME POINTS, LAST ITERATIONS, CONTACT ELEMENTS");

  for (size_t i = 0; i < element_keys.size(); i++)
  {
    str_return.append(", " + element_keys[i]);
  }
  str_return.append("\n");

  for (size_t i = 0; i < element_data.size(); i++)
  {
    for (size_t ii = 0; ii < 35; ii++)
    {
      str_return.append(element_data[i][ii] + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreFieldOutputs contact_data: \n");
  str_return.append("contact_id, FREQUENCY, TIME POINTS, LAST ITERATIONS, CONTACT ELEMENTS");
  for (size_t i = 0; i < contact_keys.size(); i++)
  {
    str_return.append(", " + contact_keys[i]);
  }
  str_return.append("\n");

  for (size_t i = 0; i < contact_data.size(); i++)
  {
    for (size_t ii = 0; ii < 9; ii++)
    {
      str_return.append(contact_data[i][ii] + " ");
    }
    str_return.append("\n");
  }

  return str_return;
}