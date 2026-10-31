#include "Core/steps/CoreInitialConditionsPersistence.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "HDF5Tool.hpp"

void CoreInitialConditionsPersistence::read(HDF5Tool& file, CoreInitialConditions& conditions)
{
  file.read_dataset_int_rank_2("initialconditions_data", "Cubit-CalculiX/InitialConditions", conditions.initialconditions_data);
  file.read_dataset_string_rank_2("displacement_data", "Cubit-CalculiX/InitialConditions", conditions.displacement_data);
  file.read_dataset_string_rank_2("temperature_data", "Cubit-CalculiX/InitialConditions", conditions.temperature_data);
  file.read_dataset_int_rank_1("stress_data", "Cubit-CalculiX/InitialConditions", conditions.stress_data);
  file.read_dataset_double_rank_2("stress_block_data", "Cubit-CalculiX/InitialConditions", conditions.stress_block_data);
  file.read_dataset_double_rank_2("stress_element_data", "Cubit-CalculiX/InitialConditions", conditions.stress_element_data);
}

void CoreInitialConditionsPersistence::write(HDF5Tool& file, const CoreInitialConditions& conditions)
{
  file.createGroup("Cubit-CalculiX/InitialConditions");
  file.write_dataset_int_rank_2("initialconditions_data", "Cubit-CalculiX/InitialConditions", conditions.initialconditions_data);
  file.write_dataset_string_rank_2("displacement_data", "Cubit-CalculiX/InitialConditions", conditions.displacement_data);
  file.write_dataset_string_rank_2("temperature_data", "Cubit-CalculiX/InitialConditions", conditions.temperature_data);
  file.write_dataset_int_rank_1("stress_data", "Cubit-CalculiX/InitialConditions", conditions.stress_data);
  file.write_dataset_double_rank_2("stress_block_data", "Cubit-CalculiX/InitialConditions", conditions.stress_block_data);
  file.write_dataset_double_rank_2("stress_element_data", "Cubit-CalculiX/InitialConditions", conditions.stress_element_data);
}
