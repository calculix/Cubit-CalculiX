#include "ReportImage.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportImage::ReportImage() = default;

QJsonObject ReportImage::to_json() const
{
  QJsonObject result;
  result.insert("id", QString::fromStdString(id));
  result.insert("caption", QString::fromStdString(caption));
  result.insert("media_type", QString::fromStdString(media_type));
  result.insert("base64_data", QString::fromStdString(base64_data));
  return result;
}

bool ReportImage::from_json(const QJsonObject& input, ReportImage& output, std::string& error)
{
  ReportImage candidate;
  if (!(ReportJson::text(input, "id", candidate.id, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "caption", candidate.caption, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "media_type", candidate.media_type, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "base64_data", candidate.base64_data, error)))
  {
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
