#include "Core/CalculiXCore.hpp"
#include "CoreSections.hpp"
#include "CoreOrientations.hpp"
#include "CoreSectionsTreeData.hpp"

bool CalculiXCore::create_section(std::string section_type, int block_id, int material_id, std::vector<std::string> options)
{
  return sections->create_section(section_type, block_id, material_id, options);
}

bool CalculiXCore::modify_section(std::string section_type, int section_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return sections->modify_section(section_type, section_id, options, options_marker);
}

bool CalculiXCore::delete_section(int section_id)
{
  return sections->delete_section(section_id);
}

std::string CalculiXCore::get_section_export_data(int topology_id, int iteration_id)
{
  return sections->get_section_export(topology_id, iteration_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_sections_tree_data()
{
  return CoreSectionsTreeData::get_sections_tree_data(*sections);
}

std::string CalculiXCore::get_orientation_name(int orientation_id)
{
  return orientations->get_orientation_name(orientation_id);
}

std::vector<int> CalculiXCore::get_orientations_ids()
{
  return orientations->get_orientations_ids();
}

bool CalculiXCore::check_orientation_exists(int orientation_id)
{
  return orientations->check_orientation_exists(orientation_id);
}

bool CalculiXCore::create_orientation(std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return orientations->create_orientation(options, options2);
}

bool CalculiXCore::modify_orientation(int orientation_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return orientations->modify_orientation(orientation_id, options, options_marker, options2);
}

bool CalculiXCore::delete_orientation(int orientation_id)
{
  return orientations->delete_orientation(orientation_id);
}

std::string CalculiXCore::get_orientation_export_data()
{
  return orientations->get_orientation_export();
}

std::vector<std::vector<std::string>> CalculiXCore::get_orientations_tree_data()
{
  return CoreSectionsTreeData::get_orientations_tree_data(*orientations);
}
