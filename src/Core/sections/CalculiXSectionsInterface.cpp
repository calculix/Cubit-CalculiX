#include "CalculiXSectionsInterface.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::string CalculiXSectionsInterface::get_orientation_name(int orientation_id)
{
  return ccx_core.get_orientation_name(orientation_id);
}

std::vector<int> CalculiXSectionsInterface::get_orientations_ids()
{
  return ccx_core.get_orientations_ids();
}

bool CalculiXSectionsInterface::check_orientation_exists(int orientation_id)
{
  return ccx_core.check_orientation_exists(orientation_id);
}

std::string CalculiXSectionsInterface::get_section_export_data(int topology_id,int iteration_id) // gets the export data from sections core
{
  return ccx_core.get_section_export_data(topology_id,iteration_id);
}

std::string CalculiXSectionsInterface::get_orientation_export_data() // gets the export data from orientation core
{
  return ccx_core.get_orientation_export_data();
}

bool CalculiXSectionsInterface::create_section(std::string section_type,int block_id, int material_id, std::vector<std::string> options)
{
  return ccx_core.create_section(section_type,block_id,material_id,options);
}

bool CalculiXSectionsInterface::modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.modify_section(section_type,section_id,options, options_marker);
}

bool CalculiXSectionsInterface::delete_section(int section_id)
{
  return ccx_core.delete_section(section_id);
}

bool CalculiXSectionsInterface::create_orientation(std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.create_orientation(options, options2);
}

bool CalculiXSectionsInterface::modify_orientation(int orientation_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.modify_orientation(orientation_id, options, options_marker, options2);
}

bool CalculiXSectionsInterface::delete_orientation(int orientation_id)
{
  return ccx_core.delete_orientation(orientation_id);
}

std::vector<std::vector<std::string>> CalculiXSectionsInterface::get_sections_tree_data()
{ 
  return ccx_core.get_sections_tree_data();
}

std::vector<std::vector<std::string>> CalculiXSectionsInterface::get_orientations_tree_data()
{ 
  return ccx_core.get_orientations_tree_data();
}
