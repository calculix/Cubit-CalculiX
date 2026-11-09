#include "ReportProfileStore.hpp"
#include <utility>
#include "Hdf5FileTransaction.hpp"
#include "HDF5Tool.hpp"
#include "Framework/Reporting/ReportProfile.hpp"
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLockFile>
#include <QUuid>
#include <algorithm>
#include <cctype>
#include <exception>
#include <limits>

namespace
{
const std::string root = "framework/reporting";

bool validIdentity(const std::string& value)
{
  return !value.empty() && value.size() <= 128 && std::all_of(value.begin(), value.end(), [](unsigned char c)
  {
    return std::isalnum(c) || c == '_' || c == '-';
  });
}

bool readProfiles(HDF5Tool& file, const std::string& workspace, std::vector<ReportProfile>& output, std::string& error)
{
  if (!file.nameExists(root))
  {
    output.clear();
    return true;
  }
  std::vector<int> schema;
  if (!file.read_dataset_int_rank_1("schema_version", root, schema) || schema != std::vector<int>{1})
  {
    error = "Unsupported report-profile schema.";
    return false;
  }
  const std::string group = root + "/workspaces/" + workspace;
  std::vector<ReportProfile> candidate;
  if (!file.nameExists(group + "/profile_ids"))
  {
    output.clear();
    return true;
  }
  std::vector<std::string> ids;
  if (!file.read_dataset_string_rank_1("profile_ids", group, ids))
  {
    error = "Cannot read report-profile index.";
    return false;
  }
  for (const auto& id : ids)
  {
    std::vector<std::string> payload;
    if (!validIdentity(id) || !file.read_dataset_string_rank_1("payload", group + "/profiles/" + id, payload) || payload.size() != 1 || payload.front().size() > 2 * 1024 * 1024)
    {
      error = "Invalid report-profile payload.";
      return false;
    }
    QJsonParseError parse;
    const auto json = QJsonDocument::fromJson(QByteArray::fromStdString(payload.front()), &parse);
    ReportProfile profile;
    if (parse.error != QJsonParseError::NoError || !json.isObject() ||
        !ReportProfile::from_json(json.object(), profile, error) || profile.id != id ||
        profile.workspace_id != workspace || profile.name.empty() || profile.revision < 1 || profile.template_revision < 1)
    {
      error = "Invalid stored report profile: " + id;
      return false;
    }
    for (const auto& existing : candidate)
    {
      if (existing.id == profile.id || existing.name == profile.name)
      {
        error = "Duplicate report profile identity or name.";
        return false;
      }
    }
    candidate.push_back(std::move(profile));
  }
  output = std::move(candidate);
  return true;
}

bool writeStrings(HDF5Tool& file, const std::string& group, const std::string& key,
  const std::vector<std::string>& values, std::string& error)
{
  if (!file.ensureGroup(group) || (file.nameExists(group + "/" + key) && !file.deleteDataset(key, group)) ||
      !file.write_dataset_string_rank_1(key, group, values))
  {
    error = "Cannot write report profile: " + file.last_error();
    return false;
  }
  return true;
}
}

ReportProfileStore::ReportProfileStore(std::string path) : filename(std::move(path))
{
}

ReportProfileStore::~ReportProfileStore() = default;

bool ReportProfileStore::list(const std::string& workspace, std::vector<ReportProfile>& output, std::string& error) const
{
  error.clear();
  if (!validIdentity(workspace))
  {
    error = "Invalid report workspace identity.";
    return false;
  }
  try
  {
    QLockFile lock(QString::fromStdString(filename) + ".lock");
    if (!lock.tryLock(0))
    {
      error = "The Framework options file is in use.";
      return false;
    }
    if (!QFileInfo::exists(QString::fromStdString(filename)))
    {
      output.clear();
      return true;
    }
    HDF5Tool file(filename, false);
    std::vector<ReportProfile> candidate;
    if (!readProfiles(file, workspace, candidate, error))
    {
      return false;
    }
    if (!file.close())
    {
      error = "Cannot close Framework profile storage.";
      return false;
    }
    output = std::move(candidate);
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

bool ReportProfileStore::load(const std::string& workspace, const std::string& name, ReportProfile& output, std::string& error) const
{
  std::vector<ReportProfile> profiles;
  if (!list(workspace, profiles, error))
  {
    return false;
  }
  for (const auto& profile : profiles)
  {
    if (profile.name == name)
    {
      output = profile;
      return true;
    }
  }
  error = "Unknown report profile: " + name;
  return false;
}

bool ReportProfileStore::save(ReportProfile& profile, std::string& error)
{
  if (!validIdentity(profile.workspace_id) || profile.name.empty() || profile.name.size() > 256 ||
      profile.template_id.empty() || profile.template_revision < 1)
  {
    error = "Report profile has invalid identity or template metadata.";
    return false;
  }
  ReportProfile candidate = profile;
  const bool saved = Hdf5FileTransaction().update(filename, [&](HDF5Tool& file, bool existed, std::string& error)
  {
    // Framework initializes the file first. Do not create an options file lacking
    // the existing root schema, or overwrite a file with an unsupported schema.
    std::vector<int> optionsSchema;
    if (!existed || !file.read_dataset_int_rank_1("schema_version", "/", optionsSchema) || optionsSchema != std::vector<int>{1})
    {
      error = "Framework options must be initialized with a supported schema before saving profiles.";
      return false;
    }
    std::vector<ReportProfile> profiles;
    if (!readProfiles(file, candidate.workspace_id, profiles, error))
    {
      return false;
    }
    auto match = std::find_if(profiles.begin(), profiles.end(), [&](const ReportProfile& item)
    {
      return item.name == candidate.name;
    });
    if (match != profiles.end())
    {
      if (match->revision == std::numeric_limits<int>::max())
      {
        error = "Report profile revision limit reached.";
        return false;
      }
      candidate.id = match->id;
      candidate.revision = match->revision + 1;
      *match = candidate;
    }
    else
    {
      candidate.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
      candidate.revision = 1;
      profiles.push_back(candidate);
    }
    if (!file.nameExists(root))
    {
      if (!file.ensureGroup(root) || !file.write_dataset_int_rank_1("schema_version", root, {1}))
      {
        error = file.last_error();
        return false;
      }
    }
    const std::string group = root + "/workspaces/" + candidate.workspace_id;
    std::vector<std::string> ids;
    for (const auto& item : profiles)
    {
      ids.push_back(item.id);
    }
    const auto payload = QJsonDocument(candidate.to_json()).toJson(QJsonDocument::Compact).toStdString();
    if (payload.size() > 2 * 1024 * 1024)
    {
      error = "Report profile exceeds the 2 MiB storage limit.";
      return false;
    }
    return writeStrings(file, group + "/profiles/" + candidate.id, "payload", {payload}, error) &&
      writeStrings(file, group, "profile_ids", ids, error);
  }, error);
  if (saved)
  {
    profile = std::move(candidate);
  }
  return saved;
}

bool ReportProfileStore::default_profile(const std::string& workspace, ReportProfile& output, bool& found, std::string& error) const
{
  if (!validIdentity(workspace))
  {
    error = "Invalid report workspace identity.";
    return false;
  }
  error.clear();
  try
  {
    QLockFile lock(QString::fromStdString(filename) + ".lock");
    if (!lock.tryLock(0))
    {
      error = "The Framework options file is in use.";
      return false;
    }
    if (!QFileInfo::exists(QString::fromStdString(filename)))
    {
      found = false;
      return true;
    }
    HDF5Tool file(filename, false);
    std::vector<ReportProfile> profiles;
    if (!readProfiles(file, workspace, profiles, error))
    {
      return false;
    }
    const std::string group = root + "/workspaces/" + workspace;
    if (!file.nameExists(group + "/default_profile_id"))
    {
      if (!file.close())
      {
        error = "Cannot close profile storage.";
        return false;
      }
      found = false;
      return true;
    }
    std::vector<std::string> ids;
    if (!file.read_dataset_string_rank_1("default_profile_id", group, ids) || ids.size() != 1)
    {
      error = "Invalid default report profile reference.";
      return false;
    }
    for (const auto& item : profiles)
    {
      if (item.id == ids.front())
      {
        if (!file.close())
        {
          error = "Cannot close profile storage.";
          return false;
        }
        output = item;
        found = true;
        return true;
      }
    }
    error = "Default report profile does not exist.";
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

bool ReportProfileStore::set_default_profile(const std::string& workspace, const std::string& name, std::string& error)
{
  if (!validIdentity(workspace))
  {
    error = "Invalid report workspace identity.";
    return false;
  }
  return Hdf5FileTransaction().update(filename, [&](HDF5Tool& file, bool, std::string& error)
  {
    std::vector<ReportProfile> profiles;
    if (!readProfiles(file, workspace, profiles, error))
    {
      return false;
    }
    for (const auto& profile : profiles)
    {
      if (profile.name == name)
      {
        return writeStrings(file, root + "/workspaces/" + workspace, "default_profile_id", {profile.id}, error);
      }
    }
    error = "Unknown report profile: " + name;
    return false;
  }, error);
}
