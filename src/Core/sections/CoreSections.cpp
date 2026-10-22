#include "Core/sections/CoreSections.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "Core/export/CalculiXExportInterface.hpp"
#include "Core/materials/CalculiXMaterialsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include <iomanip>
#include <map>
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

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
  clear_topology_solid_sections();
  //is_initialized = false;
  //init();
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

std::string CoreSections::get_section_export(int topology_id,int iteration_id)
{
  CalculiXExportInterface ccx_export_iface;
  CalculiXMaterialsInterface ccx_materials_iface;
  CalculiXMeshInterface ccx_mesh_iface;
  CalculiXSectionsInterface ccx_sections_iface;

  std::vector<std::string> sections_export_list;
  sections_export_list.push_back("********************************** S E C T I O N S ****************************");
  std::string str_temp;
  int sub_section_data_id;
  std::set<int> topology_sections;

  if (topology_id != -1 || iteration_id != -1)
  {
    if (topology_id <= 0 || iteration_id < 1)
    {
      PRINT_ERROR("Invalid topology export selection.\n");
      return "";
    }

    const auto binding = topology_solid_sections.find(topology_id);

    if (binding == topology_solid_sections.end() ||
        binding->second.empty())
    {
      PRINT_ERROR("Solid section collection for topology %d not found.\n",
                  topology_id);
      return "";
    }

    for (int section_id : binding->second)
    {
      const int index =
          get_sections_data_id_from_section_id(section_id);

      if (index < 0 || sections_data[index][1] != 1)
      {
        PRINT_ERROR("Topology solid section %d no longer exists.\n",
                    section_id);
        return "";
      }

      topology_sections.insert(section_id);
    }
  }

  // standard section export
  //loop over all sections
  for (size_t i = 0; i < sections_data.size(); i++)
  { 
    // CUSTOMLINE START
    std::vector<std::string> customline = ccx_export_iface.get_customline_data("BEFORE","SECTION",sections_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      sections_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END

    // topology solid section
    const int section_id = sections_data[i][0];
    const bool topology_section = topology_sections.count(section_id) != 0;

    if (topology_section)
    {
      const std::string data = get_topology_solid_section_export(topology_id, iteration_id, section_id);

      if (data.empty())
      {
        return "";
      }

      sections_export_list.push_back(data);
    }

    // SOLID
    if (sections_data[i][1] == 1 && !topology_section)
    {
      sub_section_data_id = get_solid_section_data_id_from_solid_section_id(sections_data[i][2]);

      str_temp = "*SOLID SECTION, MATERIAL=";
      //str_temp.append(CubitInterface::get_material_name(std::stoi(solid_section_data[sub_section_data_id][2])));
      std::string material_name = CubitInterface::get_material_name(std::stoi(solid_section_data[sub_section_data_id][2]));
      material_name = ccx_materials_iface.get_material_prefix(material_name) + material_name;
      str_temp.append(material_name); 
      str_temp.append(", ELSET=");
      str_temp.append(ccx_mesh_iface.get_block_name(std::stoi(solid_section_data[sub_section_data_id][1])));
      
      
      if (ccx_sections_iface.check_orientation_exists(std::stoi(solid_section_data[sub_section_data_id][3])))
      {
        str_temp.append(", ORIENTATION=");
        str_temp.append(ccx_sections_iface.get_orientation_name(std::stoi(solid_section_data[sub_section_data_id][3])));
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
      material_name = ccx_materials_iface.get_material_prefix(material_name) + material_name;
      str_temp.append(material_name);
      str_temp.append(", ELSET=");
      str_temp.append(ccx_mesh_iface.get_block_name(std::stoi(shell_section_data[sub_section_data_id][1])));
      
      if (ccx_sections_iface.check_orientation_exists(std::stoi(shell_section_data[sub_section_data_id][3])))
      {
        str_temp.append(", ORIENTATION=");
        str_temp.append(ccx_sections_iface.get_orientation_name(std::stoi(shell_section_data[sub_section_data_id][3])));
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
      material_name = ccx_materials_iface.get_material_prefix(material_name) + material_name;
      str_temp.append(material_name);
      str_temp.append(", ELSET=");
      str_temp.append(ccx_mesh_iface.get_block_name(std::stoi(beam_section_data[sub_section_data_id][1])));
      
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

      if (ccx_sections_iface.check_orientation_exists(std::stoi(beam_section_data[sub_section_data_id][13])))
      {
        str_temp.append(", ORIENTATION=");
        str_temp.append(ccx_sections_iface.get_orientation_name(std::stoi(beam_section_data[sub_section_data_id][13])));
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
      material_name = ccx_materials_iface.get_material_prefix(material_name) + material_name;
      str_temp.append(material_name);
      str_temp.append(", ELSET=");
      str_temp.append(ccx_mesh_iface.get_block_name(std::stoi(membrane_section_data[sub_section_data_id][1])));
      
      if (ccx_sections_iface.check_orientation_exists(std::stoi(membrane_section_data[sub_section_data_id][3])))
      {
        str_temp.append(", ORIENTATION=");
        str_temp.append(ccx_sections_iface.get_orientation_name(std::stoi(membrane_section_data[sub_section_data_id][3])));
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
    customline = ccx_export_iface.get_customline_data("AFTER","SECTION",sections_data[i][0]);
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

bool CoreSections::initialize_topology_solid_sections(int topology_id,const std::vector<int>& section_ids)
{
  CalculiXMeshInterface ccx_mesh_iface;

  if (!is_initialized || topology_id <= 0 || section_ids.empty())
  {
    PRINT_ERROR("Invalid topology solid section initialization.\n");
    return false;
  }

  if (topology_solid_sections.count(topology_id) != 0)
  {
    PRINT_ERROR("Solid section collection for topology %d already exists.\n", topology_id);
    return false;
  }

  std::set<int> unique_sections;
  std::set<int> unique_elements;

  for (int section_id : section_ids)
  {
    if (!unique_sections.insert(section_id).second)
    {
      PRINT_ERROR("Duplicate topology section %d.\n", section_id);
      return false;
    }

    const int section_index = get_sections_data_id_from_section_id(section_id);

    if (section_index < 0 || sections_data[section_index][1] != 1)
    {
      PRINT_ERROR("Section %d is not an existing solid section.\n", section_id);
      return false;
    }

    const int solid_index = get_solid_section_data_id_from_solid_section_id(sections_data[section_index][2]);

    if (solid_index < 0)
    {
      PRINT_ERROR("Solid section data missing for section %d.\n", section_id);
      return false;
    }

    const int block_id = std::stoi(solid_section_data[solid_index][1]);
    const auto element_ids = ccx_mesh_iface.get_block_element_ids(block_id);

    if (element_ids.empty())
    {
      PRINT_ERROR("Block %d has no elements for topology section %d.\n", block_id, section_id);
      return false;
    }

    for (int element_id : element_ids)
    {
      if (!unique_elements.insert(element_id).second)
      {
        PRINT_ERROR("Element %d belongs to multiple topology sections.\n", element_id);
        return false;
      }
    }
  }

  // Commit only after every section passes validation.
  topology_solid_sections.emplace(topology_id, section_ids);
  return true;
}

std::string CoreSections::get_topology_solid_section_export(int topology_id,int iteration_id,int section_id)
{
  CalculiXMeshInterface ccx_mesh_iface;
  CalculiXSectionsInterface ccx_sections_iface;
  CalculiXTopologyInterface ccx_topology_iface;

  if (!is_initialized)
  {
    PRINT_ERROR("Solid section core is not initialized.\n");
    return "";
  }

  const auto binding = topology_solid_sections.find(topology_id);

  if (binding == topology_solid_sections.end())
  {
    PRINT_ERROR("Solid section collection for topology %d not found.\n", topology_id);
    return "";
  }

  const auto& section_ids = binding->second;

  if (std::find(section_ids.begin(), section_ids.end(), section_id) == section_ids.end())
  {
    PRINT_ERROR("Section %d does not belong to topology %d.\n", section_id, topology_id);
    return "";
  }

  const int section_index = get_sections_data_id_from_section_id(section_id);

  if (section_index < 0 || sections_data[section_index][1] != 1)
  {
    PRINT_ERROR("Section %d is not an existing solid section.\n", section_id);
    return "";
  }

  const int solid_index = get_solid_section_data_id_from_solid_section_id(sections_data[section_index][2]);

  if (solid_index < 0)
  {
    PRINT_ERROR("Solid section data missing for section %d.\n", section_id);
    return "";
  }

  const auto& solid = solid_section_data[solid_index];
  const int block_id = std::stoi(solid[1]);
  const auto element_ids = ccx_mesh_iface.get_block_element_ids(block_id);

  if (element_ids.empty())
  {
    PRINT_ERROR("Block %d contains no elements.\n", block_id);
    return "";
  }

  std::string orientation;
  const int orientation_id = std::stoi(solid[3]);

  if (orientation_id != -1)
  {
    if (!ccx_sections_iface.check_orientation_exists(orientation_id))
    {
      PRINT_ERROR("Orientation %d for section %d does not exist.\n", orientation_id, section_id);
      return "";
    }

    orientation = ", ORIENTATION=" + ccx_sections_iface.get_orientation_name(orientation_id);
  }

  std::map<std::string, std::vector<int>> grouped_elements;

  for (int element_id : element_ids)
  {
    std::string material_name;

    if (!ccx_topology_iface.get_topology_group_material_name(topology_id, iteration_id, element_id, material_name))
    {
      PRINT_ERROR("Missing topology material group for element %d.\n", element_id);
      return "";
    }

    grouped_elements[material_name].push_back(element_id);
  }

  std::ostringstream output;

  for (auto& entry : grouped_elements)
  {
    auto& members = entry.second;
    std::sort(members.begin(), members.end());
    std::ostringstream set_name;
    set_name << entry.first << "_S" << std::setfill('0') << std::setw(10) << section_id << "_SET";
    const std::string elset = set_name.str();

    output << "*ELSET, ELSET=" << elset << "\n";

    for (std::size_t i = 0; i < members.size(); ++i)
    {
      output << members[i];

      if ((i + 1) % 8 == 0 || i + 1 == members.size())
      {
        output << "\n";
      }
      else
      {
        output << ", ";
      }
    }

    // Keep orientation and section data separate for each original section.
    output << "*SOLID SECTION, ELSET=" << elset << ", MATERIAL=" << entry.first << orientation << "\n";

    if (!solid[4].empty())
    {
      output << solid[4] << "\n";
    }
  }

  return output.str();
}

std::string CoreSections::get_topology_solid_section_export_legacy(int topology_id,int iteration_id,int section_id)
{
  CalculiXMeshInterface ccx_mesh_iface;
  CalculiXSectionsInterface ccx_sections_iface;
  CalculiXTopologyInterface ccx_topology_iface;

  if (!is_initialized)
  {
    PRINT_ERROR("Solid section core is not initialized.\n");
    return "";
  }

  const auto binding = topology_solid_sections.find(topology_id);

  if (binding == topology_solid_sections.end())
  {
    PRINT_ERROR("Solid section collection for topology %d not found.\n", topology_id);
    return "";
  }

  const auto& section_ids = binding->second;

  if (std::find(section_ids.begin(), section_ids.end(), section_id) == section_ids.end())
  {
    PRINT_ERROR("Section %d does not belong to topology %d.\n", section_id, topology_id);
    return "";
  }

  const int section_index = get_sections_data_id_from_section_id(section_id);

  if (section_index < 0 || sections_data[section_index][1] != 1)
  {
    PRINT_ERROR("Section %d is not an existing solid section.\n", section_id);
    return "";
  }

  const int solid_index = get_solid_section_data_id_from_solid_section_id(sections_data[section_index][2]);

  if (solid_index < 0)
  {
    PRINT_ERROR("Solid section data missing for section %d.\n", section_id);
    return "";
  }

  const auto& solid = solid_section_data[solid_index];
  const int block_id = std::stoi(solid[1]);
  const auto element_ids = ccx_mesh_iface.get_block_element_ids(block_id);

  if (element_ids.empty())
  {
    PRINT_ERROR("Block %d contains no elements.\n", block_id);
    return "";
  }

  std::string orientation;
  const int orientation_id = std::stoi(solid[3]);

  if (orientation_id != -1)
  {
    if (!ccx_sections_iface.check_orientation_exists(orientation_id))
    {
      PRINT_ERROR("Orientation %d for section %d does not exist.\n", orientation_id, section_id);
      return "";
    }

    orientation = ", ORIENTATION=" + ccx_sections_iface.get_orientation_name(orientation_id);
  }

  std::ostringstream output;

  for (int element_id : element_ids)
  {
    if (!ccx_topology_iface.check_topology_material_assignment(topology_id, iteration_id, element_id))
    {
      PRINT_ERROR(
          "Material assignment missing for topology %d, iteration %d, element %d.\n", topology_id, iteration_id, element_id);
      return "";
    }

    const std::string material_name = ccx_topology_iface.get_topology_material_name(topology_id, element_id);

    if (material_name.empty())
    {
      PRINT_ERROR("Topology material name missing for element %d.\n", element_id);
      return "";
    }

    const std::string elset = material_name + "_SET";

    output << "*ELSET, ELSET=" << elset << "\n"
           << element_id << "\n"
           << "*SOLID SECTION, ELSET=" << elset
           << ", MATERIAL=" << material_name
           << orientation << "\n";

    if (!solid[4].empty())
    {
      output << solid[4] << "\n";
    }
  }

  return output.str();
}

void CoreSections::clear_topology_solid_sections()
{
  topology_solid_sections.clear();
}