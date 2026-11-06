#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/amplitudes/CoreAmplitudesPersistence.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreAmplitudesPersistence::read(HDF5Tool& file, CoreAmplitudes& amplitudes)
{
  try
  {

    decltype(amplitudes.amplitudes_data) saved_amplitudes_data;
    decltype(amplitudes.name_amplitude_data) saved_name_amplitude_data;
    decltype(amplitudes.shiftx_amplitude_data) saved_shiftx_amplitude_data;
    decltype(amplitudes.shifty_amplitude_data) saved_shifty_amplitude_data;
    decltype(amplitudes.amplitudevalues_amplitude_data) saved_amplitudevalues_amplitude_data;
    if (file.nameExists(CalculiXModelSchema::path("Amplitudes/amplitudes_data")) && !file.read_dataset_int_rank_2("amplitudes_data", CalculiXModelSchema::path("Amplitudes"), saved_amplitudes_data))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Amplitudes/name_amplitude_data")) && !file.read_dataset_string_rank_2("name_amplitude_data", CalculiXModelSchema::path("Amplitudes"), saved_name_amplitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Amplitudes/shiftx_amplitude_data")) && !file.read_dataset_string_rank_2("shiftx_amplitude_data", CalculiXModelSchema::path("Amplitudes"), saved_shiftx_amplitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Amplitudes/shifty_amplitude_data")) && !file.read_dataset_string_rank_2("shifty_amplitude_data", CalculiXModelSchema::path("Amplitudes"), saved_shifty_amplitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Amplitudes/amplitudevalues_amplitude_data")) && !file.read_dataset_double_rank_2("amplitudevalues_amplitude_data", CalculiXModelSchema::path("Amplitudes"), saved_amplitudevalues_amplitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_amplitudes_data, 6)))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: amplitudes_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_amplitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: name_amplitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_shiftx_amplitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: shiftx_amplitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_shifty_amplitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: shifty_amplitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_amplitudevalues_amplitude_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: amplitudevalues_amplitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_amplitudes_data, 1, saved_name_amplitude_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: invalid name_amplitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_amplitudes_data, 2, saved_shiftx_amplitude_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: invalid shiftx_amplitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_amplitudes_data, 3, saved_shifty_amplitude_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: invalid shifty_amplitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_amplitudes_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Amplitudes: invalid or duplicate IDs.\n");
      return false;
    }
    amplitudes.amplitudes_data.swap(saved_amplitudes_data);
    amplitudes.name_amplitude_data.swap(saved_name_amplitude_data);
    amplitudes.shiftx_amplitude_data.swap(saved_shiftx_amplitude_data);
    amplitudes.shifty_amplitude_data.swap(saved_shifty_amplitude_data);
    amplitudes.amplitudevalues_amplitude_data.swap(saved_amplitudevalues_amplitude_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Amplitudes: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Amplitudes: %s\n", error.what());
    return false;
  }
}

bool CoreAmplitudesPersistence::write(HDF5Tool& file, const CoreAmplitudes& amplitudes)
{
  try
  {

    if (!(PersistenceTable::keyedRows(amplitudes.amplitudes_data, 6)))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: amplitudes_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(amplitudes.name_amplitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: name_amplitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(amplitudes.shiftx_amplitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: shiftx_amplitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(amplitudes.shifty_amplitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: shifty_amplitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(amplitudes.amplitudevalues_amplitude_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: amplitudevalues_amplitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(amplitudes.amplitudes_data, 1, amplitudes.name_amplitude_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: invalid name_amplitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(amplitudes.amplitudes_data, 2, amplitudes.shiftx_amplitude_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: invalid shiftx_amplitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(amplitudes.amplitudes_data, 3, amplitudes.shifty_amplitude_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: invalid shifty_amplitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(amplitudes.amplitudes_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Amplitudes")))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("amplitudes_data", CalculiXModelSchema::path("Amplitudes"), amplitudes.amplitudes_data))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_amplitude_data", CalculiXModelSchema::path("Amplitudes"), amplitudes.name_amplitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("shiftx_amplitude_data", CalculiXModelSchema::path("Amplitudes"), amplitudes.shiftx_amplitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("shifty_amplitude_data", CalculiXModelSchema::path("Amplitudes"), amplitudes.shifty_amplitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("amplitudevalues_amplitude_data", CalculiXModelSchema::path("Amplitudes"), amplitudes.amplitudevalues_amplitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX Amplitudes: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Amplitudes: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Amplitudes: %s\n", error.what());
    return false;
  }
}
