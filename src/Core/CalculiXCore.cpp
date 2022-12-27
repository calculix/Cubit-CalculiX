#include "CalculiXCore.hpp"
#include <fstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include "CubitInterface.hpp"
#include "MeshExportInterface.hpp"
#include "MaterialInterface.hpp"

#include "CoreBlocks.hpp"
#include "CoreMaterials.hpp"
#include "CoreSections.hpp"


CalculiXCore::CalculiXCore():
  cb(NULL),mat(NULL)
{
  print_to_log("CalculiXCore Initialization!");
  init();
}

CalculiXCore::~CalculiXCore()
{
  if(cb)
    delete cb;
  if(mat)
    delete mat;
  if(sections)
    delete sections;
}

bool CalculiXCore::print_to_log(std::string str_log)
{
  std::ofstream output_file;
  output_file.open("CalculiXCore_logfile", std::ios_base::app);
  auto timestamp = std::chrono::system_clock::now();
  std::time_t time_t_timestamp = std::chrono::system_clock::to_time_t(timestamp);
  output_file << std::ctime(&time_t_timestamp) + str_log + "\n";
  output_file.close();
  return true;
}

bool CalculiXCore::init()
{
  me_iface = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));
  mat_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));

  if(!cb)
    cb = new CoreBlocks;
  
  cb->init();

  if(!mat)
    mat = new CoreMaterials;
  
  mat->init();
  
  if(!sections)
    sections = new CoreSections;
  
  sections->init();

  return true;
}

bool CalculiXCore::update()
{
  cb->update();
  //mat->update();
  
  print_to_log("UPDATE");
  print_to_log(print_data());

  return true;
}

bool CalculiXCore::reset()
{
  cb->reset();
  sections->reset();
  //mat->reset();
  
  //print_to_log("RESET");
  //print_to_log(print_data());

  return true;
}

std::string CalculiXCore::print_data()
{
  std::string str_return = "";
  str_return.append(cb->print_data());
  str_return.append(sections->print_data());

  return str_return;
}

std::vector<std::string> CalculiXCore::get_ccx_element_types()
{ 
  return cb->get_ccx_element_types();
}

bool CalculiXCore::set_ccx_element_type(int block_id, std::string ccx_element_type)
{ 
  int new_element_type_ccx;
  new_element_type_ccx = cb->get_ccx_element_type_id(ccx_element_type);

  if (cb->modify_block_element_type_ccx(block_id, new_element_type_ccx))
  {
    return true;
  } else {
    return false;
  }
}

std::string CalculiXCore::get_ccx_element_type(int block_id)
{
  return cb->get_ccx_element_type_name(block_id);
}

std::string CalculiXCore::get_block_name(int block_id)
{
  std::string block_name;

  BlockHandle block;
  if (!me_iface->get_block_handle(block_id, block))
  {
    block_name = "BLOCK " + std::to_string(block_id) + " doesn't exist!";
    return block_name;
  }
  block_name = me_iface->name_from_handle(block);
    
  if (block_name == "")
  {
    block_name = "Block_" + std::to_string(block_id);
  }
  return block_name;
}

std::vector<int> CalculiXCore::get_blocks()
{ 
  std::vector<int> blocks;
  
  for (size_t i = 0; i < cb->blocks_data.size(); i++)
  {
    blocks.push_back(cb->blocks_data[i][0]);
  }
  return blocks;
}

bool CalculiXCore::create_section(std::string section_type,int block_id, std::string material_name, std::vector<std::string> options)
{
  return sections->create_section(section_type, block_id, material_name, options);
}

bool CalculiXCore::modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return sections->modify_section(section_type, section_id, options, options_marker);
}

bool CalculiXCore::delete_section(int section_id)
{
  return sections->delete_section(section_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_blocks_tree_data()
{ 
  std::vector<std::vector<std::string>> blocks_tree_data;
    
  for (size_t i = 0; i < cb->blocks_data.size(); i++)
  {
    std::vector<std::string> blocks_tree_data_set;
    std::string block_name;

    BlockHandle block;
    me_iface->get_block_handle(cb->blocks_data[i][0], block);
    block_name = me_iface->name_from_handle(block);
    
    if (block_name == "")
    {
      block_name = "Block_" + std::to_string(cb->blocks_data[i][0]);
    }
    
    blocks_tree_data_set.push_back(std::to_string(cb->blocks_data[i][0])); //block_id
    blocks_tree_data_set.push_back(block_name); //block_name
    blocks_tree_data_set.push_back(cb->get_ccx_element_type_name(cb->blocks_data[i][0]));//ccx_element_type
    blocks_tree_data.push_back(blocks_tree_data_set);
  }

  return blocks_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_nodeset_tree_data()
{ 
  std::vector<std::vector<std::string>> nodeset_tree_data;

  // Get the list of nodesets
  std::vector<int> nodesets;
  nodesets = CubitInterface::get_nodeset_id_list();
  
  // loop over the nodesets
  for (size_t i = 0; i < nodesets.size(); i++)
  {
    std::vector<std::string> nodeset_tree_data_set;
    std::string nodeset_name;

    NodesetHandle nodeset;
    me_iface->get_nodeset_handle(nodesets[i],nodeset);
    
    nodeset_name = me_iface->get_nodeset_name(nodeset);
    
    if (nodeset_name == "")
    {
      nodeset_name = "Nodeset_" + std::to_string(me_iface->id_from_handle(nodeset));
    }

    nodeset_tree_data_set.push_back(std::to_string(me_iface->id_from_handle(nodeset))); //nodeset_id
    nodeset_tree_data_set.push_back(nodeset_name); //nodeset_name
    nodeset_tree_data.push_back(nodeset_tree_data_set);
  }

  return nodeset_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_sideset_tree_data()
{ 
  std::vector<std::vector<std::string>> sideset_tree_data;

  // Get the list of sidesets
  std::vector<int> sidesets;
  sidesets = CubitInterface::get_sideset_id_list();
  
  // loop over the sidesets
  for (size_t i = 0; i < sidesets.size(); i++)
  {
    std::vector<std::string> sideset_tree_data_set;
    std::string sideset_name;

    SidesetHandle sideset;
    me_iface->get_sideset_handle(sidesets[i],sideset);
    
    sideset_name = me_iface->get_sideset_name(sideset);
    
    if (sideset_name == "")
    {
      sideset_name = "Sideset_" + std::to_string(me_iface->id_from_handle(sideset));
    }

    sideset_tree_data_set.push_back(std::to_string(me_iface->id_from_handle(sideset))); //sideset_id
    sideset_tree_data_set.push_back(sideset_name); //sideset_name
    sideset_tree_data.push_back(sideset_tree_data_set);
  }

  return sideset_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_material_tree_data()
{ 
  std::vector<std::vector<std::string>> material_tree_data;
  // Get the list of materials
  std::vector<std::string> material_name_list;
  material_name_list = CubitInterface::get_material_name_list();
  //loop over all materials
  for (size_t i = 0; i < material_name_list.size(); i++)
  { 
    MaterialInterface::Material material;
    MaterialInterface::PropertyGroup group;
    std::string group_name;
    std::vector<std::string> group_list;
    material = mat_iface->get_material(material_name_list[i]);
    group = mat_iface->get_material_property_group(material);
    group_name = mat_iface->get_group_name(group);
    group_list = mat->get_group_list();

    if (std::find(group_list.begin(), group_list.end(), group_name) != group_list.end())
    {
      std::vector<std::string> material_tree_data_set;
      material_tree_data_set.push_back(std::to_string(mat_iface->get_material_id(material))); //material_id
      material_tree_data_set.push_back(material_name_list[i]); //material_name
      material_tree_data.push_back(material_tree_data_set);
    }
  }
  return material_tree_data;
}


std::string CalculiXCore::get_material_export_data() // gets the export data from materials core
{
  return mat->get_material_export();
}

std::string CalculiXCore::get_section_export_data() // gets the export data from sections core
{
  return sections->get_section_export();
}

std::vector<std::vector<std::string>> CalculiXCore::get_sections_tree_data()
{ 
  std::vector<std::vector<std::string>> sections_tree_data;
  
  for (size_t i = 0; i < sections->sections_data.size(); i++)
  {
    std::vector<std::string> sections_tree_data_set;
    std::string section_name;
    std::string block_name;
    std::string material_name;
    int sub_section_data_id;

    if (sections->sections_data[i][1] == 1)
    {
      sub_section_data_id = sections->get_solid_section_data_id_from_solid_section_id(sections->sections_data[i][2]);
      material_name = sections->solid_section_data[sub_section_data_id][2];
      block_name = this->get_block_name(std::stoi(sections->solid_section_data[sub_section_data_id][1]));
      section_name = "SOLID (" + block_name + "|" + material_name + ")";
    } else if (sections->sections_data[i][1] == 2)
    {
      sub_section_data_id = sections->get_shell_section_data_id_from_shell_section_id(sections->sections_data[i][2]);
      material_name = sections->shell_section_data[sub_section_data_id][2];
      block_name = this->get_block_name(std::stoi(sections->shell_section_data[sub_section_data_id][1]));
      section_name = "SHELL (" + block_name + "|" + material_name + ")";
    } else if (sections->sections_data[i][1] == 3)
    {
      sub_section_data_id = sections->get_beam_section_data_id_from_beam_section_id(sections->sections_data[i][2]);
      material_name = sections->beam_section_data[sub_section_data_id][2];
      block_name = this->get_block_name(std::stoi(sections->beam_section_data[sub_section_data_id][1]));
      section_name = "BEAM (" + block_name + "|" + material_name + ")";
    } else if (sections->sections_data[i][1] == 4)
    {
      sub_section_data_id = sections->get_membrane_section_data_id_from_membrane_section_id(sections->sections_data[i][2]);
      material_name = sections->membrane_section_data[sub_section_data_id][2];
      block_name = this->get_block_name(std::stoi(sections->membrane_section_data[sub_section_data_id][1]));
      section_name = "MEMRBANE (" + block_name + "|" + material_name + ")";
    }
    
    sections_tree_data_set.push_back(std::to_string(sections->sections_data[i][0])); //section_id
    sections_tree_data_set.push_back(section_name); //section_name
    sections_tree_data.push_back(sections_tree_data_set);
  }

  return sections_tree_data;
}

std::vector<int> CalculiXCore::parser(std::string parse_type, std::string parse_string)
{
  std::vector<int> input_ids;
  
  std::vector<int> all_ids;
  std::vector<int> except_ids;
  std::vector<int> to_ids;

  size_t pos_all = parse_string.find("all");
  size_t pos_except = parse_string.find("except");
  size_t pos_to = parse_string.find("to");

  input_ids = this->extractIntegers(parse_string);

  if (pos_all != std::string::npos)
  {
    if (parse_type=="section")
    {
      for (size_t i = 0; i < sections->sections_data.size(); i++)
      {
        all_ids.push_back(sections->sections_data[i][0]);
      }
    }
  }

  if (pos_to != std::string::npos)
  { 
    if ((input_ids[0]<=input_ids[1])&&(input_ids[0]>0))
    {  
      for (size_t i = input_ids[0]; i < input_ids[1]+1; i++)
      {
        to_ids.push_back(i);
      }
    }
  }

  if ((pos_except != std::string::npos) && (pos_to == std::string::npos))
  { 
    except_ids = input_ids;
  } else if ((pos_except != std::string::npos) && (pos_to != std::string::npos))
  {
    except_ids = to_ids;
  }

  if ((pos_all == std::string::npos) && (pos_to != std::string::npos))
  {
    return to_ids;  
  }

  for (size_t i = 0; i < except_ids.size(); i++)
  {
    all_ids.erase(std::remove(all_ids.begin(), all_ids.end(), except_ids[i]), all_ids.end());
  }
  
  return all_ids;
}


std::vector<int> CalculiXCore::extractIntegers(std::string str)
{
    std::stringstream ss;
    std::vector<int> ids;
 
    /* Storing the whole string into string stream */
    ss << str;
 
    /* Running loop till the end of the stream */
    std::string temp;
    int found;
    while (!ss.eof()) {
        /* extracting word by word from stream */
        ss >> temp;
 
        /* Checking the given word is integer or not */
        if (std::stringstream(temp) >> found){
            ids.push_back(found);
        }
        /* To save from space at the end of string */
        temp = "";
    }
    return ids;
}