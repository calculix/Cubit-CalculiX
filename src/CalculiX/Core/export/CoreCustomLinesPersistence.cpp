#include "Core/export/CoreCustomLinesPersistence.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "HDF5Tool.hpp"

void CoreCustomLinesPersistence::read(HDF5Tool& file, CoreCustomLines& lines)
{
  file.read_dataset_string_rank_2("customlines_data", "Cubit-CalculiX/CustomLines", lines.customlines_data);
}

void CoreCustomLinesPersistence::write(HDF5Tool& file, const CoreCustomLines& lines)
{
  file.createGroup("Cubit-CalculiX/CustomLines");
  file.write_dataset_string_rank_2("customlines_data", "Cubit-CalculiX/CustomLines", lines.customlines_data);
}
