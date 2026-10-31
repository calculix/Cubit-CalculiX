#include "CoreSectionsTreeData.hpp"
#include "CoreSections.hpp"
#include "CoreOrientations.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/materials/CalculiXMaterialsInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::vector<std::string>> CoreSectionsTreeData::get_sections_tree_data(CoreSections& sections)
{
  CalculiXMeshInterface mesh;
  CalculiXMaterialsInterface materials;
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& section : sections.sections_data)
  {
    const auto* data = sections.get_section_data(section[0]);

    if (!data)
    {
      PRINT_ERROR("Section data missing or invalid for Section ID %d.\n", section[0]);
      continue;
    }

    std::string type;

    if (section[1] == 1)
    {
      type = "SOLID";
    }
    else if (section[1] == 2)
    {
      type = "SHELL";
    }
    else if (section[1] == 3)
    {
      type = "BEAM";
    }
    else if (section[1] == 4)
    {
      type = "MEMBRANE";
    }

    const std::string material_name = materials.get_material_name(std::stoi((*data)[2]));
    const std::string block_name = mesh.get_block_name(std::stoi((*data)[1]));
    std::string name = type + " (";

    if (section[1] == 3)
    {
      name += (*data)[3] + "|";
    }

    name += block_name + "|" + material_name + ")";
    tree_data.push_back({std::to_string(section[0]), name});
  }

  return tree_data;
}

std::vector<std::vector<std::string>> CoreSectionsTreeData::get_orientations_tree_data(CoreOrientations& orientations)
{
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& orientation : orientations.orientations_data)
  {
    const auto* name = orientations.find_orientation_name(orientation[0]);

    if (!name)
    {
      PRINT_ERROR("Name data missing for Orientation ID %d.\n", orientation[0]);
      continue;
    }

    tree_data.push_back({std::to_string(orientation[0]), *name});
  }

  return tree_data;
}
