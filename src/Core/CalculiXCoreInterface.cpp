#include "CalculiXCoreInterface.hpp"
#include "loadCalculiXCore.hpp"

CalculiXCoreInterface::CalculiXCoreInterface()
{}

CalculiXCoreInterface::~CalculiXCoreInterface()
{}

std::string CalculiXCoreInterface::print_data()
{
  return ccx_core.print_data();
}

bool CalculiXCoreInterface::log_str(std::string str_log)
{
  ccx_core.print_to_log(str_log);
  return true;
}

std::vector<int> CalculiXCoreInterface::parser(std::string parse_type, std::string parse_string)
{
  return ccx_core.parser(parse_type, parse_string);
}

std::vector<std::string> CalculiXCoreInterface::get_ccx_element_types()
{ 
  return ccx_core.get_ccx_element_types();
}

bool CalculiXCoreInterface::set_ccx_element_type(int block_id, std::string ccx_element_type)
{
  if (ccx_core.set_ccx_element_type(block_id, ccx_element_type))
  {
    return true;
  }else{
    return false;
  }
}

std::string CalculiXCoreInterface::get_ccx_element_type(int block_id)
{
  return ccx_core.get_ccx_element_type(block_id);
}

std::string CalculiXCoreInterface::get_block_name(int block_id)
{
  return ccx_core.get_block_name(block_id);
}

bool CalculiXCoreInterface::core_update()
{
  if (ccx_core.update())
  {
    return true;
  }else{
    return false;
  }
}

bool CalculiXCoreInterface::core_reset()
{
  if (ccx_core.reset())
  {
    return true;
  }else{
    return false;
  }
}

std::vector<int> CalculiXCoreInterface::get_blocks()
{ 
  return ccx_core.get_blocks();
}

bool CalculiXCoreInterface::create_section(std::string section_type,int block_id, std::string material_name, std::vector<std::string> options)
{
  return ccx_core.create_section(section_type,block_id,material_name,options);
}

bool CalculiXCoreInterface::modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_section(section_type,section_id,options, options_marker);
}

bool CalculiXCoreInterface::delete_section(int section_id)
{
  return ccx_core.delete_section(section_id);
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_blocks_tree_data()
{ 
  return ccx_core.get_blocks_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_nodeset_tree_data()
{ 
  return ccx_core.get_nodeset_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_sideset_tree_data()
{ 
  return ccx_core.get_sideset_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCoreInterface::get_material_tree_data()
{ 
  return ccx_core.get_material_tree_data();
}

std::string CalculiXCoreInterface::get_material_export_data() // gets the export data from materials core
{
  return ccx_core.get_material_export_data();
}

std::string CalculiXCoreInterface::get_section_export_data() // gets the export data from sections core
{
  return ccx_core.get_section_export_data();
}