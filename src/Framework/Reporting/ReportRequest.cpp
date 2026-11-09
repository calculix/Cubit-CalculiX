#include "ReportRequest.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportRequest::ReportRequest() = default;

QJsonObject ReportRequest::to_json() const
{
  QJsonObject result;
  result.insert("kind", QString::fromStdString(kind));
  result.insert("model_id", QString::fromStdString(model_id));
  result.insert("provider", QString::fromStdString(provider));
  result.insert("job_id", job_id);
  result.insert("profile_revision", profile_revision);
  result.insert("profile_id", QString::fromStdString(profile_id));
  result.insert("overrides", overrides.to_json());
  return result;
}

bool ReportRequest::from_json(const QJsonObject& input, ReportRequest& output, std::string& error)
{
  ReportRequest candidate;
  if (!ReportJson::text(input, "kind", candidate.kind, error) ||
      !ReportJson::text(input, "model_id", candidate.model_id, error) ||
      (candidate.kind != "job" && candidate.kind != "model"))
  {
    error = "Invalid report source kind.";
    return false;
  }
  if (!(ReportJson::text(input, "provider", candidate.provider, error)))
  {
    return false;
  }
  if (!(ReportJson::integer(input, "job_id", candidate.job_id, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "profile_id", candidate.profile_id, error)))
  {
    return false;
  }
  if (!(input.value("overrides").isObject() && ReportConfiguration::from_json(input.value("overrides").toObject(), candidate.overrides, error)))
  {
    return false;
  }
  if (!ReportJson::integer(input, "profile_revision", candidate.profile_revision, error))
  {
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
