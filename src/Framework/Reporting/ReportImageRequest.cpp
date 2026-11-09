#include "ReportImageRequest.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportImageRequest::ReportImageRequest() = default;

QJsonObject ReportImageRequest::to_json() const
{
  QJsonObject result;
  result.insert("provider", QString::fromStdString(provider));
  result.insert("source", source.to_json());
  result.insert("settings", settings.to_json());
  result.insert("width", width);
  result.insert("height", height);
  return result;
}

bool ReportImageRequest::from_json(const QJsonObject& input, ReportImageRequest& output, std::string& error)
{
  ReportImageRequest candidate;
  if (!(ReportJson::text(input, "provider", candidate.provider, error)))
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
  if (!(ReportJson::integer(input, "width", candidate.width, error)))
  {
    return false;
  }
  if (!(ReportJson::integer(input, "height", candidate.height, error)))
  {
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
