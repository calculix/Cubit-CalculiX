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
  this->add_adjust_contactpair(std::to_string(sub_id), options[4], options[5]);

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
    if (options_marker[5]==1)
    {
      sub_data_id = get_adjust_contactpair_data_id_from_adjust_contactpair_id(contactpairs_data[contactpairs_data_id][5]);
      adjust_contactpair_data[sub_data_id][2] = options[5];
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

bool CoreContactPairs::add_adjust_contactpair(std::string adjust_contactpair_id, std::string adjust_value, std::string adjust_nodeset)
{
  std::vector<std::string> v = {adjust_contactpair_id, adjust_value, adjust_nodeset};
  
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
    sub_data_id = get_adjust_contactpair_data_id_from_adjust_contactpair_id(contactpairs_data[contactpairs_data_id][5]);
    if (sub_data_id != -1){
      adjust_contactpair_data.erase(adjust_contactpair_data.begin() + sub_data_id);  
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
  std::vector<std::string> contactpairs_export_list;
  contactpairs_export_list.push_back("********************************** C O N T A C T P A I R S ****************************");
  std::string str_temp;
  int sub_data_id;
  std::string nodeset_name;
  
  //loop over all contactpairs
  for (size_t i = 0; i < contactpairs_data.size(); i++)
  { 
    // CUSTOMLINE START
    std::vector<std::string> customline = ccx_iface->get_customline_data("BEFORE","CONTACTPAIR",contactpairs_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      contactpairs_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END

    str_temp = "*CONTACT PAIR, INTERACTION=";
    str_temp.append(ccx_iface->get_surfaceinteraction_name(contactpairs_data[i][1]));

    str_temp.append(", TYPE=");
    if (contactpairs_data[i][2]==1)
    {
      str_temp.append("NODE TO SURFACE");
    }else if (contactpairs_data[i][2]==2)
    {
      str_temp.append("SURFACE TO SURFACE");
    }else if (contactpairs_data[i][2]==3)
    {
      str_temp.append("MORTAR");
    }else if (contactpairs_data[i][2]==4)
    {
      str_temp.append("LINMORTAR");
    }else if (contactpairs_data[i][2]==5)
    {
      str_temp.append("PGLINMORTAR");
    }else if (contactpairs_data[i][2]==6)
    {
      str_temp.append("MASSLESS");
    }    
    
    sub_data_id = get_adjust_contactpair_data_id_from_adjust_contactpair_id(contactpairs_data[i][5]);

    if(adjust_contactpair_data[sub_data_id][1]!="")
    {
      str_temp.append(", ADJUST=");
      str_temp.append(adjust_contactpair_data[sub_data_id][1]);
    }else if (adjust_contactpair_data[sub_data_id][2]!="")
    {
      nodeset_name = CubitInterface::get_exodus_entity_name("nodeset",std::stoi(adjust_contactpair_data[sub_data_id][2]));
      if (nodeset_name == "") {
      nodeset_name = "Nodeset_" + adjust_contactpair_data[sub_data_id][2];
      }
      str_temp.append(", ADJUST=");
      str_temp.append(nodeset_name);
    }
    
    
    contactpairs_export_list.push_back(str_temp);

    // second line
    str_temp = "";
    str_temp.append(ccx_iface->get_sideset_name(contactpairs_data[i][4]));
    str_temp.append(",");
    str_temp.append(ccx_iface->get_sideset_name(contactpairs_data[i][3]));
    contactpairs_export_list.push_back(str_temp);

    // CUSTOMLINE START
    customline = ccx_iface->get_customline_data("AFTER","CONTACTPAIR",contactpairs_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      contactpairs_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END
  }
  
  std::string contactpair_export;

  for (size_t i = 0; i < contactpairs_export_list.size(); i++)
  {
    contactpair_export.append(contactpairs_export_list[i] + "\n");
  }
  
  return contactpair_export;
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
  str_return.append("adjust_contactpair_id, adjust_value, adjust_nodeset \n");

  for (size_t i = 0; i < adjust_contactpair_data.size(); i++)
  {
    str_return.append(adjust_contactpair_data[i][0] + " " + adjust_contactpair_data[i][1] + " " + adjust_contactpair_data[i][2] + " \n");
  }
 
  return str_return;
}