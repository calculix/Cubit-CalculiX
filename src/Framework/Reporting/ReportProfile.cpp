#include "ReportProfile.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportProfile::ReportProfile() = default;

QJsonObject ReportProfile::to_json() const
{
  QJsonObject result;
  result.insert("id", QString::fromStdString(id));
  result.insert("name", QString::fromStdString(name));
  result.insert("workspace_id", QString::fromStdString(workspace_id));
  result.insert("revision", revision);
  result.insert("template_id", QString::fromStdString(template_id));
  result.insert("template_revision", template_revision);
  result.insert("configuration", configuration.to_json());
  return result;
}

bool ReportProfile::from_json(const QJsonObject& input, ReportProfile& output, std::string& error)
{
  ReportProfile candidate;
  if (!(ReportJson::text(input, "id", candidate.id, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "name", candidate.name, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "workspace_id", candidate.workspace_id, error)))
  {
    return false;
  }
  if (!(ReportJson::integer(input, "revision", candidate.revision, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "template_id", candidate.template_id, error)))
  {
    return false;
  }
  if (!(ReportJson::integer(input, "template_revision", candidate.template_revision, error)))
  {
    return false;
  }
  if (!(input.value("configuration").isObject() && ReportConfiguration::from_json(input.value("configuration").toObject(), candidate.configuration, error)))
  {
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
