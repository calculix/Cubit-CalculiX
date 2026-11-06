#include "ModelFileMetadata.hpp"
#include "HDF5Tool.hpp"
#include <exception>
#include <vector>

bool CubitSimModelFile::read(HDF5Tool& file, std::string& applicationVersion, std::string& error)
{
  error.clear();
  try
  {
    std::vector<int> schema;
    std::vector<std::string> version;
    if (!file.nameExists(std::string(Metadata) + "/schema_version") ||
        !file.read_dataset_int_rank_1("schema_version", Metadata, schema) ||
        schema.size() != 1 || schema.front() != SchemaVersion)
    {
      error = "Unsupported or missing cubit-sim model schema.";
      return false;
    }
    if (!file.read_dataset_string_rank_1("application_version", Metadata, version) ||
        version.size() != 1 || version.front().empty())
    {
      error = "Missing cubit-sim application version.";
      return false;
    }
    applicationVersion = version.front();
    return true;
  }
  catch (const H5::Exception& exception)
  {
    error = exception.getDetailMsg();
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  return false;
}

bool CubitSimModelFile::write(HDF5Tool& file, const std::string& applicationVersion, std::string& error)
{
  error.clear();
  try
  {
    if (applicationVersion.empty())
    {
      error = "The cubit-sim application version is empty.";
      return false;
    }
    const bool existing = file.nameExists(Metadata);
    if (existing)
    {
      std::string previousVersion;
      if (!read(file, previousVersion, error))
      {
        return false;
      }
    }
    if (!file.ensureGroup(Metadata) ||
        (!existing && !file.write_dataset_int_rank_1("schema_version", Metadata, {SchemaVersion})) ||
        (existing && !file.deleteDataset("application_version", Metadata)) ||
        !file.write_dataset_string_rank_1("application_version", Metadata, {applicationVersion}))
    {
      error = "Cannot write cubit-sim model metadata: " + file.last_error();
      return false;
    }
    return true;
  }
  catch (const H5::Exception& exception)
  {
    error = exception.getDetailMsg();
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  return false;
}
