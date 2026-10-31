#include "Core/sections/CoreSectionsCleanup.hpp"
#include "Core/sections/CoreSections.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/mesh/CoreBlocks.hpp"

#include <cstddef>
#include <vector>

bool CoreSectionsCleanup::cleanup(CoreSections& sections, CoreMaterials& materials, CoreBlocks& blocks, CoreOrientations* orientations, std::string& log)
{
  bool changed = false;

  for (std::size_t index = sections.sections_data.size(); index > 0; index--)
  {
    const auto& section = sections.sections_data[index - 1];
    std::vector<std::vector<std::string>>* data = nullptr;
    int data_index = -1;
    std::size_t orientation_column = 3;

    if (section[1] == 1)
    {
      data = &sections.solid_section_data;
      data_index = sections.get_solid_section_data_id_from_solid_section_id(section[2]);
    }
    else if (section[1] == 2)
    {
      data = &sections.shell_section_data;
      data_index = sections.get_shell_section_data_id_from_shell_section_id(section[2]);
    }
    else if (section[1] == 3)
    {
      data = &sections.beam_section_data;
      data_index = sections.get_beam_section_data_id_from_beam_section_id(section[2]);
      orientation_column = 13;
    }
    else if (section[1] == 4)
    {
      data = &sections.membrane_section_data;
      data_index = sections.get_membrane_section_data_id_from_membrane_section_id(section[2]);
    }

    if (!data)
    {
      continue;
    }

    const int section_id = section[0];
    const std::string section_text = std::to_string(section_id);

    if (data_index < 0)
    {
      log.append("Section data for Section ID " + section_text + " doesn't exist.\n");
      log.append("Section ID " + section_text + " will be deleted.\n");
      changed = sections.delete_section(section_id) || changed;
      continue;
    }

    auto& row = (*data)[data_index];
    bool remove_section = false;

    if (!materials.check_material_exists(std::stoi(row[2])))
    {
      log.append("Material " + row[2] + " doesn't exist.\n");
      log.append("Section ID " + section_text + " will be deleted.\n");
      remove_section = true;
    }

    if (blocks.get_blocks_data_id_from_block_id(std::stoi(row[1])) == -1)
    {
      log.append("Block ID " + row[1] + " doesn't exist.\n");
      log.append("Section ID " + section_text + " will be deleted.\n");
      remove_section = true;
    }

    if (row[orientation_column] != "-1")
    {
      const int orientation_id = std::stoi(row[orientation_column]);

      if (!orientations || !orientations->check_orientation_exists(orientation_id))
      {
        log.append("Orientation ID " + row[orientation_column] + " doesn't exist.\n");
        log.append("Orientation Reference from Section ID " + section_text + " will be deleted.\n");
        row[orientation_column] = "-1";
        changed = true;
      }
    }

    if (remove_section)
    {
      sections.delete_section(section_id);
      changed = true;
    }
  }

  return changed;
}
