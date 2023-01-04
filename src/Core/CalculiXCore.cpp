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
#include "CoreConstraints.hpp"
#include "CoreReferencePoints.hpp"


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
  if(constraints)
    delete constraints;
  if(referencepoints)
    delete referencepoints;
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

  if(!constraints)
    constraints = new CoreConstraints;
  
  constraints->init();

  if(!referencepoints)
    referencepoints = new CoreReferencePoints;
  
  constraints->init();

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
  //mat->reset();
  sections->reset();
  constraints->reset();
  referencepoints->reset();
  
  //print_to_log("RESET");
  //print_to_log(print_data());

  return true;
}

std::string CalculiXCore::print_data()
{
  std::string str_return = "";
  str_return.append(cb->print_data());
  str_return.append(sections->print_data());
  str_return.append(constraints->print_data());
  str_return.append(referencepoints->print_data());

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

std::string CalculiXCore::get_nodeset_name(int nodeset_id)
{
  std::string nodeset_name;

  NodesetHandle nodeset;
  if (!me_iface->get_nodeset_handle(nodeset_id, nodeset))
  {
    nodeset_name = "NODESET " + std::to_string(nodeset_id) + " doesn't exist!";
    return nodeset_name;
  }
  nodeset_name = me_iface->get_nodeset_name(nodeset);

  // nodesetname or id
  if (nodeset_name == "")
  {
    nodeset_name = "Nodeset_" + std::to_string(nodeset_id);
  }
  return nodeset_name;
}

std::string CalculiXCore::get_sideset_name(int sideset_id)
{
  std::string sideset_name;

  SidesetHandle sideset;
  if (!me_iface->get_sideset_handle(sideset_id, sideset))
  {
    sideset_name = "SIDESET " + std::to_string(sideset_id) + " doesn't exist!";
    return sideset_name;
  }
  sideset_name = me_iface->get_sideset_name(sideset);

  // sidesetname or id
  if (sideset_name == "")
  {
    sideset_name = "Surface_" + std::to_string(sideset_id);
  }
  return sideset_name;
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

bool CalculiXCore::create_constraint(std::string constraint_type, std::vector<std::string> options)
{
  return constraints->create_constraint(constraint_type, options);
}

bool CalculiXCore::modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return constraints->modify_constraint(constraint_type, constraint_id, options, options_marker);
}

bool CalculiXCore::delete_constraint(int constraint_id)
{
  return constraints->delete_constraint(constraint_id);
}

std::vector<int> CalculiXCore::get_rigidbody_vertex_list()
{
  return constraints->get_rigidbody_vertex_list();
}

bool CalculiXCore::referencepoints_update_on_export(int max_node_id)
{
  return referencepoints->update_on_export(max_node_id);
}

bool CalculiXCore::referencepoints_reset_on_export()
{
  return referencepoints->reset();
}

std::string CalculiXCore::get_referencepoints_export()
{
  return referencepoints->get_referencepoints_export();
}

std::string CalculiXCore::get_referencepoints_export_nodesets()
{
  return referencepoints->get_referencepoints_export_nodesets();
}

int CalculiXCore::referencepoints_get_ref_from_vertex_id(int vertex_id)
{
  return referencepoints->get_ref_from_vertex_id(vertex_id);
}

int CalculiXCore::referencepoints_get_rot_from_vertex_id(int vertex_id)
{
  return referencepoints->get_rot_from_vertex_id(vertex_id);
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

std::string CalculiXCore::get_constraint_export_data() // gets the export data from constraints core
{
  return constraints->get_constraint_export();
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

std::vector<std::vector<std::string>> CalculiXCore::get_constraints_tree_data()
{ 
  std::vector<std::vector<std::string>> constraints_tree_data;
  
  for (size_t i = 0; i < constraints->constraints_data.size(); i++)
  {
    std::vector<std::string> constraints_tree_data_set;
    std::string constraint_name;
    std::string entity_name;
    std::string entity_id;
    std::string vertex;

    int sub_constraint_data_id;

    if (constraints->constraints_data[i][1] == 1)
    {
      sub_constraint_data_id = constraints->get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(constraints->constraints_data[i][2]);
      entity_name = "";
      if (constraints->rigidbody_constraint_data[sub_constraint_data_id][1]=="1")
      {
        entity_name = "Nodeset ";
      } else if (constraints->rigidbody_constraint_data[sub_constraint_data_id][1]=="2")
      {
        entity_name = "Block ";
      }
      
      entity_id = constraints->rigidbody_constraint_data[sub_constraint_data_id][2];
      vertex = constraints->rigidbody_constraint_data[sub_constraint_data_id][3];
      constraint_name = "Rigid Body ("+ entity_name + entity_id + "| Vertex " + vertex + ")";
    } else if (constraints->constraints_data[i][1] == 2)
    {
      sub_constraint_data_id = constraints->get_tie_constraint_data_id_from_tie_constraint_id(constraints->constraints_data[i][2]);
      
      constraint_name = "TIE (" + constraints->tie_constraint_data[sub_constraint_data_id][1] + ")";
    }
    
    constraints_tree_data_set.push_back(std::to_string(constraints->constraints_data[i][0])); //constraint_id
    constraints_tree_data_set.push_back(constraint_name); //constraint_name
    constraints_tree_data.push_back(constraints_tree_data_set);
  }

  return constraints_tree_data;
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
    } else if (parse_type=="constraint")
    {
      for (size_t i = 0; i < constraints->constraints_data.size(); i++)
      {
        all_ids.push_back(constraints->constraints_data[i][0]);
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