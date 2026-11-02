#include "Core/steps/CoreStepSettingsPersistence.hpp"
#include "Core/steps/CoreDamping.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreStepSettingsPersistence::read(HDF5Tool& file, CoreDamping& damping)
{
  try
  {

    decltype(damping.damping_data) saved_damping_data;
    saved_damping_data.clear();
    if (file.nameExists("Cubit-CalculiX/Damping/damping_data") && !file.read_dataset_string_rank_1("damping_data", "Cubit-CalculiX/Damping", saved_damping_data))
    {
      PRINT_ERROR("Cannot read CalculiX Damping: %s\n", file.last_error().c_str());
      return false;
    }

    if (saved_damping_data.empty())
    {
      saved_damping_data.assign(2, "");
    }
    if (!(saved_damping_data.size() == 2))
    {
      PRINT_ERROR("Cannot read CalculiX Damping: damping requires two values.\n");
      return false;
    }
    damping.damping_data.swap(saved_damping_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Damping: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Damping: %s\n", error.what());
    return false;
  }
}

bool CoreStepSettingsPersistence::write(HDF5Tool& file, const CoreDamping& damping)
{
  try
  {

    if (!(damping.damping_data.size() == 2))
    {
      PRINT_ERROR("Cannot write CalculiX Damping: damping requires two values.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/Damping"))
    {
      PRINT_ERROR("Cannot write CalculiX Damping: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_1("damping_data", "Cubit-CalculiX/Damping", damping.damping_data))
    {
      PRINT_ERROR("Cannot write CalculiX Damping: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Damping: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Damping: %s\n", error.what());
    return false;
  }
}

bool CoreStepSettingsPersistence::read(HDF5Tool& file, CorePhysicalConstants& constants)
{
  try
  {

    decltype(constants.physicalconstants_data) saved_physicalconstants_data;
    saved_physicalconstants_data.clear();
    if (file.nameExists("Cubit-CalculiX/PhysicalConstants/physicalconstants_data") && !file.read_dataset_string_rank_1("physicalconstants_data", "Cubit-CalculiX/PhysicalConstants", saved_physicalconstants_data))
    {
      PRINT_ERROR("Cannot read CalculiX PhysicalConstants: %s\n", file.last_error().c_str());
      return false;
    }

    if (saved_physicalconstants_data.empty())
    {
      saved_physicalconstants_data.assign(3, "");
    }
    if (!(saved_physicalconstants_data.size() == 3))
    {
      PRINT_ERROR("Cannot read CalculiX PhysicalConstants: physical constants require three values.\n");
      return false;
    }
    constants.physicalconstants_data.swap(saved_physicalconstants_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX PhysicalConstants: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX PhysicalConstants: %s\n", error.what());
    return false;
  }
}

bool CoreStepSettingsPersistence::write(HDF5Tool& file, const CorePhysicalConstants& constants)
{
  try
  {

    if (!(constants.physicalconstants_data.size() == 3))
    {
      PRINT_ERROR("Cannot write CalculiX PhysicalConstants: physical constants require three values.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/PhysicalConstants"))
    {
      PRINT_ERROR("Cannot write CalculiX PhysicalConstants: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_1("physicalconstants_data", "Cubit-CalculiX/PhysicalConstants", constants.physicalconstants_data))
    {
      PRINT_ERROR("Cannot write CalculiX PhysicalConstants: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX PhysicalConstants: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX PhysicalConstants: %s\n", error.what());
    return false;
  }
}
