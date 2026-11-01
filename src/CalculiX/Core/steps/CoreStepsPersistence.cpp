#include "Core/steps/CoreStepsPersistence.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "HDF5Tool.hpp"

void CoreStepsPersistence::read(HDF5Tool& file, CoreSteps& steps)
{
  file.read_dataset_int_rank_2("steps_data", "Cubit-CalculiX/Steps", steps.steps_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Steps", steps.name_data);
  file.read_dataset_string_rank_2("parameter_data", "Cubit-CalculiX/Steps", steps.parameter_data);
  file.read_dataset_string_rank_2("static_data", "Cubit-CalculiX/Steps", steps.static_data);
  file.read_dataset_string_rank_2("frequency_data", "Cubit-CalculiX/Steps", steps.frequency_data);
  file.read_dataset_string_rank_2("buckle_data", "Cubit-CalculiX/Steps", steps.buckle_data);
  file.read_dataset_string_rank_2("heattransfer_data", "Cubit-CalculiX/Steps", steps.heattransfer_data);
  file.read_dataset_string_rank_2("coupledtd_data", "Cubit-CalculiX/Steps", steps.coupledtd_data);
  file.read_dataset_string_rank_2("uncoupledtd_data", "Cubit-CalculiX/Steps", steps.uncoupledtd_data);
  file.read_dataset_string_rank_2("dynamic_data", "Cubit-CalculiX/Steps", steps.dynamic_data);
  file.read_dataset_string_rank_2("modal_dynamic_data", "Cubit-CalculiX/Steps", steps.modal_dynamic_data);
  file.read_dataset_string_rank_2("steady_state_dynamics_data", "Cubit-CalculiX/Steps", steps.steady_state_dynamics_data);
  file.read_dataset_string_rank_2("complex_frequency_data", "Cubit-CalculiX/Steps", steps.complex_frequency_data);
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Steps", steps.loads_data);
  file.read_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/Steps", steps.bcs_data);
  file.read_dataset_int_rank_2("historyoutputs_data", "Cubit-CalculiX/Steps", steps.historyoutputs_data);
  file.read_dataset_int_rank_2("fieldoutputs_data", "Cubit-CalculiX/Steps", steps.fieldoutputs_data);
}

void CoreStepsPersistence::write(HDF5Tool& file, const CoreSteps& steps)
{
  file.createGroup("Cubit-CalculiX/Steps");
  file.write_dataset_int_rank_2("steps_data", "Cubit-CalculiX/Steps", steps.steps_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Steps", steps.name_data);
  file.write_dataset_string_rank_2("parameter_data", "Cubit-CalculiX/Steps", steps.parameter_data);
  file.write_dataset_string_rank_2("static_data", "Cubit-CalculiX/Steps", steps.static_data);
  file.write_dataset_string_rank_2("frequency_data", "Cubit-CalculiX/Steps", steps.frequency_data);
  file.write_dataset_string_rank_2("buckle_data", "Cubit-CalculiX/Steps", steps.buckle_data);
  file.write_dataset_string_rank_2("heattransfer_data", "Cubit-CalculiX/Steps", steps.heattransfer_data);
  file.write_dataset_string_rank_2("coupledtd_data", "Cubit-CalculiX/Steps", steps.coupledtd_data);
  file.write_dataset_string_rank_2("uncoupledtd_data", "Cubit-CalculiX/Steps", steps.uncoupledtd_data);
  file.write_dataset_string_rank_2("dynamic_data", "Cubit-CalculiX/Steps", steps.dynamic_data);
  file.write_dataset_string_rank_2("modal_dynamic_data", "Cubit-CalculiX/Steps", steps.modal_dynamic_data);
  file.write_dataset_string_rank_2("steady_state_dynamics_data", "Cubit-CalculiX/Steps", steps.steady_state_dynamics_data);
  file.write_dataset_string_rank_2("complex_frequency_data", "Cubit-CalculiX/Steps", steps.complex_frequency_data);
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Steps", steps.loads_data);
  file.write_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/Steps", steps.bcs_data);
  file.write_dataset_int_rank_2("historyoutputs_data", "Cubit-CalculiX/Steps", steps.historyoutputs_data);
  file.write_dataset_int_rank_2("fieldoutputs_data", "Cubit-CalculiX/Steps", steps.fieldoutputs_data);
}
