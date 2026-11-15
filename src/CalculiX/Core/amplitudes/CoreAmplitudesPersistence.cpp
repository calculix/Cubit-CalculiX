#include "Core/units/CalculiXDomainUnits.hpp"
#include "Core/units/CalculiXPhysicalModelUnits.hpp"
#include <utility>
#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/amplitudes/CoreAmplitudesPersistence.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreAmplitudesPersistence::read(HDF5Tool& file, CoreAmplitudes& amplitudes)
{
  if (!amplitudes.domain_unit_view) {
    CoreAmplitudes candidate;
    candidate.is_initialized = amplitudes.is_initialized;
    candidate.domain_unit_view = true;
    if (!read(file, candidate)) {
      return false;
    }
    std::vector<std::vector<std::string>> axes;
    const std::string group = CalculiXModelSchema::path("Amplitudes");
    if (file.nameExists(group + "/axis_quantities")) {
      if (!file.read_dataset_string_rank_2("axis_quantities", group, axes)) {
        PRINT_ERROR("Cannot read amplitude axes.\n");
        return false;
      }
      for (const auto& row : axes) {
        if (row.size() != 2 || (row[1] != "time" && row[1] != "frequency")) {
          PRINT_ERROR("Invalid persisted amplitude axis.\n");
          return false;
        }
        try {
          std::size_t consumed = 0;
          const int id = std::stoi(row[0], &consumed);
          if (consumed != row[0].size() || !candidate.check_amplitude_exists(id) || !candidate.axis_quantities.emplace(id, row[1]).second) {
            PRINT_ERROR("Invalid persisted amplitude axis reference.\n");
            return false;
          }
        } catch (const std::exception& exception) {
          PRINT_ERROR("Invalid amplitude axis: %s\n", exception.what());
          return false;
        }
      }
    }
    std::string basis;
    std::string error;
    if (!CalculiXPhysicalModelUnits::read_basis(file, CalculiXModelSchema::path("Amplitudes"), basis, error)) {
      PRINT_ERROR("%s\n", error.c_str());
      return false;
    }
    if (basis.empty() && !CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Native, error)) {
      PRINT_ERROR("Cannot restore Amplitudes units: %s\n", error.c_str());
      return false;
    }
    auto validated = candidate;
    if (!CalculiXDomainUnits::convert(validated, CalculiXDomainUnitView::Solver, error)) {
      PRINT_ERROR("Cannot restore invalid physical values: %s\n", error.c_str());
      return false;
    }
    candidate.domain_unit_view = false;
    amplitudes = std::move(candidate);
    return true;
  }

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
  std::vector<std::vector<std::string>> axes;
  for (const auto& amplitude : amplitudes.amplitudes_data) {
    if (amplitude.empty()) {
      PRINT_ERROR("Cannot save an incomplete amplitude.\n");
      return false;
    }
    std::string quantity = "time";
    const auto found = amplitudes.axis_quantities.find(amplitude[0]);
    if (found != amplitudes.axis_quantities.end()) {
      quantity = found->second;
    }
    axes.push_back({std::to_string(amplitude[0]), quantity});
  }
  if (!file.ensureGroup(CalculiXModelSchema::path("Amplitudes")) || !file.write_dataset_string_rank_2("axis_quantities", CalculiXModelSchema::path("Amplitudes"), axes)) {
    PRINT_ERROR("Cannot save amplitude axes.\n");
    return false;
  }
  std::string unitError;
  if (amplitudes.domain_unit_view || !CalculiXPhysicalModelUnits::write_basis(file, CalculiXModelSchema::path("Amplitudes"), "SI", unitError)) {
    PRINT_ERROR("Cannot save Amplitudes canonical units: %s\n", unitError.c_str());
    return false;
  }

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
