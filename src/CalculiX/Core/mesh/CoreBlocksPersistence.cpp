#include "Core/mesh/CoreBlocksPersistence.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreBlocksPersistence::read(HDF5Tool& file, CoreBlocks& blocks)
{
  try
  {

    decltype(blocks.blocks_data) saved_blocks_data;
    if (file.nameExists("Cubit-CalculiX/Blocks/blocks_data") && !file.read_dataset_int_rank_2("blocks_data", "Cubit-CalculiX/Blocks", saved_blocks_data))
    {
      PRINT_ERROR("Cannot read CalculiX Blocks: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_blocks_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX Blocks: blocks_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_blocks_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Blocks: invalid or duplicate IDs.\n");
      return false;
    }
    blocks.blocks_data.swap(saved_blocks_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Blocks: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Blocks: %s\n", error.what());
    return false;
  }
}

bool CoreBlocksPersistence::write(HDF5Tool& file, const CoreBlocks& blocks)
{
  try
  {

    if (!(PersistenceTable::keyedRows(blocks.blocks_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX Blocks: blocks_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(blocks.blocks_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Blocks: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/Blocks"))
    {
      PRINT_ERROR("Cannot write CalculiX Blocks: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("blocks_data", "Cubit-CalculiX/Blocks", blocks.blocks_data))
    {
      PRINT_ERROR("Cannot write CalculiX Blocks: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Blocks: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Blocks: %s\n", error.what());
    return false;
  }
}
