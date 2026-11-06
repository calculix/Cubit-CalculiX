#include "CoreModelMetadataPersistence.hpp"
#include "CalculiXModelSchema.hpp"
#include "Framework/Core/CubitSimVersion.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <vector>

bool CoreModelMetadataPersistence::read(HDF5Tool& file)
{
  try
  {
    std::string applicationVersion;
    std::string error;
    if (!CubitSimModelFile::read(file, applicationVersion, error))
    {
      PRINT_ERROR("Cannot read cubit-sim model metadata: %s\n", error.c_str());
      return false;
    }
    const auto metadata = CalculiXModelSchema::path("metadata");
    std::vector<int> schema;
    std::vector<std::string> version;
    if (!file.read_dataset_int_rank_1("schema_version", metadata, schema) ||
        schema.size() != 1 || schema.front() != CalculiXModelSchema::SchemaVersion ||
        !file.read_dataset_string_rank_1("workspace_version", metadata, version) ||
        version.size() != 1 || version.front().empty())
    {
      PRINT_ERROR("Unsupported or incomplete CalculiX workspace model schema.\n");
      return false;
    }
    PRINT_INFO("Model saved by cubit-sim %s; CalculiX workspace %s.\n",
      applicationVersion.c_str(), version.front().c_str());
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX model metadata: %s\n", error.getDetailMsg().c_str());
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX model metadata: %s\n", error.what());
  }
  return false;
}

bool CoreModelMetadataPersistence::write(HDF5Tool& file, const std::string& version)
{
  try
  {
    std::string error;
    if (!CubitSimModelFile::write(file, CubitSim::Version, error))
    {
      PRINT_ERROR("Cannot save cubit-sim model metadata: %s\n", error.c_str());
      return false;
    }
    const auto metadata = CalculiXModelSchema::path("metadata");
    if (!file.ensureGroup(metadata) ||
        !file.ensureGroup(CalculiXModelSchema::path("Core")) ||
        !file.ensureGroup(CalculiXModelSchema::path("Materials")) ||
        !file.write_dataset_int_rank_1("schema_version", metadata, {CalculiXModelSchema::SchemaVersion}) ||
        !file.write_dataset_string_rank_1("workspace_version", metadata, {version}))
    {
      PRINT_ERROR("Cannot save CalculiX model metadata: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot save CalculiX model metadata: %s\n", error.getDetailMsg().c_str());
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot save CalculiX model metadata: %s\n", error.what());
  }
  return false;
}
