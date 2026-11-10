#include "PostprocessorProjectPersistence.hpp"
#include "PostprocessorSessionCodec.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include "Framework/Storage/ModelFileMetadata.hpp"
#include "Framework/Core/CubitSimVersion.hpp"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <exception>
namespace
{
  const std::string root = "cubit-sim/workspaces/postprocessor";
}
bool Postprocessor::PostprocessorProjectPersistence::read(const std::string& filename,
  PostprocessorProjectState& output, std::string& error) const
{
  error.clear();
  try
  {
    HDF5Tool file(filename,false);
    const auto effective = file.nameExists(root) ? root : file.nameExists(root+"-previous") ? root+"-previous" : std::string{};
    if (effective.empty())
    {
      if (!file.close())
      {
        error = "Cannot close project file.";
        return false;
      }
      output = PostprocessorProjectState{};
      return true;
    }
    // Bound HDF5 dimensions/string storage before allocating the JSON payload.
    {
      auto dataset = file.file->openDataSet(effective+"/session");
      auto space = dataset.getSpace();
      hsize_t dimensions[1] = {0};
      if (space.getSimpleExtentNdims() != 1 || space.getSimpleExtentDims(dimensions) != 1 || dimensions[0] != 1)
      {
        error = "Invalid Postprocessor payload dimensions.";
        return false;
      }
      auto type = dataset.getDataType();
      if (type.getClass() != H5T_STRING)
      {
        error = "Postprocessor payload is not a string dataset.";
        return false;
      }
      const auto bytes = H5Tis_variable_str(type.getId()) > 0 ? dataset.getVlenBufSize(type,space) : static_cast<hsize_t>(type.getSize());
      if (bytes > PersistenceDefaults::maximum_payload_bytes+1)
      {
        error = "Postprocessor payload exceeds the configured size limit.";
        return false;
      }
      auto schemaData = file.file->openDataSet(effective+"/schema_version");
      auto schemaSpace = schemaData.getSpace();
      if (schemaSpace.getSimpleExtentNdims() != 1 || schemaSpace.getSimpleExtentDims(dimensions) != 1 || dimensions[0] != 1)
      {
        error = "Invalid Postprocessor schema dimensions.";
        return false;
      }
    }
    std::vector<int> schema;
    std::vector<std::string> payload;
    std::string version;
    if (!CubitSimModelFile::read(file,version,error) || !file.read_dataset_int_rank_1("schema_version",effective,schema) ||
        schema != std::vector<int>{PersistenceDefaults::schema_version} || !file.read_dataset_string_rank_1("session",effective,payload) ||
        payload.size() != 1 || payload.front().size() > PersistenceDefaults::maximum_payload_bytes)
    {
      if (error.empty())
      {
        error = "Unsupported or oversized Postprocessor project data.";
      }
      return false;
    }
    if (!file.close())
    {
      error = "Cannot close project file.";
      return false;
    }
    QJsonParseError parse;
    const auto document = QJsonDocument::fromJson(QByteArray::fromStdString(payload.front()),&parse);
    if (parse.error != QJsonParseError::NoError || !document.isObject())
    {
      error = "Malformed Postprocessor session JSON.";
      return false;
    }
    return project_from_json(document.object(),QFileInfo(QString::fromUtf8(filename.c_str())).absolutePath().toUtf8().toStdString(),output,error);
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
bool Postprocessor::PostprocessorProjectPersistence::write(const std::string& filename,
  const PostprocessorProjectState& state, std::string& error) const
{
  if (!validate_project(state,error))
  {
    return false;
  }
  try
  {
    const auto directory = QFileInfo(QString::fromUtf8(filename.c_str())).absolutePath().toUtf8().toStdString();
    const auto payload = QJsonDocument(project_to_json(state,directory)).toJson(QJsonDocument::Compact);
    if (static_cast<std::size_t>(payload.size()) > PersistenceDefaults::maximum_payload_bytes)
    {
      error = "Postprocessor project definitions exceed the configured size limit.";
      return false;
    }
    HDF5Tool file(filename,false);
    const auto staged = root+"-staged", previous = root+"-previous";
    if (!file.nameExists(root) && file.nameExists(previous) && !file.renameGroup(previous,root))
    {
      error = "Cannot recover the previous committed Postprocessor definitions.";
      return false;
    }
    if (!CubitSimModelFile::write(file,CubitSim::Version,error) ||
        (file.nameExists(staged) && !file.deleteGroup(staged)) || !file.ensureGroup(staged) ||
        !file.write_dataset_int_rank_1("schema_version",staged,{PersistenceDefaults::schema_version}) ||
        !file.write_dataset_string_rank_1("session",staged,{payload.toStdString()}))
    {
      if (error.empty())
      {
        error = "Cannot stage Postprocessor project data: "+file.last_error();
      }
      return false;
    }
    file.file->flush(H5F_SCOPE_GLOBAL);
    if (file.nameExists(previous) && !file.deleteGroup(previous))
    {
      error = "Cannot remove old Postprocessor staging.";
      return false;
    }
    const bool hadPrevious = file.nameExists(root);
    if (hadPrevious && !file.renameGroup(root,previous))
    {
      error = "Cannot preserve existing Postprocessor definitions.";
      return false;
    }
    if (!file.renameGroup(staged,root))
    {
      if (hadPrevious)
      {
        file.renameGroup(previous,root);
      }
      error = "Cannot publish staged Postprocessor definitions.";
      return false;
    }
    file.file->flush(H5F_SCOPE_GLOBAL);
    if (hadPrevious && !file.deleteGroup(previous))
    {
      error = "Postprocessor definitions saved, but old staging could not be removed.";
      return false;
    }
    if (!file.close())
    {
      error = "Cannot close saved Postprocessor definitions.";
      return false;
    }
    error.clear();
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
