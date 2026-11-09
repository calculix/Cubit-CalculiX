#include "ReportResultRequest.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportResultRequest::ReportResultRequest() = default;

QJsonObject ReportResultRequest::to_json() const
{
  QJsonObject result;
  result.insert("provider", QString::fromStdString(provider));
  result.insert("source", source.to_json());
  result.insert("settings", settings.to_json());
  return result;
}

bool ReportResultRequest::from_json(const QJsonObject& input, ReportResultRequest& output, std::string& error)
{
  ReportResultRequest candidate;
  if (!ReportJson::text(input, "provider", candidate.provider, error))
  {
    return false;
  }
  if (!(input.value("source").isObject() && ReportSource::from_json(input.value("source").toObject(), candidate.source, error)))
  {
    return false;
  }
  if (!(input.value("settings").isObject() && ReportConfiguration::from_json(input.value("settings").toObject(), candidate.settings, error)))
  {
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
