#include "ReportProjectPersistence.hpp"
#include "ReportSession.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include "Framework/Storage/ModelFileMetadata.hpp"
#include "Framework/Core/CubitSimVersion.hpp"
#include <QJsonDocument>
#include <QJsonParseError>
#include <exception>

namespace
{
const std::string root = "cubit-sim/report";
}

ReportProjectPersistence::ReportProjectPersistence() = default;
ReportProjectPersistence::~ReportProjectPersistence() = default;

bool ReportProjectPersistence::read(HDF5Tool& file, ReportSession& output, std::string& error) const
{
  error.clear();
  try
  {
    if (!file.nameExists(root))
    {
      output = ReportSession();
      return true;
    }
    std::vector<int> schema;
    std::vector<std::string> payload;
    std::string version;
    if (!CubitSimModelFile::read(file, version, error) ||
        !file.read_dataset_int_rank_1("schema_version", root, schema) || schema != std::vector<int>{2} ||
        !file.read_dataset_string_rank_1("session", root, payload) || payload.size() != 1 || payload.front().size() > 64 * 1024 * 1024)
    {
      error = "Unsupported or invalid report project data.";
      return false;
    }
    QJsonParseError parse;
    const auto json = QJsonDocument::fromJson(QByteArray::fromStdString(payload.front()), &parse);
    if (parse.error != QJsonParseError::NoError || !json.isObject())
    {
      error = "Malformed saved report session.";
      return false;
    }
    return ReportSession::from_json(json.object(), output, error);
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

bool ReportProjectPersistence::write(HDF5Tool& file, const ReportSession& session, std::string& error) const
{
  error.clear();
  try
  {
    for (const auto& item : session.documents)
    {
      if (!item.second->validate(error))
      {
        return false;
      }
    }
    const auto data = QJsonDocument(session.to_json()).toJson(QJsonDocument::Compact);
    if (data.size() > 64 * 1024 * 1024)
    {
      error = "Saved report session exceeds 64 MiB.";
      return false;
    }
    const std::string staged = "cubit-sim/report-staged";
    const std::string previous = "cubit-sim/report-previous";
    if (!CubitSimModelFile::write(file, CubitSim::Version, error) ||
        (file.nameExists(staged) && !file.deleteGroup(staged)) ||
        !file.ensureGroup(staged) || !file.write_dataset_int_rank_1("schema_version", staged, {2}) ||
        !file.write_dataset_string_rank_1("session", staged, {data.toStdString()}))
    {
      if (error.empty())
      {
        error = "Cannot stage report project data: " + file.last_error();
      }
      return false;
    }
    if (file.nameExists(previous) && !file.deleteGroup(previous))
    {
      error = "Cannot remove previous report persistence staging.";
      return false;
    }
    const bool hadPrevious = file.nameExists(root);
    if (hadPrevious && !file.renameGroup(root, previous))
    {
      error = "Cannot preserve the previous report data.";
      return false;
    }
    if (!file.renameGroup(staged, root))
    {
      if (hadPrevious)
      {
        file.renameGroup(previous, root);
      }
      error = "Cannot publish staged report project data.";
      return false;
    }
    if (hadPrevious && !file.deleteGroup(previous))
    {
      error = "Report data was written, but previous staging could not be removed.";
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
