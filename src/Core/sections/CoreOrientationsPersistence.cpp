#include "Core/sections/CoreOrientationsPersistence.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "HDF5Tool.hpp"

void CoreOrientationsPersistence::read(HDF5Tool& file, CoreOrientations& orientations)
{
  file.read_dataset_int_rank_2("orientations_data", "Cubit-CalculiX/Orientations", orientations.orientations_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Orientations", orientations.name_data);
  file.read_dataset_string_rank_2("a_data", "Cubit-CalculiX/Orientations", orientations.a_data);
  file.read_dataset_string_rank_2("b_data", "Cubit-CalculiX/Orientations", orientations.b_data);
  file.read_dataset_string_rank_2("rotation_data", "Cubit-CalculiX/Orientations", orientations.rotation_data);
}

void CoreOrientationsPersistence::write(HDF5Tool& file, const CoreOrientations& orientations)
{
  file.createGroup("Cubit-CalculiX/Orientations");
  file.write_dataset_int_rank_2("orientations_data", "Cubit-CalculiX/Orientations", orientations.orientations_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Orientations", orientations.name_data);
  file.write_dataset_string_rank_2("a_data", "Cubit-CalculiX/Orientations", orientations.a_data);
  file.write_dataset_string_rank_2("b_data", "Cubit-CalculiX/Orientations", orientations.b_data);
  file.write_dataset_string_rank_2("rotation_data", "Cubit-CalculiX/Orientations", orientations.rotation_data);
}
