#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/sections/CoreSectionsPersistence.hpp"
#include "Core/sections/CoreSections.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreSectionsPersistence::read(HDF5Tool& file, CoreSections& sections)
{
  try
  {

    decltype(sections.sections_data) saved_sections_data;
    decltype(sections.solid_section_data) saved_solid_section_data;
    decltype(sections.shell_section_data) saved_shell_section_data;
    decltype(sections.beam_section_data) saved_beam_section_data;
    decltype(sections.membrane_section_data) saved_membrane_section_data;
    if (file.nameExists(CalculiXModelSchema::path("Sections/sections_data")) && !file.read_dataset_int_rank_2("sections_data", CalculiXModelSchema::path("Sections"), saved_sections_data))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Sections/solid_section_data")) && !file.read_dataset_string_rank_2("solid_section_data", CalculiXModelSchema::path("Sections"), saved_solid_section_data))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Sections/shell_section_data")) && !file.read_dataset_string_rank_2("shell_section_data", CalculiXModelSchema::path("Sections"), saved_shell_section_data))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Sections/beam_section_data")) && !file.read_dataset_string_rank_2("beam_section_data", CalculiXModelSchema::path("Sections"), saved_beam_section_data))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Sections/membrane_section_data")) && !file.read_dataset_string_rank_2("membrane_section_data", CalculiXModelSchema::path("Sections"), saved_membrane_section_data))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_sections_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: sections_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_solid_section_data, 5)))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: solid_section_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_shell_section_data, 6)))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: shell_section_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_beam_section_data, 16)))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: beam_section_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_membrane_section_data, 6)))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: membrane_section_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_sections_data, 1, 1, 2, saved_solid_section_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: invalid solid_section_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_sections_data, 1, 2, 2, saved_shell_section_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: invalid shell_section_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_sections_data, 1, 3, 2, saved_beam_section_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: invalid beam_section_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_sections_data, 1, 4, 2, saved_membrane_section_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: invalid membrane_section_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(saved_sections_data, 1, {1, 2, 3, 4})))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_sections_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Sections: invalid or duplicate IDs.\n");
      return false;
    }
    sections.sections_data.swap(saved_sections_data);
    sections.solid_section_data.swap(saved_solid_section_data);
    sections.shell_section_data.swap(saved_shell_section_data);
    sections.beam_section_data.swap(saved_beam_section_data);
    sections.membrane_section_data.swap(saved_membrane_section_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Sections: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Sections: %s\n", error.what());
    return false;
  }
}

bool CoreSectionsPersistence::write(HDF5Tool& file, const CoreSections& sections)
{
  try
  {

    if (!(PersistenceTable::keyedRows(sections.sections_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: sections_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sections.solid_section_data, 5)))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: solid_section_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sections.shell_section_data, 6)))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: shell_section_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sections.beam_section_data, 16)))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: beam_section_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sections.membrane_section_data, 6)))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: membrane_section_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(sections.sections_data, 1, 1, 2, sections.solid_section_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: invalid solid_section_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(sections.sections_data, 1, 2, 2, sections.shell_section_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: invalid shell_section_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(sections.sections_data, 1, 3, 2, sections.beam_section_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: invalid beam_section_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(sections.sections_data, 1, 4, 2, sections.membrane_section_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: invalid membrane_section_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(sections.sections_data, 1, {1, 2, 3, 4})))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(sections.sections_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Sections")))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("sections_data", CalculiXModelSchema::path("Sections"), sections.sections_data))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("solid_section_data", CalculiXModelSchema::path("Sections"), sections.solid_section_data))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("shell_section_data", CalculiXModelSchema::path("Sections"), sections.shell_section_data))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("beam_section_data", CalculiXModelSchema::path("Sections"), sections.beam_section_data))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("membrane_section_data", CalculiXModelSchema::path("Sections"), sections.membrane_section_data))
    {
      PRINT_ERROR("Cannot write CalculiX Sections: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Sections: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Sections: %s\n", error.what());
    return false;
  }
}
