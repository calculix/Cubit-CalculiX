#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/export/CoreCustomLinesPersistence.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreCustomLinesPersistence::read(HDF5Tool& file, CoreCustomLines& lines)
{
  try
  {

    decltype(lines.customlines_data) saved_customlines_data;
    if (file.nameExists(CalculiXModelSchema::path("CustomLines/customlines_data")) && !file.read_dataset_string_rank_2("customlines_data", CalculiXModelSchema::path("CustomLines"), saved_customlines_data))
    {
      PRINT_ERROR("Cannot read CalculiX CustomLines: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_customlines_data, 6)))
    {
      PRINT_ERROR("Cannot read CalculiX CustomLines: customlines_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_customlines_data)))
    {
      PRINT_ERROR("Cannot read CalculiX CustomLines: invalid or duplicate IDs.\n");
      return false;
    }
    lines.customlines_data.swap(saved_customlines_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX CustomLines: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX CustomLines: %s\n", error.what());
    return false;
  }
}

bool CoreCustomLinesPersistence::write(HDF5Tool& file, const CoreCustomLines& lines)
{
  try
  {

    if (!(PersistenceTable::keyedRows(lines.customlines_data, 6)))
    {
      PRINT_ERROR("Cannot write CalculiX CustomLines: customlines_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(lines.customlines_data)))
    {
      PRINT_ERROR("Cannot write CalculiX CustomLines: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("CustomLines")))
    {
      PRINT_ERROR("Cannot write CalculiX CustomLines: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("customlines_data", CalculiXModelSchema::path("CustomLines"), lines.customlines_data))
    {
      PRINT_ERROR("Cannot write CalculiX CustomLines: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX CustomLines: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX CustomLines: %s\n", error.what());
    return false;
  }
}
