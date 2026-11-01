#include "Core/steps/CoreStepSettingsPersistence.hpp"
#include "Core/steps/CoreDamping.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include "HDF5Tool.hpp"

void CoreStepSettingsPersistence::read(HDF5Tool& file, CoreDamping& damping)
{
  damping.damping_data.clear();
  file.read_dataset_string_rank_1("damping_data", "Cubit-CalculiX/Damping", damping.damping_data);

  if (damping.damping_data.empty())
  {
    damping.damping_data.assign(2, "");
  }
}

void CoreStepSettingsPersistence::write(HDF5Tool& file, const CoreDamping& damping)
{
  file.createGroup("Cubit-CalculiX/Damping");
  file.write_dataset_string_rank_1("damping_data", "Cubit-CalculiX/Damping", damping.damping_data);
}

void CoreStepSettingsPersistence::read(HDF5Tool& file, CorePhysicalConstants& constants)
{
  constants.physicalconstants_data.clear();
  file.read_dataset_string_rank_1("physicalconstants_data", "Cubit-CalculiX/PhysicalConstants", constants.physicalconstants_data);

  if (constants.physicalconstants_data.empty())
  {
    constants.physicalconstants_data.assign(3, "");
  }
}

void CoreStepSettingsPersistence::write(HDF5Tool& file, const CorePhysicalConstants& constants)
{
  file.createGroup("Cubit-CalculiX/PhysicalConstants");
  file.write_dataset_string_rank_1("physicalconstants_data", "Cubit-CalculiX/PhysicalConstants", constants.physicalconstants_data);
}
