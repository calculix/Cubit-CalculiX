#include "CoreHistoryOutputs.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreHistoryOutputs::CoreHistoryOutputs()
{
  node_keys.push_back("U");
  node_keys.push_back("NT");
  node_keys.push_back("TSF");
  node_keys.push_back("TTF");
  node_keys.push_back("PN");
  node_keys.push_back("PSF");
  node_keys.push_back("PTF");
  node_keys.push_back("MACH");
  node_keys.push_back("CP");
  node_keys.push_back("VF");
  node_keys.push_back("DEPF");
  node_keys.push_back("TURB");
  node_keys.push_back("MF");
  node_keys.push_back("RF");
  node_keys.push_back("RFL");
 
  element_keys.push_back("S");
  element_keys.push_back("SVF");
  element_keys.push_back("E");
  element_keys.push_back("ME");
  element_keys.push_back("PEEQ");
  element_keys.push_back("CEEQ");
  element_keys.push_back("ENER");
  element_keys.push_back("SDV");
  element_keys.push_back("HFL");
  element_keys.push_back("HFLF");
  element_keys.push_back("COORD");
  element_keys.push_back("ELSE");
  element_keys.push_back("ELKE");
  element_keys.push_back("EVOL");
  element_keys.push_back("EMAS");
  element_keys.push_back("EBHE");
  element_keys.push_back("CENT");

  contact_keys.push_back("CDIS");
  contact_keys.push_back("CSTR");
  contact_keys.push_back("CELS");
  contact_keys.push_back("CNUM");
  contact_keys.push_back("CF");
  contact_keys.push_back("CFN");
  contact_keys.push_back("CFS");
}

CoreHistoryOutputs::~CoreHistoryOutputs()
{}

bool CoreHistoryOutputs::init()
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

bool CoreHistoryOutputs::update()
{ 
  return true;
}

bool CoreHistoryOutputs::reset()
{
  outputs_data.clear();
  name_data.clear();
  node_data.clear();
  element_data.clear();
  contact_data.clear();
  
  init();
  return true;
}

bool CoreHistoryOutputs::check_initialized()
{
  return is_initialized;
}

bool CoreHistoryOutputs::create_output(std::vector<std::string> options)
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

bool CoreHistoryOutputs::modify_output(int output_id,int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
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

bool CoreHistoryOutputs::add_output(int output_id, int name_id, int output_type, int output_type_id)
{
  std::vector<int> v = {output_id, name_id, output_type, output_type_id};
      
  outputs_data.push_back(v);

  return true;
}

bool CoreHistoryOutputs::add_name(std::string name_id, std::string name)
{
  std::vector<std::string> v = {name_id, name};
  
  name_data.push_back(v);
  
  return true;
}

bool CoreHistoryOutputs::add_node(std::string node_id)
{
  std::vector<std::string> v = {node_id,"","","","","","","","","","","","","","","","","","","","",""};
  
  node_data.push_back(v);
  
  return true;
}

bool CoreHistoryOutputs::add_element(std::string element_id)
{
  std::vector<std::string> v = {element_id,"","","","","","","","","","","","","","","","","","","","","","",""};

  element_data.push_back(v);
  
  return true;
}

bool CoreHistoryOutputs::add_contact(std::string contact_id)
{
  std::vector<std::string> v = {contact_id,"","","","","","","","","","","",""};

  contact_data.push_back(v);
  
  return true;
}

bool CoreHistoryOutputs::delete_output(int output_id)
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

int CoreHistoryOutputs::get_outputs_data_id_from_output_id(int output_id)
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

int CoreHistoryOutputs::get_name_data_id_from_name_id(int name_id)
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

int CoreHistoryOutputs::get_node_data_id_from_node_id(int node_id)
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

int CoreHistoryOutputs::get_element_data_id_from_element_id(int element_id)
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

int CoreHistoryOutputs::get_contact_data_id_from_contact_id(int contact_id)
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

std::string CoreHistoryOutputs::get_output_export(int output_id) // get a list of the CalculiX output export
{
  std::vector<std::string> outputs_export_list;
  std::string str_temp;
  std::string output_name;
  int output_data_id;
  int sub_data_id;
  
  output_data_id = get_outputs_data_id_from_output_id(output_id);

  sub_data_id = get_name_data_id_from_name_id(outputs_data[output_data_id][1]);
  output_name = name_data[sub_data_id][1];
    
  if (outputs_data[output_data_id][2]==1)
  {
    sub_data_id = get_node_data_id_from_node_id(outputs_data[output_data_id][3]);
    str_temp = "*NODE PRINT, NSET=";
    if (node_data[sub_data_id][1]=="")
    {
      return "";
    }
    
    str_temp.append(ccx_iface->get_nodeset_name(std::stoi(node_data[sub_data_id][1])));

    outputs_export_list.push_back(str_temp);

    //second line
    str_temp = "";
    for (size_t i = 7; i < 7 + node_keys.size(); i++)
    {
      if (i!=7)
      {
        str_temp.append(",");
      }
      if (node_data[sub_data_id][i]!="")
      {
        str_temp.append(node_data[sub_data_id][i]);
      }
    }

  }else if (outputs_data[output_data_id][2]==2)
  {
    output_name = output_name + " (element)";
  }else if (outputs_data[output_data_id][2]==3)
  {
    output_name = output_name + " (contact)";
  }

  outputs_export_list.push_back(str_temp);

  std::string output_export;

  for (size_t i = 0; i < outputs_export_list.size(); i++)
  {
    output_export.append(outputs_export_list[i] + "\n");
  }

  return output_export;
}

std::string CoreHistoryOutputs::print_data()
{
  std::string str_return;
  str_return = "\n CoreHistoryOutputs outputs_data: \n";
  str_return.append("output_id, name_id, output_type, output_type_id \n");

  for (size_t i = 0; i < outputs_data.size(); i++)
  {
    str_return.append(std::to_string(outputs_data[i][0]) + " " + std::to_string(outputs_data[i][1]) + " " + std::to_string(outputs_data[i][2]) + " " + std::to_string(outputs_data[i][3]) + " \n");
  }

  str_return.append("\n CoreHistoryOutputs name_data: \n");
  str_return.append("name_id, name \n");

  for (size_t i = 0; i < name_data.size(); i++)
  {
    str_return.append(name_data[i][0] + " " + name_data[i][1] + " \n");
  }

  str_return.append("\n CoreHistoryOutputs node_data: \n");
  str_return.append("node_id, nodeset_id, FREQUENCY, FREQUENCYF, TOTALS, GLOBAL, TIME POINTS");
  for (size_t i = 0; i < node_keys.size(); i++)
  {
    str_return.append(", " + node_keys[i]);
  }
  str_return.append("\n");

  for (size_t i = 0; i < node_data.size(); i++)
  {
    for (size_t ii = 0; ii < 22; ii++)
    {
      str_return.append(node_data[i][ii] + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreHistoryOutputs element_data: \n");
  str_return.append("element_id, elementset_id, FREQUENCY, FREQUENCYF, TOTALS, GLOBAL, TIME POINTS");

  for (size_t i = 0; i < element_keys.size(); i++)
  {
    str_return.append(", " + element_keys[i]);
  }
  str_return.append("\n");

  for (size_t i = 0; i < element_data.size(); i++)
  {
    for (size_t ii = 0; ii < 24; ii++)
    {
      str_return.append(element_data[i][ii] + " ");
    }
    str_return.append("\n");
  }

  str_return.append("\n CoreHistoryOutputs contact_data: \n");
  str_return.append("contact_id, contactpair_id, FREQUENCY, TOTALS, GLOBAL, TIME POINTS");
  for (size_t i = 0; i < contact_keys.size(); i++)
  {
    str_return.append(", " + contact_keys[i]);
  }
  str_return.append("\n");

  for (size_t i = 0; i < contact_data.size(); i++)
  {
    for (size_t ii = 0; ii < 13; ii++)
    {
      str_return.append(contact_data[i][ii] + " ");
    }
    str_return.append("\n");
  }

  return str_return;
}