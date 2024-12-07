#include "CoreSections.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreSections::CoreSections()
{}

CoreSections::~CoreSections()
{}

bool CoreSections::init()
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

bool CoreSections::update()
{ 
  return true;
}

bool CoreSections::reset()
{
  sections_data.clear();
  solid_section_data.clear();
  shell_section_data.clear();
  beam_section_data.clear();
  membrane_section_data.clear();
  init();
  return true;
}

bool CoreSections::check_initialized()
{
  return is_initialized;
}

bool CoreSections::create_section(std::string section_type,int block_id, int material_id, std::vector<std::string> options)
{
  int section_id;
  int section_type_value;
  int section_last;
  int sub_section_id;
  int sub_section_last;

  if (section_type=="SOLID")
  {
    if (solid_section_data.size()==0)
    {
      sub_section_id = 1;
    }
    else
    {
      sub_section_last = int(solid_section_data.size()) - 1;
      sub_section_id = std::stoi(solid_section_data[sub_section_last][0]) + 1;
    }
    section_type_value = 1;
    this->add_solid_section(std::to_string(sub_section_id), std::to_string(block_id), std::to_string(material_id), options[0], options[1]);
  } else if (section_type=="SHELL")
  {
    if (shell_section_data.size()==0)
    {
      sub_section_id = 1;
    }
    else
    {
      sub_section_last = int(shell_section_data.size()) - 1;
      sub_section_id = std::stoi(shell_section_data[sub_section_last][0]) + 1;
    }
    section_type_value = 2;
    this->add_shell_section(std::to_string(sub_section_id), std::to_string(block_id), std::to_string(material_id), options[0], options[1], options[2]);
  } else if (section_type=="BEAM")
  {
    if (beam_section_data.size()==0)
    {
      sub_section_id = 1;
    }
    else
    {
      sub_section_last = int(beam_section_data.size()) - 1;
      sub_section_id = std::stoi(beam_section_data[sub_section_last][0]) + 1;
    }
    section_type_value = 3;
    this->add_beam_section(std::to_string(sub_section_id), std::to_string(block_id), std::to_string(material_id), options[0], options[1], options[2], options[3], options[4], options[5], options[6], options[7], options[8], options[9], options[10], options[11], options[12]);
  }else if (section_type=="MEMBRANE")
  {
    if (membrane_section_data.size()==0)
    {
      sub_section_id = 1;
    }
    else
    {
      sub_section_last = int(membrane_section_data.size()) - 1;
      sub_section_id = std::stoi(membrane_section_data[sub_section_last][0]) + 1;
    }
    section_type_value = 4;
    this->add_membrane_section(std::to_string(sub_section_id), std::to_string(block_id), std::to_string(material_id), options[0], options[1], options[2]);
  }
  
  if (sections_data.size()==0)
  {
    section_id = 1;
  }
  else
  {
    section_last = int(sections_data.size()) - 1;
    section_id = sections_data[section_last][0] + 1;
  }

  this->add_section(section_id,section_type_value,sub_section_id);
  return true;
}

bool CoreSections::modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  int section_type_value;
  if (section_type=="SOLID")
  {
    section_type_value = 1;
  }else if (section_type=="SHELL")
  {
    section_type_value = 2;
  }else if (section_type=="BEAM")
  {
    section_type_value = 3;
  }else if (section_type=="MEMBRANE")
  {
    section_type_value = 4;
  }

  int sub_section_data_id;
  int sections_data_id = get_sections_data_id_from_section_id(section_id);
  
  if (sections_data_id == -1)
  {
    return false;
  } else {
    if ((sections_data[sections_data_id][1]==1) && (sections_data[sections_data_id][1]==section_type_value))
    {
      sub_section_data_id = get_solid_section_data_id_from_solid_section_id(sections_data[sections_data_id][2]);

      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          solid_section_data[sub_section_data_id][i+1] = options[i];
        }
      }
    }else if ((sections_data[sections_data_id][1]==2) && (sections_data[sections_data_id][1]==section_type_value))
    {
      sub_section_data_id = get_shell_section_data_id_from_shell_section_id(sections_data[sections_data_id][2]);

      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          shell_section_data[sub_section_data_id][i+1] = options[i];
        }
      }
    }else if ((sections_data[sections_data_id][1]==3) && (sections_data[sections_data_id][1]==section_type_value))
    {
      sub_section_data_id = get_beam_section_data_id_from_beam_section_id(sections_data[sections_data_id][2]);

      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          beam_section_data[sub_section_data_id][i+1] = options[i];
        }
      }
    }else if ((sections_data[sections_data_id][1]==4) && (sections_data[sections_data_id][1]==section_type_value))
    {
      sub_section_data_id = get_membrane_section_data_id_from_membrane_section_id(sections_data[sections_data_id][2]);

      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          membrane_section_data[sub_section_data_id][i+1] = options[i];
        }
      }
    }
    return true;
  }
}

bool CoreSections::add_section(int section_id, int section_type, int section_type_id)
{
  std::vector<int> v = {section_id, section_type, section_type_id};
      
  sections_data.push_back(v);

  return true;
}

bool CoreSections::add_solid_section(std::string solid_section_id, std::string block_id, std::string material_id,std::string orientation,std::string thickness)
{
  std::vector<std::string> v = {solid_section_id, block_id, material_id, orientation, thickness};
      
  solid_section_data.push_back(v);

  return true;
}

bool CoreSections::add_shell_section(std::string shell_section_id, std::string block_id, std::string material_id,std::string orientation,std::string thickness,std::string offset)
{
  std::vector<std::string> v = {shell_section_id, block_id, material_id, orientation, thickness, offset};
      
  shell_section_data.push_back(v);

  return true;
}

bool CoreSections::add_beam_section(std::string beam_section_id, std::string block_id, std::string material_id,std::string beam_type,std::string parameter1,std::string parameter2,std::string parameter3,std::string parameter4,std::string parameter5,std::string parameter6,std::string x,std::string y,std::string z,std::string orientation,std::string offset1,std::string offset2)
{
  std::vector<std::string> v = {beam_section_id, block_id, material_id, beam_type, parameter1, parameter2, parameter3, parameter4, parameter5, parameter6, x, y, z, orientation, offset1, offset2};
      
  beam_section_data.push_back(v);

  return true;
}

bool CoreSections::add_membrane_section(std::string membrane_section_id, std::string block_id, std::string material_id,std::string orientation,std::string thickness,std::string offset)
{
  std::vector<std::string> v = {membrane_section_id, block_id, material_id, orientation, thickness, offset};
      
  membrane_section_data.push_back(v);

  return true;
}

bool CoreSections::delete_section(int section_id)
{
  int sub_section_data_id;
  int sections_data_id = get_sections_data_id_from_section_id(section_id);
  if (sections_data_id == -1)
  {
    return false;
  } else {
    if (sections_data[sections_data_id][1]==1)
    {
      sub_section_data_id = get_solid_section_data_id_from_solid_section_id(sections_data[sections_data_id][2]);
      solid_section_data.erase(solid_section_data.begin() + sub_section_data_id);  
    }else if (sections_data[sections_data_id][1]==2)
    {
      sub_section_data_id = get_shell_section_data_id_from_shell_section_id(sections_data[sections_data_id][2]);
      shell_section_data.erase(shell_section_data.begin() + sub_section_data_id);  
    }else if (sections_data[sections_data_id][1]==3)
    {
      sub_section_data_id = get_beam_section_data_id_from_beam_section_id(sections_data[sections_data_id][2]);
      beam_section_data.erase(beam_section_data.begin() + sub_section_data_id);  
    }else if (sections_data[sections_data_id][1]==4)
    {
      sub_section_data_id = get_membrane_section_data_id_from_membrane_section_id(sections_data[sections_data_id][2]);
      membrane_section_data.erase(membrane_section_data.begin() + sub_section_data_id);  
    }
    sections_data.erase(sections_data.begin() + sections_data_id);
    return true;
  }
}
bool CoreSections::modify_section_id(int section_id, int new_section_id)
{
  int sections_data_id = get_sections_data_id_from_section_id(section_id);
  if (sections_data_id == -1)
  {
    return false;
  } else {
    sections_data[sections_data_id][0]=new_section_id;
    return true;
  }
}

int CoreSections::get_sections_data_id_from_section_id(int section_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < sections_data.size(); i++)
  {
    if (sections_data[i][0]==section_id)
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreSections::get_solid_section_data_id_from_solid_section_id(int solid_section_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < solid_section_data.size(); i++)
  {
    if (solid_section_data[i][0]==std::to_string(solid_section_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}


int CoreSections::get_shell_section_data_id_from_shell_section_id(int shell_section_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < shell_section_data.size(); i++)
  {
    if (shell_section_data[i][0]==std::to_string(shell_section_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}


int CoreSections::get_beam_section_data_id_from_beam_section_id(int beam_section_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < beam_section_data.size(); i++)
  {
    if (beam_section_data[i][0]==std::to_string(beam_section_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}


int CoreSections::get_membrane_section_data_id_from_membrane_section_id(int membrane_section_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < membrane_section_data.size(); i++)
  {
    if (membrane_section_data[i][0]==std::to_string(membrane_section_id))
    {
        return_int = int(i);
    }  
  }
  return return_int;
}

std::string CoreSections::get_section_export() // get a list of the CalculiX section exports
{
  std::vector<std::string> sections_export_list;
  sections_export_list.push_back("********************************** S E C T I O N S ****************************");
  std::string str_temp;
  int sub_section_data_id;

  //loop over all sections
  for (size_t i = 0; i < sections_data.size(); i++)
  { 
    // CUSTOMLINE START
    std::vector<std::string> customline = ccx_iface->get_customline_data("BEFORE","SECTION",sections_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      sections_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END

    // SOLID
    if (sections_data[i][1] == 1)
    {
      sub_section_data_id = get_solid_section_data_id_from_solid_section_id(sections_data[i][2]);

      str_temp = "*SOLID SECTION, MATERIAL=";
      //str_temp.append(CubitInterface::get_material_name(std::stoi(solid_section_data[sub_section_data_id][2])));
      std::string material_name = CubitInterface::get_material_name(std::stoi(solid_section_data[sub_section_data_id][2]));
      material_name = ccx_iface->get_material_prefix(material_name) + material_name;
      str_temp.append(material_name); 
      str_temp.append(", ELSET=");
      str_temp.append(ccx_iface->get_block_name(std::stoi(solid_section_data[sub_section_data_id][1])));
      
      
      if (ccx_iface->check_orientation_exists(std::stoi(solid_section_data[sub_section_data_id][3])))
      {
        str_temp.append(", ORIENTATION=");
        str_temp.append(ccx_iface->get_orientation_name(std::stoi(solid_section_data[sub_section_data_id][3])));
      }

      sections_export_list.push_back(str_temp);

      if (solid_section_data[sub_section_data_id][4]!="")
      { 
        sections_export_list.push_back(solid_section_data[sub_section_data_id][4]);
      }    
    }
    // SHELL
    if (sections_data[i][1] == 2) 
    {
      sub_section_data_id = get_shell_section_data_id_from_shell_section_id(sections_data[i][2]);

      str_temp = "*SHELL SECTION, MATERIAL=";
      //str_temp.append(CubitInterface::get_material_name(std::stoi(shell_section_data[sub_section_data_id][2])));
      std::string material_name = CubitInterface::get_material_name(std::stoi(shell_section_data[sub_section_data_id][2]));
      material_name = ccx_iface->get_material_prefix(material_name) + material_name;
      str_temp.append(material_name);
      str_temp.append(", ELSET=");
      str_temp.append(ccx_iface->get_block_name(std::stoi(shell_section_data[sub_section_data_id][1])));
      
      if (ccx_iface->check_orientation_exists(std::stoi(shell_section_data[sub_section_data_id][3])))
      {
        str_temp.append(", ORIENTATION=");
        str_temp.append(ccx_iface->get_orientation_name(std::stoi(shell_section_data[sub_section_data_id][3])));
      }

      if (shell_section_data[sub_section_data_id][5]!="")
      {
        str_temp.append(", OFFSET=");
        str_temp.append(shell_section_data[sub_section_data_id][5]);
      }

      sections_export_list.push_back(str_temp);

      if (shell_section_data[sub_section_data_id][4]!="")
      { 
        sections_export_list.push_back(shell_section_data[sub_section_data_id][4]);
      }    
    }
    // BEAM
    if (sections_data[i][1] == 3) 
    {
      sub_section_data_id = get_beam_section_data_id_from_beam_section_id(sections_data[i][2]);

      str_temp = "*BEAM SECTION, MATERIAL=";
      //str_temp.append(CubitInterface::get_material_name(std::stoi(beam_section_data[sub_section_data_id][2])));
      std::string material_name = CubitInterface::get_material_name(std::stoi(beam_section_data[sub_section_data_id][2]));
      material_name = ccx_iface->get_material_prefix(material_name) + material_name;
      str_temp.append(material_name);
      str_temp.append(", ELSET=");
      str_temp.append(ccx_iface->get_block_name(std::stoi(beam_section_data[sub_section_data_id][1])));
      
      if (beam_section_data[sub_section_data_id][3]!="")
      {
        str_temp.append(", SECTION=");
        str_temp.append(beam_section_data[sub_section_data_id][3]);
      }
      
      if (beam_section_data[sub_section_data_id][14]!="")
      {
        str_temp.append(", OFFSET1=");
        str_temp.append(beam_section_data[sub_section_data_id][14]);
      }

      if (beam_section_data[sub_section_data_id][15]!="")
      {
        str_temp.append(", OFFSET2=");
        str_temp.append(beam_section_data[sub_section_data_id][15]);
      }

      if (ccx_iface->check_orientation_exists(std::stoi(beam_section_data[sub_section_data_id][13])))
      {
        str_temp.append(", ORIENTATION=");
        str_temp.append(ccx_iface->get_orientation_name(std::stoi(beam_section_data[sub_section_data_id][13])));
      }

      sections_export_list.push_back(str_temp);
      if (beam_section_data[sub_section_data_id][3]=="BOX")
      {
        sections_export_list.push_back(beam_section_data[sub_section_data_id][4] + ", " + beam_section_data[sub_section_data_id][5] + ", " + beam_section_data[sub_section_data_id][6] + ", " + beam_section_data[sub_section_data_id][7] + ", " + beam_section_data[sub_section_data_id][8] + ", " + beam_section_data[sub_section_data_id][9]);
      }else{
        sections_export_list.push_back(beam_section_data[sub_section_data_id][4] + ", " + beam_section_data[sub_section_data_id][5]);
      }
      sections_export_list.push_back(beam_section_data[sub_section_data_id][10] + ", " + beam_section_data[sub_section_data_id][11] + ", " + beam_section_data[sub_section_data_id][12]);
    }
    // MEMBRANE
    if (sections_data[i][1] == 4) 
    {
      sub_section_data_id = get_membrane_section_data_id_from_membrane_section_id(sections_data[i][2]);

      str_temp = "*MEMBRANE SECTION, MATERIAL=";
      //str_temp.append(CubitInterface::get_material_name(std::stoi(membrane_section_data[sub_section_data_id][2])));
      std::string material_name = CubitInterface::get_material_name(std::stoi(membrane_section_data[sub_section_data_id][2]));
      material_name = ccx_iface->get_material_prefix(material_name) + material_name;
      str_temp.append(material_name);
      str_temp.append(", ELSET=");
      str_temp.append(ccx_iface->get_block_name(std::stoi(membrane_section_data[sub_section_data_id][1])));
      
      if (ccx_iface->check_orientation_exists(std::stoi(membrane_section_data[sub_section_data_id][3])))
      {
        str_temp.append(", ORIENTATION=");
        str_temp.append(ccx_iface->get_orientation_name(std::stoi(membrane_section_data[sub_section_data_id][3])));
      }

      if (membrane_section_data[sub_section_data_id][5]!="")
      {
        str_temp.append(", OFFSET=");
        str_temp.append(membrane_section_data[sub_section_data_id][5]);
      }

      sections_export_list.push_back(str_temp);

      if (membrane_section_data[sub_section_data_id][4]!="")
      { 
        sections_export_list.push_back(membrane_section_data[sub_section_data_id][4]);
      }    
    }
    // CUSTOMLINE START
    customline = ccx_iface->get_customline_data("AFTER","SECTION",sections_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      sections_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END
  }

  std::string section_export;

  for (size_t i = 0; i < sections_export_list.size(); i++)
  {
    section_export.append(sections_export_list[i] + "\n");
  }
  
  return section_export;
}

std::string CoreSections::print_data()
{
  std::string str_return;
  str_return = "\n CoreSections sections_data: \n";
  str_return.append("section_id, section_type, section_type_id \n");

  for (size_t i = 0; i < sections_data.size(); i++)
  {
    str_return.append(std::to_string(sections_data[i][0]) + " " + std::to_string(sections_data[i][1]) + " " + std::to_string(sections_data[i][2]) + " \n");
  }

  str_return.append("\n CoreSections solid_section_data: \n");
  str_return.append("solid_section_id,block_id,material,orientation,thickness \n");

  for (size_t i = 0; i < solid_section_data.size(); i++)
  {
    str_return.append(solid_section_data[i][0] + " " + solid_section_data[i][1] + " " + solid_section_data[i][2] + " " + solid_section_data[i][3] + " " + solid_section_data[i][4] + " \n");
  }
  
  str_return.append("\n CoreSections shell_section_data: \n");
  str_return.append("shell_section_id,block_id,material,orientation,thickness,offset \n");

  for (size_t i = 0; i < shell_section_data.size(); i++)
  {
    str_return.append(shell_section_data[i][0] + " " + shell_section_data[i][1] + " " + shell_section_data[i][2] + " " + shell_section_data[i][3] + " " + shell_section_data[i][4] + " " + shell_section_data[i][5] + " \n");
  }

  str_return.append("\n CoreSections beam_section_data: \n");
  str_return.append("beam_section_id,block_id,material,beam_type,parameter1,parameter2,parameter3,parameter4,parameter5,parameter6,x,y,z,orientation,offset1,offset2 \n");

  for (size_t i = 0; i < beam_section_data.size(); i++)
  {
    str_return.append(beam_section_data[i][0] + " " + beam_section_data[i][1] + " " + beam_section_data[i][2] + " " + beam_section_data[i][3] + " " + beam_section_data[i][4] + " " + beam_section_data[i][5] + " " + beam_section_data[i][6] + " " + beam_section_data[i][7] + " " + beam_section_data[i][8] + " " + beam_section_data[i][9] + " " + beam_section_data[i][10] + " " + beam_section_data[i][11] + " " + beam_section_data[i][1] + " " + beam_section_data[i][12] + " " + beam_section_data[i][13] + " " + beam_section_data[i][14] + " " + beam_section_data[i][15] + " \n");
  }

  str_return.append("\n CoreSections membrane_section_data: \n");
  str_return.append("membrane_section_id,block_id,material,orientation,thickness,offset \n");

  for (size_t i = 0; i < membrane_section_data.size(); i++)
  {
    str_return.append(membrane_section_data[i][0] + " " + membrane_section_data[i][1] + " " + membrane_section_data[i][2] + " " + membrane_section_data[i][3] + " " + membrane_section_data[i][4] + " " + membrane_section_data[i][5] + " \n");
  }

  return str_return;
}