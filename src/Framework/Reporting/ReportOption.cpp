#include "ReportOption.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportOption::ReportOption() = default;

QJsonObject ReportOption::to_json() const
{
  QJsonObject result;
  result.insert("key", QString::fromStdString(key));
  result.insert("label", QString::fromStdString(label));
  result.insert("kind", QString::fromStdString(kind));
  result.insert("default_value", QString::fromStdString(default_value));
  result.insert("choices", ReportJson::strings(choices));
  result.insert("minimum", minimum);
  result.insert("maximum", maximum);
  return result;
}

bool ReportOption::from_json(const QJsonObject& input, ReportOption& output, std::string& error)
{
  ReportOption candidate;
  if (!ReportJson::strings(input.value("choices"), candidate.choices, error) ||
      !ReportJson::integer(input, "minimum", candidate.minimum, error) ||
      !ReportJson::integer(input, "maximum", candidate.maximum, error))
  {
    return false;
  }
  if (!(ReportJson::text(input, "key", candidate.key, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "label", candidate.label, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "kind", candidate.kind, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "default_value", candidate.default_value, error)))
  {
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
