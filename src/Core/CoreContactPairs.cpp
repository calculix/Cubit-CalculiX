#include "CoreContactPairs.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreContactPairs::CoreContactPairs()
{}

CoreContactPairs::~CoreContactPairs()
{}

bool CoreContactPairs::init()
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

bool CoreContactPairs::update()
{ 
  return true;
}

bool CoreContactPairs::reset()
{
  contactpairs_data.clear();
  adjust_contactpair_data.clear();
  
  init();
  return true;
}

bool CoreContactPairs::check_initialized()
{
  return is_initialized;
}

bool CoreContactPairs::create_contactpair(std::vector<std::string> options)
{
  int contactpair_id;
  std::string contactpair_type;
  int contactpair_type_value;
  int contactpair_last;
  int sub_id;
  int sub_last;
  int surfaceinteractions_id;
  int master_id;
  int slave_id;
  int adjust_id;
  
  contactpair_type = options[1];

  if (contactpair_type=="NODE TO SURFACE")
  {
    contactpair_type_value = 1;
  } else if (contactpair_type=="SURFACE TO SURFACE")
  {
    contactpair_type_value = 2;
  } else if (contactpair_type=="MORTAR")
  {
    contactpair_type_value = 3;
  } else if (contactpair_type=="LINMORTAR")
  {
    contactpair_type_value = 4;
  } else if (contactpair_type=="PGLINMORTAR")
  {
    contactpair_type_value = 5;
  } else if (contactpair_type=="MASSLESS")
  {
    contactpair_type_value = 6;
  }

  if (contactpairs_data.size()==0)
  {
    contactpair_id = 1;
  }
  else
  {
    contactpair_last = contactpairs_data.size() - 1;
    contactpair_id = contactpairs_data[contactpair_last][0] + 1;
  }

  if (adjust_contactpair_data.size()==0)
  {
    sub_id = 1;
  }
  else
  {
    sub_last = adjust_contactpair_data.size() - 1;
    sub_id = std::stoi(adjust_contactpair_data[sub_last][0]) + 1;
  }
  this->add_adjust_contactpair(std::to_string(sub_id), options[4]);

  //contactpair_id, surfaceinteractions_id, contactpair_type, master_id, slave_id, adjust_id
  surfaceinteractions_id = std::stoi(options[0]);
  master_id = std::stoi(options[2]);
  slave_id = std::stoi(options[3]);
  adjust_id = sub_id;
  
  this->add_contactpair(contactpair_id, surfaceinteractions_id, contactpair_type_value, master_id, slave_id, adjust_id);
  return true;
}

bool CoreContactPairs::modify_contactpair(int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  std::string contactpair_type;
  int contactpair_type_value;
  int sub_data_id;
  int contactpairs_data_id = get_contactpairs_data_id_from_contactpair_id(contactpair_id);
  
  if (contactpairs_data_id == -1)
  {
    return false;
  } else {
    // surfaceinteraction id
    if (options_marker[0]==1)
    {
      contactpairs_data[contactpairs_data_id][1] = std::stoi(options[0]);
    }
    // contactpair type
    if (options_marker[1]==1)
    {
      contactpair_type = options[1];

      if (contactpair_type=="NODE TO SURFACE")
      {
        contactpair_type_value = 1;
      } else if (contactpair_type=="SURFACE TO SURFACE")
      {
        contactpair_type_value = 2;
      } else if (contactpair_type=="MORTAR")
      {
        contactpair_type_value = 3;
      } else if (contactpair_type=="LINMORTAR")
      {
        contactpair_type_value = 4;
      } else if (contactpair_type=="PGLINMORTAR")
      {
        contactpair_type_value = 5;
      } else if (contactpair_type=="MASSLESS")
      {
        contactpair_type_value = 6;
      }
      contactpairs_data[contactpairs_data_id][2] = contactpair_type_value;
    }
    // master id
    if (options_marker[2]==1)
    {
      contactpairs_data[contactpairs_data_id][3] = std::stoi(options[2]);
    }
    // slave id
    if (options_marker[3]==1)
    {
      contactpairs_data[contactpairs_data_id][4] = std::stoi(options[3]);
    }
    // adjust
    if (options_marker[4]==1)
    {
      sub_data_id = get_adjust_contactpair_data_id_from_adjust_contactpair_id(contactpairs_data[contactpairs_data_id][5]);
      adjust_contactpair_data[sub_data_id][1] = options[4];
    }
    return true;
  }
}

bool CoreContactPairs::add_contactpair(int contactpair_id, int surfaceinteractions_id, int contactpair_type, int master_id, int slave_id, int adjust_id)
{
  std::vector<int> v = {contactpair_id, surfaceinteractions_id, contactpair_type, master_id, slave_id, adjust_id};
      
  contactpairs_data.push_back(v);

  return true;
}

bool CoreContactPairs::add_adjust_contactpair(std::string adjust_contactpair_id, std::string adjust_value)
{
  std::vector<std::string> v = {adjust_contactpair_id, adjust_value};
  
  adjust_contactpair_data.push_back(v);
  
  return true;
}

bool CoreContactPairs::delete_contactpair(int contactpair_id)
{
  int sub_data_id;
  int contactpairs_data_id = get_contactpairs_data_id_from_contactpair_id(contactpair_id);
  if (contactpairs_data_id == -1)
  {
    return false;
  } else {
    if (contactpairs_data[contactpairs_data_id][1]==1)
    {
      sub_data_id = get_adjust_contactpair_data_id_from_adjust_contactpair_id(contactpairs_data[contactpairs_data_id][5]);
      if (sub_data_id != -1){
        adjust_contactpair_data.erase(adjust_contactpair_data.begin() + sub_data_id);  
      }
    }
    contactpairs_data.erase(contactpairs_data.begin() + contactpairs_data_id);
    return true;
  }
}

int CoreContactPairs::get_contactpairs_data_id_from_contactpair_id(int contactpair_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < contactpairs_data.size(); i++)
  {
    if (contactpairs_data[i][0]==contactpair_id)
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreContactPairs::get_adjust_contactpair_data_id_from_adjust_contactpair_id(int adjust_contactpair_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < adjust_contactpair_data.size(); i++)
  {
    if (adjust_contactpair_data[i][0]==std::to_string(adjust_contactpair_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

std::string CoreContactPairs::get_contactpair_export() // get a list of the CalculiX contactpair exports
{
  std::vector<std::string> constraints_export_list;
  constraints_export_list.push_back("********************************** C O N S T R A I N T S ****************************");
  std::string str_temp;
  int sub_constraint_data_id;
  /*
  //loop over all constraints
  for (size_t i = 0; i < constraints_data.size(); i++)
  { 
    // RIGID BODY
    if (constraints_data[i][1] == 1)
    {
      sub_constraint_data_id = get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(constraints_data[i][2]);

      str_temp = "*RIGID BODY, ";
      
      if (rigidbody_constraint_data[sub_constraint_data_id][1]=="1")
      {
        str_temp.append("NSET=");
        str_temp.append(ccx_iface->get_nodeset_name(std::stoi(rigidbody_constraint_data[sub_constraint_data_id][2])));
      } else if (rigidbody_constraint_data[sub_constraint_data_id][1]=="2")
      {
        str_temp.append("ELSET=");
        str_temp.append(ccx_iface->get_block_name(std::stoi(rigidbody_constraint_data[sub_constraint_data_id][2])));
      }

      str_temp.append(", REF NODE=");
      str_temp.append(std::to_string(ccx_iface->referencepoints_get_ref_from_vertex_id(std::stoi(rigidbody_constraint_data[sub_constraint_data_id][3]))));
      
      str_temp.append(", ROT NODE=");
      str_temp.append(std::to_string(ccx_iface->referencepoints_get_rot_from_vertex_id(std::stoi(rigidbody_constraint_data[sub_constraint_data_id][3]))));
      constraints_export_list.push_back(str_temp);
    }
    // TIE
    if (constraints_data[i][1] == 2) 
    {
      sub_constraint_data_id = get_tie_constraint_data_id_from_tie_constraint_id(constraints_data[i][2]);
      
      str_temp = "*TIE, NAME=";
      str_temp.append(tie_constraint_data[sub_constraint_data_id][1]);
      
      if (tie_constraint_data[sub_constraint_data_id][4]!="")
      {
        str_temp.append(", POSITION TOLERANCE=");
        str_temp.append(tie_constraint_data[sub_constraint_data_id][4]);
      }

      constraints_export_list.push_back(str_temp);
      // second line
      str_temp = "";
      str_temp.append(ccx_iface->get_sideset_name(std::stoi(tie_constraint_data[sub_constraint_data_id][2])));
      str_temp.append(",");
      str_temp.append(ccx_iface->get_sideset_name(std::stoi(tie_constraint_data[sub_constraint_data_id][3])));
      constraints_export_list.push_back(str_temp);
    }
  }
  */
  std::string constraint_export;

  for (size_t i = 0; i < constraints_export_list.size(); i++)
  {
    constraint_export.append(constraints_export_list[i] + "\n");
  }
  
  return constraint_export;
}

std::string CoreContactPairs::print_data()
{
  std::string str_return;
  str_return = "\n CoreContactPairs contactpairs_data: \n";
  str_return.append("contactpair_id, surfaceinteractions_id, contactpair_type, master_id, slave_id, adjust_id \n");

  for (size_t i = 0; i < contactpairs_data.size(); i++)
  {
    str_return.append(std::to_string(contactpairs_data[i][0]) + " " + std::to_string(contactpairs_data[i][1]) + " " + std::to_string(contactpairs_data[i][2]) + " " + std::to_string(contactpairs_data[i][3]) + " " + std::to_string(contactpairs_data[i][4]) + " " + std::to_string(contactpairs_data[i][5]) + " \n");
  }

  str_return.append("\n CoreContactPairs adjust_contactpair_data: \n");
  str_return.append("adjust_contactpair_id, adjust_value \n");

  for (size_t i = 0; i < adjust_contactpair_data.size(); i++)
  {
    str_return.append(adjust_contactpair_data[i][0] + " " + adjust_contactpair_data[i][1] + " \n");
  }
 
  return str_return;
}