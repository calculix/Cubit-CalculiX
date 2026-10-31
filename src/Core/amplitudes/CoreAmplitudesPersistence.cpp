#include "Core/amplitudes/CoreAmplitudesPersistence.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "HDF5Tool.hpp"

void CoreAmplitudesPersistence::read(HDF5Tool& file, CoreAmplitudes& amplitudes)
{
  file.read_dataset_int_rank_2("amplitudes_data", "Cubit-CalculiX/Amplitudes", amplitudes.amplitudes_data);
  file.read_dataset_string_rank_2("name_amplitude_data", "Cubit-CalculiX/Amplitudes", amplitudes.name_amplitude_data);
  file.read_dataset_string_rank_2("shiftx_amplitude_data", "Cubit-CalculiX/Amplitudes", amplitudes.shiftx_amplitude_data);
  file.read_dataset_string_rank_2("shifty_amplitude_data", "Cubit-CalculiX/Amplitudes", amplitudes.shifty_amplitude_data);
  file.read_dataset_double_rank_2("amplitudevalues_amplitude_data", "Cubit-CalculiX/Amplitudes", amplitudes.amplitudevalues_amplitude_data);
}

void CoreAmplitudesPersistence::write(HDF5Tool& file, const CoreAmplitudes& amplitudes)
{
  file.createGroup("Cubit-CalculiX/Amplitudes");
  file.write_dataset_int_rank_2("amplitudes_data", "Cubit-CalculiX/Amplitudes", amplitudes.amplitudes_data);
  file.write_dataset_string_rank_2("name_amplitude_data", "Cubit-CalculiX/Amplitudes", amplitudes.name_amplitude_data);
  file.write_dataset_string_rank_2("shiftx_amplitude_data", "Cubit-CalculiX/Amplitudes", amplitudes.shiftx_amplitude_data);
  file.write_dataset_string_rank_2("shifty_amplitude_data", "Cubit-CalculiX/Amplitudes", amplitudes.shifty_amplitude_data);
  file.write_dataset_double_rank_2("amplitudevalues_amplitude_data", "Cubit-CalculiX/Amplitudes", amplitudes.amplitudevalues_amplitude_data);
}
