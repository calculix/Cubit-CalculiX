#include "CalculiXCore.hpp"
#include <fstream>
#include <chrono>
#include <ctime>
#include "CubitInterface.hpp"
#include "MeshExportInterface.hpp"


#include "CoreBlocks.hpp"

CalculiXCore::CalculiXCore():
  cb(NULL)
{
  print_to_log("CalculiXCore Initialization!");
  init();
}

CalculiXCore::~CalculiXCore()
{
  if(cb)
    delete cb;
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
  
  if(!cb)
    cb = new CoreBlocks;
  
  cb->init();
  
  return true;
}

bool CalculiXCore::update()
{
  cb->update();
  
  print_to_log("UPDATE");
  print_to_log(print_data());

  return true;
}

bool CalculiXCore::reset()
{
  cb->reset();
  
  //print_to_log("RESET");
  //print_to_log(print_data());

  return true;
}

std::string CalculiXCore::print_data()
{
  std::string str_return = "";
  str_return.append(cb->print_data());

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
  me_iface->get_block_handle(block_id, block);
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