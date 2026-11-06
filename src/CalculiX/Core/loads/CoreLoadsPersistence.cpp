#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/loads/CoreLoadsPersistence.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsTrajectoryHeatflux.hpp"
#include "Core/loads/CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsForces& forces)
{
  try
  {

    decltype(forces.loads_data) saved_loads_data;
    decltype(forces.time_delay_data) saved_time_delay_data;
    decltype(forces.name_data) saved_name_data;
    decltype(forces.force_data) saved_force_data;
    if (file.nameExists(CalculiXModelSchema::path("Loads/Forces/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Forces"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Forces/time_delay_data")) && !file.read_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Forces"), saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Forces/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Forces"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Forces/force_data")) && !file.read_dataset_double_rank_2("force_data", CalculiXModelSchema::path("Loads/Forces"), saved_force_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 7)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_force_data, 7)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: force_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 5, saved_force_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: invalid force_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 6, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsForces: invalid or duplicate IDs.\n");
      return false;
    }
    forces.loads_data.swap(saved_loads_data);
    forces.time_delay_data.swap(saved_time_delay_data);
    forces.name_data.swap(saved_name_data);
    forces.force_data.swap(saved_force_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsForces: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsForces: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsForces& forces)
{
  try
  {

    if (!(PersistenceTable::keyedRows(forces.loads_data, 7)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(forces.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(forces.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(forces.force_data, 7)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: force_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(forces.loads_data, 3, forces.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(forces.loads_data, 5, forces.force_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: invalid force_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(forces.loads_data, 6, forces.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(forces.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/Forces")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Forces"), forces.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Forces"), forces.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Forces"), forces.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("force_data", CalculiXModelSchema::path("Loads/Forces"), forces.force_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsForces: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsForces: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsForces: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsPressures& pressures)
{
  try
  {

    decltype(pressures.loads_data) saved_loads_data;
    decltype(pressures.time_delay_data) saved_time_delay_data;
    decltype(pressures.name_data) saved_name_data;
    decltype(pressures.magnitude_data) saved_magnitude_data;
    if (file.nameExists(CalculiXModelSchema::path("Loads/Pressures/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Pressures"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Pressures/time_delay_data")) && !file.read_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Pressures"), saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Pressures/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Pressures"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Pressures/magnitude_data")) && !file.read_dataset_double_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/Pressures"), saved_magnitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 7)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 5, saved_magnitude_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 6, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsPressures: invalid or duplicate IDs.\n");
      return false;
    }
    pressures.loads_data.swap(saved_loads_data);
    pressures.time_delay_data.swap(saved_time_delay_data);
    pressures.name_data.swap(saved_name_data);
    pressures.magnitude_data.swap(saved_magnitude_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsPressures: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsPressures: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsPressures& pressures)
{
  try
  {

    if (!(PersistenceTable::keyedRows(pressures.loads_data, 7)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(pressures.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(pressures.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(pressures.magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(pressures.loads_data, 3, pressures.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(pressures.loads_data, 5, pressures.magnitude_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(pressures.loads_data, 6, pressures.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(pressures.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/Pressures")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Pressures"), pressures.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Pressures"), pressures.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Pressures"), pressures.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/Pressures"), pressures.magnitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsPressures: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsPressures: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsPressures: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsHeatfluxes& heatfluxes)
{
  try
  {

    decltype(heatfluxes.loads_data) saved_loads_data;
    decltype(heatfluxes.time_delay_data) saved_time_delay_data;
    decltype(heatfluxes.name_data) saved_name_data;
    decltype(heatfluxes.magnitude_data) saved_magnitude_data;
    if (file.nameExists(CalculiXModelSchema::path("Loads/HeatFluxes/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/HeatFluxes"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/HeatFluxes/time_delay_data")) && !file.read_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/HeatFluxes"), saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/HeatFluxes/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/HeatFluxes"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/HeatFluxes/magnitude_data")) && !file.read_dataset_double_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/HeatFluxes"), saved_magnitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 7)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 5, saved_magnitude_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 6, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: invalid or duplicate IDs.\n");
      return false;
    }
    heatfluxes.loads_data.swap(saved_loads_data);
    heatfluxes.time_delay_data.swap(saved_time_delay_data);
    heatfluxes.name_data.swap(saved_name_data);
    heatfluxes.magnitude_data.swap(saved_magnitude_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsHeatfluxes: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsHeatfluxes& heatfluxes)
{
  try
  {

    if (!(PersistenceTable::keyedRows(heatfluxes.loads_data, 7)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(heatfluxes.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(heatfluxes.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(heatfluxes.magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(heatfluxes.loads_data, 3, heatfluxes.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(heatfluxes.loads_data, 5, heatfluxes.magnitude_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(heatfluxes.loads_data, 6, heatfluxes.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(heatfluxes.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/HeatFluxes")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/HeatFluxes"), heatfluxes.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/HeatFluxes"), heatfluxes.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/HeatFluxes"), heatfluxes.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/HeatFluxes"), heatfluxes.magnitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsHeatfluxes: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsGravity& gravity)
{
  try
  {

    decltype(gravity.loads_data) saved_loads_data;
    decltype(gravity.time_delay_data) saved_time_delay_data;
    decltype(gravity.direction_data) saved_direction_data;
    decltype(gravity.magnitude_data) saved_magnitude_data;
    decltype(gravity.name_data) saved_name_data;
    if (file.nameExists(CalculiXModelSchema::path("Loads/Gravity/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Gravity"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Gravity/time_delay_data")) && !file.read_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Gravity"), saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Gravity/direction_data")) && !file.read_dataset_string_rank_2("direction_data", CalculiXModelSchema::path("Loads/Gravity"), saved_direction_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Gravity/magnitude_data")) && !file.read_dataset_string_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/Gravity"), saved_magnitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Gravity/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Gravity"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 8)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_direction_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: direction_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 5, saved_direction_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: invalid direction_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 6, saved_magnitude_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 7, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsGravity: invalid or duplicate IDs.\n");
      return false;
    }
    gravity.loads_data.swap(saved_loads_data);
    gravity.time_delay_data.swap(saved_time_delay_data);
    gravity.direction_data.swap(saved_direction_data);
    gravity.magnitude_data.swap(saved_magnitude_data);
    gravity.name_data.swap(saved_name_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsGravity: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsGravity: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsGravity& gravity)
{
  try
  {

    if (!(PersistenceTable::keyedRows(gravity.loads_data, 8)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(gravity.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(gravity.direction_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: direction_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(gravity.magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(gravity.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(gravity.loads_data, 3, gravity.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(gravity.loads_data, 5, gravity.direction_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: invalid direction_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(gravity.loads_data, 6, gravity.magnitude_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(gravity.loads_data, 7, gravity.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(gravity.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/Gravity")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Gravity"), gravity.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Gravity"), gravity.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("direction_data", CalculiXModelSchema::path("Loads/Gravity"), gravity.direction_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/Gravity"), gravity.magnitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Gravity"), gravity.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsGravity: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsGravity: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsGravity: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsCentrifugal& centrifugal)
{
  try
  {

    decltype(centrifugal.loads_data) saved_loads_data;
    decltype(centrifugal.time_delay_data) saved_time_delay_data;
    decltype(centrifugal.direction_data) saved_direction_data;
    decltype(centrifugal.magnitude_data) saved_magnitude_data;
    decltype(centrifugal.coordinate_data) saved_coordinate_data;
    decltype(centrifugal.name_data) saved_name_data;
    if (file.nameExists(CalculiXModelSchema::path("Loads/Centrifugal/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Centrifugal"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Centrifugal/time_delay_data")) && !file.read_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Centrifugal"), saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Centrifugal/direction_data")) && !file.read_dataset_string_rank_2("direction_data", CalculiXModelSchema::path("Loads/Centrifugal"), saved_direction_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Centrifugal/magnitude_data")) && !file.read_dataset_string_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/Centrifugal"), saved_magnitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Centrifugal/coordinate_data")) && !file.read_dataset_string_rank_2("coordinate_data", CalculiXModelSchema::path("Loads/Centrifugal"), saved_coordinate_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Centrifugal/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Centrifugal"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 9)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_direction_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: direction_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_coordinate_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: coordinate_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 5, saved_direction_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: invalid direction_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 6, saved_magnitude_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 7, saved_coordinate_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: invalid coordinate_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 8, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: invalid or duplicate IDs.\n");
      return false;
    }
    centrifugal.loads_data.swap(saved_loads_data);
    centrifugal.time_delay_data.swap(saved_time_delay_data);
    centrifugal.direction_data.swap(saved_direction_data);
    centrifugal.magnitude_data.swap(saved_magnitude_data);
    centrifugal.coordinate_data.swap(saved_coordinate_data);
    centrifugal.name_data.swap(saved_name_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsCentrifugal: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsCentrifugal& centrifugal)
{
  try
  {

    if (!(PersistenceTable::keyedRows(centrifugal.loads_data, 9)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(centrifugal.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(centrifugal.direction_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: direction_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(centrifugal.magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(centrifugal.coordinate_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: coordinate_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(centrifugal.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(centrifugal.loads_data, 3, centrifugal.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(centrifugal.loads_data, 5, centrifugal.direction_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: invalid direction_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(centrifugal.loads_data, 6, centrifugal.magnitude_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(centrifugal.loads_data, 7, centrifugal.coordinate_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: invalid coordinate_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(centrifugal.loads_data, 8, centrifugal.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(centrifugal.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/Centrifugal")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Centrifugal"), centrifugal.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Centrifugal"), centrifugal.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("direction_data", CalculiXModelSchema::path("Loads/Centrifugal"), centrifugal.direction_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/Centrifugal"), centrifugal.magnitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("coordinate_data", CalculiXModelSchema::path("Loads/Centrifugal"), centrifugal.coordinate_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Centrifugal"), centrifugal.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsCentrifugal: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsTrajectory& trajectory)
{
  try
  {
    if (trajectory.heatflux == nullptr || trajectory.bodyheatfluxsphere == nullptr)
    {
      PRINT_ERROR("Cannot access CalculiX trajectory data: subtype storage is missing.\n");
      return false;
    }
    auto& heatflux = *trajectory.heatflux;
    auto& sphere = *trajectory.bodyheatfluxsphere;
    decltype(trajectory.loads_data) saved_loads_data;
    decltype(heatflux.loads_data) saved_heatflux_loads_data;
    decltype(heatflux.fire_ray_surface_data) saved_heatflux_fire_ray_surface_data;
    decltype(heatflux.direction_data) saved_heatflux_direction_data;
    decltype(heatflux.magnitude_data) saved_heatflux_magnitude_data;
    decltype(heatflux.radius_data) saved_heatflux_radius_data;
    decltype(heatflux.time_data) saved_heatflux_time_data;
    decltype(heatflux.name_data) saved_heatflux_name_data;
    decltype(sphere.loads_data) saved_sphere_loads_data;
    decltype(sphere.fire_ray_surface_data) saved_sphere_fire_ray_surface_data;
    decltype(sphere.direction_data) saved_sphere_direction_data;
    decltype(sphere.magnitude_data) saved_sphere_magnitude_data;
    decltype(sphere.radius_data) saved_sphere_radius_data;
    decltype(sphere.depth_data) saved_sphere_depth_data;
    decltype(sphere.time_data) saved_sphere_time_data;
    decltype(sphere.name_data) saved_sphere_name_data;
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Trajectory"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }

    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/Heatflux/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), saved_heatflux_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/Heatflux/fire_ray_surface_data")) && !file.read_dataset_int_rank_2("fire_ray_surface_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), saved_heatflux_fire_ray_surface_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/Heatflux/direction_data")) && !file.read_dataset_string_rank_2("direction_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), saved_heatflux_direction_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/Heatflux/magnitude_data")) && !file.read_dataset_double_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), saved_heatflux_magnitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/Heatflux/radius_data")) && !file.read_dataset_double_rank_2("radius_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), saved_heatflux_radius_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/Heatflux/time_data")) && !file.read_dataset_string_rank_2("time_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), saved_heatflux_time_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/Heatflux/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), saved_heatflux_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }

    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), saved_sphere_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere/fire_ray_surface_data")) && !file.read_dataset_int_rank_2("fire_ray_surface_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), saved_sphere_fire_ray_surface_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere/direction_data")) && !file.read_dataset_string_rank_2("direction_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), saved_sphere_direction_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere/magnitude_data")) && !file.read_dataset_double_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), saved_sphere_magnitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere/radius_data")) && !file.read_dataset_double_rank_2("radius_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), saved_sphere_radius_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere/depth_data")) && !file.read_dataset_double_rank_2("depth_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), saved_sphere_depth_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere/time_data")) && !file.read_dataset_string_rank_2("time_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), saved_sphere_time_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), saved_sphere_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX trajectory: invalid loads_data record (at least 3 columns required).\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_heatflux_loads_data, 10)))
    {
      PRINT_ERROR("Cannot read CalculiX trajectory heatflux: invalid loads_data record (at least 10 columns required).\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_heatflux_fire_ray_surface_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: fire_ray_surface_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_heatflux_direction_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: direction_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_heatflux_magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_heatflux_radius_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: radius_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_heatflux_time_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: time_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_heatflux_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_sphere_loads_data, 12)))
    {
      PRINT_ERROR("Cannot read CalculiX trajectory body-heatflux-sphere: invalid loads_data record (at least 12 columns required).\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_sphere_fire_ray_surface_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: fire_ray_surface_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_sphere_direction_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: direction_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_sphere_magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_sphere_radius_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: radius_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_sphere_depth_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: depth_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_sphere_time_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: time_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_sphere_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::types(saved_loads_data, 1, {1, 2})))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: invalid or duplicate IDs.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_loads_data, 1, 1, 2, saved_heatflux_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: invalid trajectory subtype reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_heatflux_loads_data, 5, saved_heatflux_direction_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: invalid trajectory direction_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_heatflux_loads_data, 7, saved_heatflux_time_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: invalid trajectory time_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_heatflux_loads_data, 9, saved_heatflux_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: invalid trajectory name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_loads_data, 1, 2, 2, saved_sphere_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: invalid trajectory subtype reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_sphere_loads_data, 5, saved_sphere_direction_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: invalid trajectory direction_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_sphere_loads_data, 7, saved_sphere_time_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: invalid trajectory time_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_sphere_loads_data, 10, saved_sphere_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: invalid trajectory name_data reference.\n");
      return false;
    }
    trajectory.loads_data.swap(saved_loads_data);
    heatflux.loads_data.swap(saved_heatflux_loads_data);
    heatflux.fire_ray_surface_data.swap(saved_heatflux_fire_ray_surface_data);
    heatflux.direction_data.swap(saved_heatflux_direction_data);
    heatflux.magnitude_data.swap(saved_heatflux_magnitude_data);
    heatflux.radius_data.swap(saved_heatflux_radius_data);
    heatflux.time_data.swap(saved_heatflux_time_data);
    heatflux.name_data.swap(saved_heatflux_name_data);
    sphere.loads_data.swap(saved_sphere_loads_data);
    sphere.fire_ray_surface_data.swap(saved_sphere_fire_ray_surface_data);
    sphere.direction_data.swap(saved_sphere_direction_data);
    sphere.magnitude_data.swap(saved_sphere_magnitude_data);
    sphere.radius_data.swap(saved_sphere_radius_data);
    sphere.depth_data.swap(saved_sphere_depth_data);
    sphere.time_data.swap(saved_sphere_time_data);
    sphere.name_data.swap(saved_sphere_name_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsTrajectory: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsTrajectory& trajectory)
{
  try
  {
    if (trajectory.heatflux == nullptr || trajectory.bodyheatfluxsphere == nullptr)
    {
      PRINT_ERROR("Cannot access CalculiX trajectory data: subtype storage is missing.\n");
      return false;
    }
    const auto& heatflux = *trajectory.heatflux;
    const auto& sphere = *trajectory.bodyheatfluxsphere;
    if (!(PersistenceTable::keyedRows(trajectory.loads_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX trajectory: invalid loads_data record (at least 3 columns required).\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(heatflux.loads_data, 10)))
    {
      PRINT_ERROR("Cannot write CalculiX trajectory heatflux: invalid loads_data record (at least 10 columns required).\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(heatflux.fire_ray_surface_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: fire_ray_surface_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(heatflux.direction_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: direction_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(heatflux.magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(heatflux.radius_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: radius_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(heatflux.time_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: time_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(heatflux.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sphere.loads_data, 12)))
    {
      PRINT_ERROR("Cannot write CalculiX trajectory body-heatflux-sphere: invalid loads_data record (at least 12 columns required).\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sphere.fire_ray_surface_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: fire_ray_surface_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sphere.direction_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: direction_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sphere.magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sphere.radius_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: radius_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sphere.depth_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: depth_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sphere.time_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: time_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(sphere.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::types(trajectory.loads_data, 1, {1, 2})))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(trajectory.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: invalid or duplicate IDs.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(trajectory.loads_data, 1, 1, 2, heatflux.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: invalid trajectory subtype reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(heatflux.loads_data, 5, heatflux.direction_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: invalid trajectory direction_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(heatflux.loads_data, 7, heatflux.time_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: invalid trajectory time_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(heatflux.loads_data, 9, heatflux.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: invalid trajectory name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(trajectory.loads_data, 1, 2, 2, sphere.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: invalid trajectory subtype reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(sphere.loads_data, 5, sphere.direction_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: invalid trajectory direction_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(sphere.loads_data, 7, sphere.time_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: invalid trajectory time_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(sphere.loads_data, 10, sphere.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: invalid trajectory name_data reference.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/Trajectory")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Trajectory"), trajectory.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }

    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/Trajectory/Heatflux")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), heatflux.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("fire_ray_surface_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), heatflux.fire_ray_surface_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("direction_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), heatflux.direction_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), heatflux.magnitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("radius_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), heatflux.radius_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), heatflux.time_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Trajectory/Heatflux"), heatflux.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }

    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), sphere.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("fire_ray_surface_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), sphere.fire_ray_surface_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("direction_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), sphere.direction_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), sphere.magnitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("radius_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), sphere.radius_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("depth_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), sphere.depth_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), sphere.time_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Trajectory/BodyHeatfluxSphere"), sphere.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsTrajectory: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsFilm& film)
{
  try
  {

    decltype(film.loads_data) saved_loads_data;
    decltype(film.time_delay_data) saved_time_delay_data;
    decltype(film.temperature_data) saved_temperature_data;
    decltype(film.coefficient_data) saved_coefficient_data;
    decltype(film.film_time_delay_data) saved_film_time_delay_data;
    decltype(film.name_data) saved_name_data;
    if (file.nameExists(CalculiXModelSchema::path("Loads/Film/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Film"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Film/time_delay_data")) && !file.read_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Film"), saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Film/temperature_data")) && !file.read_dataset_string_rank_2("temperature_data", CalculiXModelSchema::path("Loads/Film"), saved_temperature_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Film/coefficient_data")) && !file.read_dataset_string_rank_2("coefficient_data", CalculiXModelSchema::path("Loads/Film"), saved_coefficient_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Film/film_time_delay_data")) && !file.read_dataset_string_rank_2("film_time_delay_data", CalculiXModelSchema::path("Loads/Film"), saved_film_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Film/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Film"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 10)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_temperature_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: temperature_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_coefficient_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: coefficient_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_film_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: film_time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 5, saved_temperature_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: invalid temperature_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 6, saved_coefficient_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: invalid coefficient_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 8, saved_film_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: invalid film_time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 9, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsFilm: invalid or duplicate IDs.\n");
      return false;
    }
    film.loads_data.swap(saved_loads_data);
    film.time_delay_data.swap(saved_time_delay_data);
    film.temperature_data.swap(saved_temperature_data);
    film.coefficient_data.swap(saved_coefficient_data);
    film.film_time_delay_data.swap(saved_film_time_delay_data);
    film.name_data.swap(saved_name_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsFilm: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsFilm: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsFilm& film)
{
  try
  {

    if (!(PersistenceTable::keyedRows(film.loads_data, 10)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(film.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(film.temperature_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: temperature_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(film.coefficient_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: coefficient_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(film.film_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: film_time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(film.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(film.loads_data, 3, film.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(film.loads_data, 5, film.temperature_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: invalid temperature_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(film.loads_data, 6, film.coefficient_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: invalid coefficient_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(film.loads_data, 8, film.film_time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: invalid film_time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(film.loads_data, 9, film.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(film.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/Film")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Film"), film.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Film"), film.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("temperature_data", CalculiXModelSchema::path("Loads/Film"), film.temperature_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("coefficient_data", CalculiXModelSchema::path("Loads/Film"), film.coefficient_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("film_time_delay_data", CalculiXModelSchema::path("Loads/Film"), film.film_time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Film"), film.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsFilm: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsFilm: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsFilm: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsRadiation& radiation)
{
  try
  {

    decltype(radiation.loads_data) saved_loads_data;
    decltype(radiation.time_delay_data) saved_time_delay_data;
    decltype(radiation.temperature_data) saved_temperature_data;
    decltype(radiation.emissivity_data) saved_emissivity_data;
    decltype(radiation.radiation_time_delay_data) saved_radiation_time_delay_data;
    decltype(radiation.name_data) saved_name_data;
    decltype(radiation.cavity_data) saved_cavity_data;
    if (file.nameExists(CalculiXModelSchema::path("Loads/Radiation/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Radiation"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Radiation/time_delay_data")) && !file.read_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Radiation"), saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Radiation/temperature_data")) && !file.read_dataset_string_rank_2("temperature_data", CalculiXModelSchema::path("Loads/Radiation"), saved_temperature_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Radiation/emissivity_data")) && !file.read_dataset_string_rank_2("emissivity_data", CalculiXModelSchema::path("Loads/Radiation"), saved_emissivity_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Radiation/radiation_time_delay_data")) && !file.read_dataset_string_rank_2("radiation_time_delay_data", CalculiXModelSchema::path("Loads/Radiation"), saved_radiation_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Radiation/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Radiation"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/Radiation/cavity_data")) && !file.read_dataset_string_rank_2("cavity_data", CalculiXModelSchema::path("Loads/Radiation"), saved_cavity_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 12)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_temperature_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: temperature_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_emissivity_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: emissivity_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_radiation_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: radiation_time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_cavity_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: cavity_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 5, saved_temperature_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: invalid temperature_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 6, saved_emissivity_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: invalid emissivity_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 8, saved_radiation_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: invalid radiation_time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 9, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 10, saved_cavity_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: invalid cavity_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsRadiation: invalid or duplicate IDs.\n");
      return false;
    }
    radiation.loads_data.swap(saved_loads_data);
    radiation.time_delay_data.swap(saved_time_delay_data);
    radiation.temperature_data.swap(saved_temperature_data);
    radiation.emissivity_data.swap(saved_emissivity_data);
    radiation.radiation_time_delay_data.swap(saved_radiation_time_delay_data);
    radiation.name_data.swap(saved_name_data);
    radiation.cavity_data.swap(saved_cavity_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsRadiation: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsRadiation: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsRadiation& radiation)
{
  try
  {

    if (!(PersistenceTable::keyedRows(radiation.loads_data, 12)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(radiation.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(radiation.temperature_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: temperature_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(radiation.emissivity_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: emissivity_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(radiation.radiation_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: radiation_time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(radiation.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(radiation.cavity_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: cavity_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(radiation.loads_data, 3, radiation.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(radiation.loads_data, 5, radiation.temperature_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: invalid temperature_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(radiation.loads_data, 6, radiation.emissivity_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: invalid emissivity_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(radiation.loads_data, 8, radiation.radiation_time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: invalid radiation_time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(radiation.loads_data, 9, radiation.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(radiation.loads_data, 10, radiation.cavity_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: invalid cavity_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(radiation.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/Radiation")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/Radiation"), radiation.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/Radiation"), radiation.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("temperature_data", CalculiXModelSchema::path("Loads/Radiation"), radiation.temperature_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("emissivity_data", CalculiXModelSchema::path("Loads/Radiation"), radiation.emissivity_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("radiation_time_delay_data", CalculiXModelSchema::path("Loads/Radiation"), radiation.radiation_time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/Radiation"), radiation.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("cavity_data", CalculiXModelSchema::path("Loads/Radiation"), radiation.cavity_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsRadiation: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsRadiation: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsRadiation: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsSurfaceTraction& traction)
{
  try
  {

    decltype(traction.loads_data) saved_loads_data;
    decltype(traction.time_delay_data) saved_time_delay_data;
    decltype(traction.force_data) saved_force_data;
    decltype(traction.name_data) saved_name_data;
    if (file.nameExists(CalculiXModelSchema::path("Loads/SurfaceTraction/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/SurfaceTraction"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/SurfaceTraction/time_delay_data")) && !file.read_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/SurfaceTraction"), saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/SurfaceTraction/force_data")) && !file.read_dataset_double_rank_2("force_data", CalculiXModelSchema::path("Loads/SurfaceTraction"), saved_force_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/SurfaceTraction/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/SurfaceTraction"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 7)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_force_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: force_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 5, saved_force_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: invalid force_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 6, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: invalid or duplicate IDs.\n");
      return false;
    }
    traction.loads_data.swap(saved_loads_data);
    traction.time_delay_data.swap(saved_time_delay_data);
    traction.force_data.swap(saved_force_data);
    traction.name_data.swap(saved_name_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsSurfaceTraction: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsSurfaceTraction& traction)
{
  try
  {

    if (!(PersistenceTable::keyedRows(traction.loads_data, 7)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(traction.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(traction.force_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: force_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(traction.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(traction.loads_data, 3, traction.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(traction.loads_data, 5, traction.force_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: invalid force_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(traction.loads_data, 6, traction.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(traction.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/SurfaceTraction")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/SurfaceTraction"), traction.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/SurfaceTraction"), traction.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("force_data", CalculiXModelSchema::path("Loads/SurfaceTraction"), traction.force_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/SurfaceTraction"), traction.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsSurfaceTraction: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsBodyHeatflux& bodyheatflux)
{
  try
  {

    decltype(bodyheatflux.loads_data) saved_loads_data;
    decltype(bodyheatflux.time_delay_data) saved_time_delay_data;
    decltype(bodyheatflux.element_data) saved_element_data;
    decltype(bodyheatflux.magnitude_data) saved_magnitude_data;
    decltype(bodyheatflux.name_data) saved_name_data;
    if (file.nameExists(CalculiXModelSchema::path("Loads/BodyHeatflux/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/BodyHeatflux"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/BodyHeatflux/time_delay_data")) && !file.read_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/BodyHeatflux"), saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/BodyHeatflux/element_data")) && !file.read_dataset_int_rank_2("element_data", CalculiXModelSchema::path("Loads/BodyHeatflux"), saved_element_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/BodyHeatflux/magnitude_data")) && !file.read_dataset_double_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/BodyHeatflux"), saved_magnitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Loads/BodyHeatflux/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/BodyHeatflux"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 8)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_element_data, 1)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: element_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 6, saved_magnitude_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_loads_data, 7, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_loads_data)))
    {
      PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: invalid or duplicate IDs.\n");
      return false;
    }
    bodyheatflux.loads_data.swap(saved_loads_data);
    bodyheatflux.time_delay_data.swap(saved_time_delay_data);
    bodyheatflux.element_data.swap(saved_element_data);
    bodyheatflux.magnitude_data.swap(saved_magnitude_data);
    bodyheatflux.name_data.swap(saved_name_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX LoadsBodyHeatflux: %s\n", error.what());
    return false;
  }
}

bool CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsBodyHeatflux& bodyheatflux)
{
  try
  {

    if (!(PersistenceTable::keyedRows(bodyheatflux.loads_data, 8)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(bodyheatflux.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(bodyheatflux.element_data, 1)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: element_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(bodyheatflux.magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(bodyheatflux.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(bodyheatflux.loads_data, 3, bodyheatflux.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(bodyheatflux.loads_data, 6, bodyheatflux.magnitude_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(bodyheatflux.loads_data, 7, bodyheatflux.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(bodyheatflux.loads_data)))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Loads/BodyHeatflux")))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Loads/BodyHeatflux"), bodyheatflux.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", CalculiXModelSchema::path("Loads/BodyHeatflux"), bodyheatflux.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("element_data", CalculiXModelSchema::path("Loads/BodyHeatflux"), bodyheatflux.element_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("magnitude_data", CalculiXModelSchema::path("Loads/BodyHeatflux"), bodyheatflux.magnitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Loads/BodyHeatflux"), bodyheatflux.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX LoadsBodyHeatflux: %s\n", error.what());
    return false;
  }
}
