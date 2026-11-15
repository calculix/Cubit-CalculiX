#include "Core/units/CalculiXDomainUnits.hpp"
#include "Core/units/CalculiXPhysicalModelUnits.hpp"
#include <utility>
#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/steps/CoreStepSettingsPersistence.hpp"
#include "Core/steps/CoreDamping.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreStepSettingsPersistence::read(HDF5Tool& file, CoreDamping& damping)
{
  if (!damping.domain_unit_view) {
    CoreDamping candidate;
    candidate.is_initialized = damping.is_initialized;
    candidate.domain_unit_view = true;
    if (!read(file, candidate)) {
      return false;
    }
    std::string basis;
    std::string error;
    if (!CalculiXPhysicalModelUnits::read_basis(file, CalculiXModelSchema::path("Damping"), basis, error)) {
      PRINT_ERROR("%s\n", error.c_str());
      return false;
    }
    if (basis.empty() && !CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Native, error)) {
      PRINT_ERROR("Cannot restore Damping units: %s\n", error.c_str());
      return false;
    }
    auto validated = candidate;
    if (!CalculiXDomainUnits::convert(validated, CalculiXDomainUnitView::Solver, error)) {
      PRINT_ERROR("Cannot restore invalid physical values: %s\n", error.c_str());
      return false;
    }
    candidate.domain_unit_view = false;
    damping = std::move(candidate);
    return true;
  }

  try
  {

    decltype(damping.damping_data) saved_damping_data;
    saved_damping_data.clear();
    if (file.nameExists(CalculiXModelSchema::path("Damping/damping_data")) && !file.read_dataset_string_rank_1("damping_data", CalculiXModelSchema::path("Damping"), saved_damping_data))
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
  std::string unitError;
  if (damping.domain_unit_view || !CalculiXPhysicalModelUnits::write_basis(file, CalculiXModelSchema::path("Damping"), "SI", unitError)) {
    PRINT_ERROR("Cannot save Damping canonical units: %s\n", unitError.c_str());
    return false;
  }

  try
  {

    if (!(damping.damping_data.size() == 2))
    {
      PRINT_ERROR("Cannot write CalculiX Damping: damping requires two values.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Damping")))
    {
      PRINT_ERROR("Cannot write CalculiX Damping: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_1("damping_data", CalculiXModelSchema::path("Damping"), damping.damping_data))
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
  if (!constants.domain_unit_view) {
    CorePhysicalConstants candidate;
    candidate.is_initialized = constants.is_initialized;
    candidate.domain_unit_view = true;
    if (!read(file, candidate)) {
      return false;
    }
    std::string basis;
    std::string error;
    if (!CalculiXPhysicalModelUnits::read_basis(file, CalculiXModelSchema::path("PhysicalConstants"), basis, error)) {
      PRINT_ERROR("%s\n", error.c_str());
      return false;
    }
    if (basis.empty() && !CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Native, error)) {
      PRINT_ERROR("Cannot restore PhysicalConstants units: %s\n", error.c_str());
      return false;
    }
    auto validated = candidate;
    if (!CalculiXDomainUnits::convert(validated, CalculiXDomainUnitView::Solver, error)) {
      PRINT_ERROR("Cannot restore invalid physical values: %s\n", error.c_str());
      return false;
    }
    candidate.domain_unit_view = false;
    constants = std::move(candidate);
    return true;
  }

  try
  {

    decltype(constants.physicalconstants_data) saved_physicalconstants_data;
    saved_physicalconstants_data.clear();
    if (file.nameExists(CalculiXModelSchema::path("PhysicalConstants/physicalconstants_data")) && !file.read_dataset_string_rank_1("physicalconstants_data", CalculiXModelSchema::path("PhysicalConstants"), saved_physicalconstants_data))
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
  std::string unitError;
  if (constants.domain_unit_view || !CalculiXPhysicalModelUnits::write_basis(file, CalculiXModelSchema::path("PhysicalConstants"), "SI", unitError)) {
    PRINT_ERROR("Cannot save PhysicalConstants canonical units: %s\n", unitError.c_str());
    return false;
  }

  try
  {

    if (!(constants.physicalconstants_data.size() == 3))
    {
      PRINT_ERROR("Cannot write CalculiX PhysicalConstants: physical constants require three values.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("PhysicalConstants")))
    {
      PRINT_ERROR("Cannot write CalculiX PhysicalConstants: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_1("physicalconstants_data", CalculiXModelSchema::path("PhysicalConstants"), constants.physicalconstants_data))
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
