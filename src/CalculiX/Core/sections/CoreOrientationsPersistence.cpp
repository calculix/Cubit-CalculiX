#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/sections/CoreOrientationsPersistence.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreOrientationsPersistence::read(HDF5Tool& file, CoreOrientations& orientations)
{
  try
  {

    decltype(orientations.orientations_data) saved_orientations_data;
    decltype(orientations.name_data) saved_name_data;
    decltype(orientations.a_data) saved_a_data;
    decltype(orientations.b_data) saved_b_data;
    decltype(orientations.rotation_data) saved_rotation_data;
    if (file.nameExists(CalculiXModelSchema::path("Orientations/orientations_data")) && !file.read_dataset_int_rank_2("orientations_data", CalculiXModelSchema::path("Orientations"), saved_orientations_data))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Orientations/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Orientations"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Orientations/a_data")) && !file.read_dataset_string_rank_2("a_data", CalculiXModelSchema::path("Orientations"), saved_a_data))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Orientations/b_data")) && !file.read_dataset_string_rank_2("b_data", CalculiXModelSchema::path("Orientations"), saved_b_data))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Orientations/rotation_data")) && !file.read_dataset_string_rank_2("rotation_data", CalculiXModelSchema::path("Orientations"), saved_rotation_data))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_orientations_data, 8)))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: orientations_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_a_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: a_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_b_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: b_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_rotation_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: rotation_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_orientations_data, 1, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_orientations_data, 4, saved_a_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: invalid a_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_orientations_data, 5, saved_b_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: invalid b_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_orientations_data, 7, saved_rotation_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: invalid rotation_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_orientations_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Orientations: invalid or duplicate IDs.\n");
      return false;
    }
    orientations.orientations_data.swap(saved_orientations_data);
    orientations.name_data.swap(saved_name_data);
    orientations.a_data.swap(saved_a_data);
    orientations.b_data.swap(saved_b_data);
    orientations.rotation_data.swap(saved_rotation_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Orientations: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Orientations: %s\n", error.what());
    return false;
  }
}

bool CoreOrientationsPersistence::write(HDF5Tool& file, const CoreOrientations& orientations)
{
  try
  {

    if (!(PersistenceTable::keyedRows(orientations.orientations_data, 8)))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: orientations_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(orientations.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(orientations.a_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: a_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(orientations.b_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: b_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(orientations.rotation_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: rotation_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(orientations.orientations_data, 1, orientations.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(orientations.orientations_data, 4, orientations.a_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: invalid a_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(orientations.orientations_data, 5, orientations.b_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: invalid b_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(orientations.orientations_data, 7, orientations.rotation_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: invalid rotation_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(orientations.orientations_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Orientations")))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("orientations_data", CalculiXModelSchema::path("Orientations"), orientations.orientations_data))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Orientations"), orientations.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("a_data", CalculiXModelSchema::path("Orientations"), orientations.a_data))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("b_data", CalculiXModelSchema::path("Orientations"), orientations.b_data))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("rotation_data", CalculiXModelSchema::path("Orientations"), orientations.rotation_data))
    {
      PRINT_ERROR("Cannot write CalculiX Orientations: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Orientations: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Orientations: %s\n", error.what());
    return false;
  }
}
