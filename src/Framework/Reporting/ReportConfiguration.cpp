#include "ReportConfiguration.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportConfiguration::ReportConfiguration() = default;

QJsonObject ReportConfiguration::to_json() const
{
  QJsonObject result;
  result.insert("values", ReportJson::strings(values));
  result.insert("company_logo", [&]() -> QJsonValue {
    if (company_logo) {
      return QJsonValue(company_logo->to_json());
    } else {
      return QJsonValue(QJsonValue::Null);
    }
  }());
  if (units)
  {
    result.insert("units", units->to_json());
  }
  else
  {
    result.insert("units", QJsonValue(QJsonValue::Null));
  }
  return result;
}

bool ReportConfiguration::from_json(const QJsonObject& input, ReportConfiguration& output, std::string& error)
{
  ReportConfiguration candidate;
  if (!(ReportJson::strings(input.value("values"), candidate.values, error)))
  {
    return false;
  }
  candidate.replace_company_logo = input.contains("company_logo");
  if (candidate.replace_company_logo && !input.value("company_logo").isNull())
  {
    ReportImage image;
    if (!input.value("company_logo").isObject() || !ReportImage::from_json(input.value("company_logo").toObject(), image, error))
    {
      error = "Invalid embedded company logo.";
      return false;
    }
    candidate.company_logo = std::move(image);
  }
  if (!input.value("units").isNull())
  {
    ReportUnitSettings settings;
    if (!input.value("units").isObject() || !ReportUnitSettings::from_json(input.value("units").toObject(), settings, error))
    {
      error = "Invalid report presentation units: " + error;
      return false;
    }
    candidate.units = std::move(settings);
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
