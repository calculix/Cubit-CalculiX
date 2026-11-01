#include "Core/mesh/CoreBlocksPersistence.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "HDF5Tool.hpp"

void CoreBlocksPersistence::read(HDF5Tool& file, CoreBlocks& blocks)
{
  file.read_dataset_int_rank_2("blocks_data", "Cubit-CalculiX/Blocks", blocks.blocks_data);
}

void CoreBlocksPersistence::write(HDF5Tool& file, const CoreBlocks& blocks)
{
  file.createGroup("Cubit-CalculiX/Blocks");
  file.write_dataset_int_rank_2("blocks_data", "Cubit-CalculiX/Blocks", blocks.blocks_data);
}
