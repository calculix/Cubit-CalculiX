#include "CalculiXSectionsInterface.hpp"
#include "CoreSectionsTreeData.hpp"
#include "CoreSections.hpp"
#include "CoreOrientations.hpp"
#include <string>
#include <vector>

#include "Core/loadCalculiXCore.hpp"

std::string CalculiXSectionsInterface::get_orientation_name(int orientation_id)
{
  return ccx_core.orientations->get_orientation_name(orientation_id);
}

std::vector<int> CalculiXSectionsInterface::get_orientations_ids()
{
  return ccx_core.orientations->get_orientations_ids();
}

bool CalculiXSectionsInterface::check_orientation_exists(int orientation_id)
{
  return ccx_core.orientations->check_orientation_exists(orientation_id);
}

std::string CalculiXSectionsInterface::get_section_export_data(int topology_id, int iteration_id)
{
  return ccx_core.sections->get_section_export(topology_id, iteration_id);
}

std::string CalculiXSectionsInterface::get_orientation_export_data()
{
  return ccx_core.orientations->get_orientation_export();
}

bool CalculiXSectionsInterface::create_section(std::string section_type, int block_id, int material_id, std::vector<std::string> options)
{
  return ccx_core.sections->create_section(section_type, block_id, material_id, options);
}

bool CalculiXSectionsInterface::modify_section(std::string section_type, int section_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return ccx_core.sections->modify_section(section_type, section_id, options, options_marker);
}

bool CalculiXSectionsInterface::delete_section(int section_id)
{
  return ccx_core.sections->delete_section(section_id);
}

bool CalculiXSectionsInterface::create_orientation(std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.orientations->create_orientation(options, options2);
}

bool CalculiXSectionsInterface::modify_orientation(int orientation_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return ccx_core.orientations->modify_orientation(orientation_id, options, options_marker, options2);
}

bool CalculiXSectionsInterface::delete_orientation(int orientation_id)
{
  return ccx_core.orientations->delete_orientation(orientation_id);
}

std::vector<std::vector<std::string>> CalculiXSectionsInterface::get_sections_tree_data()
{ 
  return CoreSectionsTreeData::get_sections_tree_data(*ccx_core.sections);
}

std::vector<std::vector<std::string>> CalculiXSectionsInterface::get_orientations_tree_data()
{ 
  return CoreSectionsTreeData::get_orientations_tree_data(*ccx_core.orientations);
}


bool CalculiXSectionsInterface::get_section_settings(int id, SectionsSettings& settings, std::string& error)
{
  return ccx_core.sections->get_settings(id, settings, error);
}


bool CalculiXSectionsInterface::get_orientation_settings(int id, OrientationsSettings& settings, std::string& error)
{
  return ccx_core.orientations->get_settings(id, settings, error);
}
