#include "CoreModelMetadataPersistence.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <vector>

bool CoreModelMetadataPersistence::read(HDF5Tool& file)
{
  try
  {
    std::vector<std::string> general;
    if (file.nameExists("Cubit-CalculiX/general") && !file.read_dataset_string_rank_1("general", "Cubit-CalculiX", general))
    {
      PRINT_ERROR("Cannot read CalculiX model metadata: %s\n", file.last_error().c_str());
      return false;
    }
    if (!general.empty())
    {
      PRINT_INFO("Save file was created with Cubit-CalculiX %s\n", general.front().c_str());
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX model metadata: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX model metadata: %s\n", error.what());
    return false;
  }
}

bool CoreModelMetadataPersistence::write(HDF5Tool& file, const std::string& version)
{
  try
  {
    if (!file.ensureGroup("Cubit-CalculiX") || !file.ensureGroup("Cubit-CalculiX/Core") || !file.ensureGroup("Cubit-CalculiX/Materials") || !file.write_dataset_string_rank_1("general", "Cubit-CalculiX", {version}))
    {
      PRINT_ERROR("Cannot save CalculiX model metadata: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot save CalculiX model metadata: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot save CalculiX model metadata: %s\n", error.what());
    return false;
  }
}
