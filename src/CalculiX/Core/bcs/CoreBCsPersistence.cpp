#include "Core/bcs/CoreBCsPersistence.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/bcs/CoreHBCs.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreBCsPersistence::read(HDF5Tool& file, CoreBCsDisplacements& displacements)
{
  try
  {

    decltype(displacements.bcs_data) saved_bcs_data;
    decltype(displacements.time_delay_data) saved_time_delay_data;
    decltype(displacements.displacement_data) saved_displacement_data;
    decltype(displacements.mode_data) saved_mode_data;
    decltype(displacements.name_data) saved_name_data;
    if (file.nameExists("Cubit-CalculiX/BCs/Displacements/bcs_data") && !file.read_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/BCs/Displacements", saved_bcs_data))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/BCs/Displacements/time_delay_data") && !file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/BCs/Displacements", saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/BCs/Displacements/displacement_data") && !file.read_dataset_double_rank_2("displacement_data", "Cubit-CalculiX/BCs/Displacements", saved_displacement_data))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/BCs/Displacements/mode_data") && !file.read_dataset_int_rank_2("mode_data", "Cubit-CalculiX/BCs/Displacements", saved_mode_data))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/BCs/Displacements/name_data") && !file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/BCs/Displacements", saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_bcs_data, 7)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: bcs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_displacement_data, 7)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: displacement_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_mode_data, 7)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: mode_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_bcs_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_bcs_data, 5, saved_displacement_data)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: invalid displacement_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_bcs_data, 6, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_bcs_data)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsDisplacements: invalid or duplicate IDs.\n");
      return false;
    }
    displacements.bcs_data.swap(saved_bcs_data);
    displacements.time_delay_data.swap(saved_time_delay_data);
    displacements.displacement_data.swap(saved_displacement_data);
    displacements.mode_data.swap(saved_mode_data);
    displacements.name_data.swap(saved_name_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX BCsDisplacements: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX BCsDisplacements: %s\n", error.what());
    return false;
  }
}

bool CoreBCsPersistence::write(HDF5Tool& file, const CoreBCsDisplacements& displacements)
{
  try
  {

    if (!(PersistenceTable::keyedRows(displacements.bcs_data, 7)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: bcs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(displacements.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(displacements.displacement_data, 7)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: displacement_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(displacements.mode_data, 7)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: mode_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(displacements.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(displacements.bcs_data, 3, displacements.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(displacements.bcs_data, 5, displacements.displacement_data)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: invalid displacement_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(displacements.bcs_data, 6, displacements.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(displacements.bcs_data)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/BCs/Displacements"))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/BCs/Displacements", displacements.bcs_data))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/BCs/Displacements", displacements.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("displacement_data", "Cubit-CalculiX/BCs/Displacements", displacements.displacement_data))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("mode_data", "Cubit-CalculiX/BCs/Displacements", displacements.mode_data))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/BCs/Displacements", displacements.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX BCsDisplacements: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX BCsDisplacements: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX BCsDisplacements: %s\n", error.what());
    return false;
  }
}

bool CoreBCsPersistence::read(HDF5Tool& file, CoreBCsTemperatures& temperatures)
{
  try
  {

    decltype(temperatures.bcs_data) saved_bcs_data;
    decltype(temperatures.time_delay_data) saved_time_delay_data;
    decltype(temperatures.magnitude_data) saved_magnitude_data;
    decltype(temperatures.name_data) saved_name_data;
    if (file.nameExists("Cubit-CalculiX/BCs/Temperatures/bcs_data") && !file.read_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/BCs/Temperatures", saved_bcs_data))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/BCs/Temperatures/time_delay_data") && !file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/BCs/Temperatures", saved_time_delay_data))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/BCs/Temperatures/magnitude_data") && !file.read_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/BCs/Temperatures", saved_magnitude_data))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/BCs/Temperatures/name_data") && !file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/BCs/Temperatures", saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_bcs_data, 7)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: bcs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_bcs_data, 3, saved_time_delay_data)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_bcs_data, 5, saved_magnitude_data)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_bcs_data, 6, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_bcs_data)))
    {
      PRINT_ERROR("Cannot read CalculiX BCsTemperatures: invalid or duplicate IDs.\n");
      return false;
    }
    temperatures.bcs_data.swap(saved_bcs_data);
    temperatures.time_delay_data.swap(saved_time_delay_data);
    temperatures.magnitude_data.swap(saved_magnitude_data);
    temperatures.name_data.swap(saved_name_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX BCsTemperatures: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX BCsTemperatures: %s\n", error.what());
    return false;
  }
}

bool CoreBCsPersistence::write(HDF5Tool& file, const CoreBCsTemperatures& temperatures)
{
  try
  {

    if (!(PersistenceTable::keyedRows(temperatures.bcs_data, 7)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: bcs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(temperatures.time_delay_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: time_delay_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(temperatures.magnitude_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: magnitude_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(temperatures.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(temperatures.bcs_data, 3, temperatures.time_delay_data)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: invalid time_delay_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(temperatures.bcs_data, 5, temperatures.magnitude_data)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: invalid magnitude_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(temperatures.bcs_data, 6, temperatures.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(temperatures.bcs_data)))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/BCs/Temperatures"))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.bcs_data))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.time_delay_data))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.magnitude_data))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX BCsTemperatures: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX BCsTemperatures: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX BCsTemperatures: %s\n", error.what());
    return false;
  }
}

bool CoreBCsPersistence::read(HDF5Tool& file, CoreHBCs& hbcs)
{
  try
  {

    decltype(hbcs.bcs_data) saved_bcs_data;
    if (file.nameExists("Cubit-CalculiX/HBCs/bcs_data") && !file.read_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/HBCs", saved_bcs_data))
    {
      PRINT_ERROR("Cannot read CalculiX HBCs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_bcs_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX HBCs: bcs_data has incomplete rows.\n");
      return false;
    }
    hbcs.bcs_data.swap(saved_bcs_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX HBCs: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX HBCs: %s\n", error.what());
    return false;
  }
}

bool CoreBCsPersistence::write(HDF5Tool& file, const CoreHBCs& hbcs)
{
  try
  {

    if (!(PersistenceTable::keyedRows(hbcs.bcs_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX HBCs: bcs_data has incomplete rows.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/HBCs"))
    {
      PRINT_ERROR("Cannot write CalculiX HBCs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/HBCs", hbcs.bcs_data))
    {
      PRINT_ERROR("Cannot write CalculiX HBCs: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX HBCs: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX HBCs: %s\n", error.what());
    return false;
  }
}
