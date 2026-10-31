#include "Core/bcs/CoreBCsPersistence.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/bcs/CoreHBCs.hpp"
#include "HDF5Tool.hpp"

void CoreBCsPersistence::read(HDF5Tool& file, CoreBCsDisplacements& displacements)
{
  file.read_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/BCs/Displacements", displacements.bcs_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/BCs/Displacements", displacements.time_delay_data);
  file.read_dataset_double_rank_2("displacement_data", "Cubit-CalculiX/BCs/Displacements", displacements.displacement_data);
  file.read_dataset_int_rank_2("mode_data", "Cubit-CalculiX/BCs/Displacements", displacements.mode_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/BCs/Displacements", displacements.name_data);
}

void CoreBCsPersistence::write(HDF5Tool& file, const CoreBCsDisplacements& displacements)
{
  file.createGroup("Cubit-CalculiX/BCs/Displacements");
  file.write_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/BCs/Displacements", displacements.bcs_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/BCs/Displacements", displacements.time_delay_data);
  file.write_dataset_double_rank_2("displacement_data", "Cubit-CalculiX/BCs/Displacements", displacements.displacement_data);
  file.write_dataset_int_rank_2("mode_data", "Cubit-CalculiX/BCs/Displacements", displacements.mode_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/BCs/Displacements", displacements.name_data);
}

void CoreBCsPersistence::read(HDF5Tool& file, CoreBCsTemperatures& temperatures)
{
  file.read_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.bcs_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.time_delay_data);
  file.read_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.magnitude_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.name_data);
}

void CoreBCsPersistence::write(HDF5Tool& file, const CoreBCsTemperatures& temperatures)
{
  file.createGroup("Cubit-CalculiX/BCs/Temperatures");
  file.write_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.bcs_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.time_delay_data);
  file.write_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.magnitude_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/BCs/Temperatures", temperatures.name_data);
}

void CoreBCsPersistence::read(HDF5Tool& file, CoreHBCs& hbcs)
{
  file.read_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/HBCs", hbcs.bcs_data);
}

void CoreBCsPersistence::write(HDF5Tool& file, const CoreHBCs& hbcs)
{
  file.createGroup("Cubit-CalculiX/HBCs");
  file.write_dataset_int_rank_2("bcs_data", "Cubit-CalculiX/HBCs", hbcs.bcs_data);
}
