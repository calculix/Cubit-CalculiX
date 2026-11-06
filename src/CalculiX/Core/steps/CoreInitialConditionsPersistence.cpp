#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/steps/CoreInitialConditionsPersistence.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>
#include <algorithm>

bool CoreInitialConditionsPersistence::read(HDF5Tool& file, CoreInitialConditions& conditions)
{
  try
  {

    decltype(conditions.initialconditions_data) saved_initialconditions_data;
    decltype(conditions.displacement_data) saved_displacement_data;
    decltype(conditions.temperature_data) saved_temperature_data;
    decltype(conditions.stress_data) saved_stress_data;
    decltype(conditions.stress_block_data) saved_stress_block_data;
    decltype(conditions.stress_element_data) saved_stress_element_data;
    if (file.nameExists(CalculiXModelSchema::path("InitialConditions/initialconditions_data")) && !file.read_dataset_int_rank_2("initialconditions_data", CalculiXModelSchema::path("InitialConditions"), saved_initialconditions_data))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("InitialConditions/displacement_data")) && !file.read_dataset_string_rank_2("displacement_data", CalculiXModelSchema::path("InitialConditions"), saved_displacement_data))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("InitialConditions/temperature_data")) && !file.read_dataset_string_rank_2("temperature_data", CalculiXModelSchema::path("InitialConditions"), saved_temperature_data))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("InitialConditions/stress_data")) && !file.read_dataset_int_rank_1("stress_data", CalculiXModelSchema::path("InitialConditions"), saved_stress_data))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("InitialConditions/stress_block_data")) && !file.read_dataset_double_rank_2("stress_block_data", CalculiXModelSchema::path("InitialConditions"), saved_stress_block_data))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("InitialConditions/stress_element_data")) && !file.read_dataset_double_rank_2("stress_element_data", CalculiXModelSchema::path("InitialConditions"), saved_stress_element_data))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_initialconditions_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: initialconditions_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_displacement_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: displacement_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_temperature_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: temperature_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_stress_block_data, 8)))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: stress_block_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_stress_element_data, 9)))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: stress_element_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_initialconditions_data, 1, 1, 2, saved_displacement_data)))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: invalid displacement_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_initialconditions_data, 1, 2, 2, saved_temperature_data)))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: invalid temperature_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(saved_initialconditions_data, 1, {1, 2, 3})))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: unknown feature type.\n");
      return false;
    }
    for (const auto& row : saved_initialconditions_data)
    {
      if (row[1] == 3 && std::find(saved_stress_data.begin(), saved_stress_data.end(), row[2]) == saved_stress_data.end())
      {
        PRINT_ERROR("Cannot read CalculiX initial conditions: invalid stress reference.\n");
        return false;
      }
    }
    if (!(PersistenceTable::uniqueIds(saved_initialconditions_data)))
    {
      PRINT_ERROR("Cannot read CalculiX InitialConditions: invalid or duplicate IDs.\n");
      return false;
    }
    conditions.initialconditions_data.swap(saved_initialconditions_data);
    conditions.displacement_data.swap(saved_displacement_data);
    conditions.temperature_data.swap(saved_temperature_data);
    conditions.stress_data.swap(saved_stress_data);
    conditions.stress_block_data.swap(saved_stress_block_data);
    conditions.stress_element_data.swap(saved_stress_element_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX InitialConditions: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX InitialConditions: %s\n", error.what());
    return false;
  }
}

bool CoreInitialConditionsPersistence::write(HDF5Tool& file, const CoreInitialConditions& conditions)
{
  try
  {

    if (!(PersistenceTable::keyedRows(conditions.initialconditions_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: initialconditions_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(conditions.displacement_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: displacement_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(conditions.temperature_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: temperature_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(conditions.stress_block_data, 8)))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: stress_block_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(conditions.stress_element_data, 9)))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: stress_element_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(conditions.initialconditions_data, 1, 1, 2, conditions.displacement_data)))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: invalid displacement_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(conditions.initialconditions_data, 1, 2, 2, conditions.temperature_data)))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: invalid temperature_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(conditions.initialconditions_data, 1, {1, 2, 3})))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: unknown feature type.\n");
      return false;
    }
    for (const auto& row : conditions.initialconditions_data)
    {
      if (row[1] == 3 && std::find(conditions.stress_data.begin(), conditions.stress_data.end(), row[2]) == conditions.stress_data.end())
      {
        PRINT_ERROR("Cannot write CalculiX initial conditions: invalid stress reference.\n");
        return false;
      }
    }
    if (!(PersistenceTable::uniqueIds(conditions.initialconditions_data)))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("InitialConditions")))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("initialconditions_data", CalculiXModelSchema::path("InitialConditions"), conditions.initialconditions_data))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("displacement_data", CalculiXModelSchema::path("InitialConditions"), conditions.displacement_data))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("temperature_data", CalculiXModelSchema::path("InitialConditions"), conditions.temperature_data))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_1("stress_data", CalculiXModelSchema::path("InitialConditions"), conditions.stress_data))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("stress_block_data", CalculiXModelSchema::path("InitialConditions"), conditions.stress_block_data))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("stress_element_data", CalculiXModelSchema::path("InitialConditions"), conditions.stress_element_data))
    {
      PRINT_ERROR("Cannot write CalculiX InitialConditions: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX InitialConditions: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX InitialConditions: %s\n", error.what());
    return false;
  }
}
