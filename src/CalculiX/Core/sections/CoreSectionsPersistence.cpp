#include "Core/sections/CoreSectionsPersistence.hpp"
#include "Core/sections/CoreSections.hpp"
#include "HDF5Tool.hpp"

void CoreSectionsPersistence::read(HDF5Tool& file, CoreSections& sections)
{
  file.read_dataset_int_rank_2("sections_data", "Cubit-CalculiX/Sections", sections.sections_data);
  file.read_dataset_string_rank_2("solid_section_data", "Cubit-CalculiX/Sections", sections.solid_section_data);
  file.read_dataset_string_rank_2("shell_section_data", "Cubit-CalculiX/Sections", sections.shell_section_data);
  file.read_dataset_string_rank_2("beam_section_data", "Cubit-CalculiX/Sections", sections.beam_section_data);
  file.read_dataset_string_rank_2("membrane_section_data", "Cubit-CalculiX/Sections", sections.membrane_section_data);
}

void CoreSectionsPersistence::write(HDF5Tool& file, const CoreSections& sections)
{
  file.createGroup("Cubit-CalculiX/Sections");
  file.write_dataset_int_rank_2("sections_data", "Cubit-CalculiX/Sections", sections.sections_data);
  file.write_dataset_string_rank_2("solid_section_data", "Cubit-CalculiX/Sections", sections.solid_section_data);
  file.write_dataset_string_rank_2("shell_section_data", "Cubit-CalculiX/Sections", sections.shell_section_data);
  file.write_dataset_string_rank_2("beam_section_data", "Cubit-CalculiX/Sections", sections.beam_section_data);
  file.write_dataset_string_rank_2("membrane_section_data", "Cubit-CalculiX/Sections", sections.membrane_section_data);
}
