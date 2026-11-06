#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/steps/CoreStepsPersistence.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreStepsPersistence::read(HDF5Tool& file, CoreSteps& steps)
{
  try
  {

    decltype(steps.steps_data) saved_steps_data;
    decltype(steps.name_data) saved_name_data;
    decltype(steps.parameter_data) saved_parameter_data;
    decltype(steps.static_data) saved_static_data;
    decltype(steps.frequency_data) saved_frequency_data;
    decltype(steps.buckle_data) saved_buckle_data;
    decltype(steps.heattransfer_data) saved_heattransfer_data;
    decltype(steps.coupledtd_data) saved_coupledtd_data;
    decltype(steps.uncoupledtd_data) saved_uncoupledtd_data;
    decltype(steps.dynamic_data) saved_dynamic_data;
    decltype(steps.modal_dynamic_data) saved_modal_dynamic_data;
    decltype(steps.steady_state_dynamics_data) saved_steady_state_dynamics_data;
    decltype(steps.complex_frequency_data) saved_complex_frequency_data;
    decltype(steps.loads_data) saved_loads_data;
    decltype(steps.bcs_data) saved_bcs_data;
    decltype(steps.historyoutputs_data) saved_historyoutputs_data;
    decltype(steps.fieldoutputs_data) saved_fieldoutputs_data;
    if (file.nameExists(CalculiXModelSchema::path("Steps/steps_data")) && !file.read_dataset_int_rank_2("steps_data", CalculiXModelSchema::path("Steps"), saved_steps_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/name_data")) && !file.read_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Steps"), saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/parameter_data")) && !file.read_dataset_string_rank_2("parameter_data", CalculiXModelSchema::path("Steps"), saved_parameter_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/static_data")) && !file.read_dataset_string_rank_2("static_data", CalculiXModelSchema::path("Steps"), saved_static_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/frequency_data")) && !file.read_dataset_string_rank_2("frequency_data", CalculiXModelSchema::path("Steps"), saved_frequency_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/buckle_data")) && !file.read_dataset_string_rank_2("buckle_data", CalculiXModelSchema::path("Steps"), saved_buckle_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/heattransfer_data")) && !file.read_dataset_string_rank_2("heattransfer_data", CalculiXModelSchema::path("Steps"), saved_heattransfer_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/coupledtd_data")) && !file.read_dataset_string_rank_2("coupledtd_data", CalculiXModelSchema::path("Steps"), saved_coupledtd_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/uncoupledtd_data")) && !file.read_dataset_string_rank_2("uncoupledtd_data", CalculiXModelSchema::path("Steps"), saved_uncoupledtd_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/dynamic_data")) && !file.read_dataset_string_rank_2("dynamic_data", CalculiXModelSchema::path("Steps"), saved_dynamic_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/modal_dynamic_data")) && !file.read_dataset_string_rank_2("modal_dynamic_data", CalculiXModelSchema::path("Steps"), saved_modal_dynamic_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/steady_state_dynamics_data")) && !file.read_dataset_string_rank_2("steady_state_dynamics_data", CalculiXModelSchema::path("Steps"), saved_steady_state_dynamics_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/complex_frequency_data")) && !file.read_dataset_string_rank_2("complex_frequency_data", CalculiXModelSchema::path("Steps"), saved_complex_frequency_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/loads_data")) && !file.read_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Steps"), saved_loads_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/bcs_data")) && !file.read_dataset_int_rank_2("bcs_data", CalculiXModelSchema::path("Steps"), saved_bcs_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/historyoutputs_data")) && !file.read_dataset_int_rank_2("historyoutputs_data", CalculiXModelSchema::path("Steps"), saved_historyoutputs_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists(CalculiXModelSchema::path("Steps/fieldoutputs_data")) && !file.read_dataset_int_rank_2("fieldoutputs_data", CalculiXModelSchema::path("Steps"), saved_fieldoutputs_data))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_steps_data, 9)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: steps_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_parameter_data, 8)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: parameter_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_static_data, 11)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: static_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_frequency_data, 8)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: frequency_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_buckle_data, 6)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: buckle_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_heattransfer_data, 17)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: heattransfer_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_coupledtd_data, 14)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: coupledtd_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_uncoupledtd_data, 13)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: uncoupledtd_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_dynamic_data, 11)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: dynamic_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_modal_dynamic_data, 9)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: modal_dynamic_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_steady_state_dynamics_data, 10)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: steady_state_dynamics_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_complex_frequency_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: complex_frequency_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_loads_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_bcs_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: bcs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_historyoutputs_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: historyoutputs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_fieldoutputs_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: fieldoutputs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_steps_data, 1, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_steps_data, 3, 2, 4, saved_static_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid static_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_steps_data, 3, 3, 4, saved_frequency_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid frequency_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_steps_data, 3, 4, 4, saved_buckle_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid buckle_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_steps_data, 3, 5, 4, saved_heattransfer_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid heattransfer_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_steps_data, 3, 6, 4, saved_coupledtd_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid coupledtd_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_steps_data, 3, 7, 4, saved_uncoupledtd_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid uncoupledtd_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_steps_data, 3, 8, 4, saved_dynamic_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid dynamic_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_steps_data, 3, 9, 4, saved_modal_dynamic_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid modal_dynamic_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_steps_data, 3, 10, 4, saved_steady_state_dynamics_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid steady_state_dynamics_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_steps_data, 3, 11, 4, saved_complex_frequency_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid complex_frequency_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(saved_steps_data, 3, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11})))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_steps_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Steps: invalid or duplicate IDs.\n");
      return false;
    }
    steps.steps_data.swap(saved_steps_data);
    steps.name_data.swap(saved_name_data);
    steps.parameter_data.swap(saved_parameter_data);
    steps.static_data.swap(saved_static_data);
    steps.frequency_data.swap(saved_frequency_data);
    steps.buckle_data.swap(saved_buckle_data);
    steps.heattransfer_data.swap(saved_heattransfer_data);
    steps.coupledtd_data.swap(saved_coupledtd_data);
    steps.uncoupledtd_data.swap(saved_uncoupledtd_data);
    steps.dynamic_data.swap(saved_dynamic_data);
    steps.modal_dynamic_data.swap(saved_modal_dynamic_data);
    steps.steady_state_dynamics_data.swap(saved_steady_state_dynamics_data);
    steps.complex_frequency_data.swap(saved_complex_frequency_data);
    steps.loads_data.swap(saved_loads_data);
    steps.bcs_data.swap(saved_bcs_data);
    steps.historyoutputs_data.swap(saved_historyoutputs_data);
    steps.fieldoutputs_data.swap(saved_fieldoutputs_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Steps: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Steps: %s\n", error.what());
    return false;
  }
}

bool CoreStepsPersistence::write(HDF5Tool& file, const CoreSteps& steps)
{
  try
  {

    if (!(PersistenceTable::keyedRows(steps.steps_data, 9)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: steps_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.parameter_data, 8)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: parameter_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.static_data, 11)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: static_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.frequency_data, 8)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: frequency_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.buckle_data, 6)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: buckle_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.heattransfer_data, 17)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: heattransfer_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.coupledtd_data, 14)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: coupledtd_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.uncoupledtd_data, 13)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: uncoupledtd_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.dynamic_data, 11)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: dynamic_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.modal_dynamic_data, 9)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: modal_dynamic_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.steady_state_dynamics_data, 10)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: steady_state_dynamics_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.complex_frequency_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: complex_frequency_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.loads_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: loads_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.bcs_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: bcs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.historyoutputs_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: historyoutputs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(steps.fieldoutputs_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: fieldoutputs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(steps.steps_data, 1, steps.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(steps.steps_data, 3, 2, 4, steps.static_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid static_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(steps.steps_data, 3, 3, 4, steps.frequency_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid frequency_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(steps.steps_data, 3, 4, 4, steps.buckle_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid buckle_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(steps.steps_data, 3, 5, 4, steps.heattransfer_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid heattransfer_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(steps.steps_data, 3, 6, 4, steps.coupledtd_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid coupledtd_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(steps.steps_data, 3, 7, 4, steps.uncoupledtd_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid uncoupledtd_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(steps.steps_data, 3, 8, 4, steps.dynamic_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid dynamic_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(steps.steps_data, 3, 9, 4, steps.modal_dynamic_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid modal_dynamic_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(steps.steps_data, 3, 10, 4, steps.steady_state_dynamics_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid steady_state_dynamics_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(steps.steps_data, 3, 11, 4, steps.complex_frequency_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid complex_frequency_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(steps.steps_data, 3, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11})))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(steps.steps_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup(CalculiXModelSchema::path("Steps")))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("steps_data", CalculiXModelSchema::path("Steps"), steps.steps_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", CalculiXModelSchema::path("Steps"), steps.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("parameter_data", CalculiXModelSchema::path("Steps"), steps.parameter_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("static_data", CalculiXModelSchema::path("Steps"), steps.static_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("frequency_data", CalculiXModelSchema::path("Steps"), steps.frequency_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("buckle_data", CalculiXModelSchema::path("Steps"), steps.buckle_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("heattransfer_data", CalculiXModelSchema::path("Steps"), steps.heattransfer_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("coupledtd_data", CalculiXModelSchema::path("Steps"), steps.coupledtd_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("uncoupledtd_data", CalculiXModelSchema::path("Steps"), steps.uncoupledtd_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("dynamic_data", CalculiXModelSchema::path("Steps"), steps.dynamic_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("modal_dynamic_data", CalculiXModelSchema::path("Steps"), steps.modal_dynamic_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("steady_state_dynamics_data", CalculiXModelSchema::path("Steps"), steps.steady_state_dynamics_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("complex_frequency_data", CalculiXModelSchema::path("Steps"), steps.complex_frequency_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("loads_data", CalculiXModelSchema::path("Steps"), steps.loads_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("bcs_data", CalculiXModelSchema::path("Steps"), steps.bcs_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("historyoutputs_data", CalculiXModelSchema::path("Steps"), steps.historyoutputs_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("fieldoutputs_data", CalculiXModelSchema::path("Steps"), steps.fieldoutputs_data))
    {
      PRINT_ERROR("Cannot write CalculiX Steps: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Steps: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Steps: %s\n", error.what());
    return false;
  }
}
