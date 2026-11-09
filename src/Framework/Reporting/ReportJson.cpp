#include "ReportJson.hpp"
#include <utility>
#include <cmath>
#include <limits>

QJsonObject ReportJson::strings(const std::map<std::string, std::string>& values)
{
  QJsonObject result;
  for (const auto& entry : values)
  {
    result.insert(QString::fromStdString(entry.first), QString::fromStdString(entry.second));
  }
  return result;
}

bool ReportJson::strings(const QJsonValue& input, std::map<std::string, std::string>& output, std::string& error)
{
  if (!input.isObject())
  {
    error = "Expected a report settings object.";
    return false;
  }
  std::map<std::string, std::string> candidate;
  const auto object = input.toObject();
  for (auto it = object.begin(); it != object.end(); ++it)
  {
    if (!it.value().isString())
    {
      error = "Report settings must contain string values.";
      return false;
    }
    candidate.emplace(it.key().toStdString(), it.value().toString().toStdString());
  }
  output = std::move(candidate);
  return true;
}

QJsonArray ReportJson::strings(const std::vector<std::string>& values)
{
  QJsonArray result;
  for (const auto& value : values)
  {
    result.append(QString::fromStdString(value));
  }
  return result;
}

bool ReportJson::strings(const QJsonValue& input, std::vector<std::string>& output, std::string& error)
{
  if (!input.isArray())
  {
    error = "Expected a report string array.";
    return false;
  }
  std::vector<std::string> candidate;
  for (const auto value : input.toArray())
  {
    if (!value.isString())
    {
      error = "Invalid report string-array entry.";
      return false;
    }
    candidate.push_back(value.toString().toStdString());
  }
  output = std::move(candidate);
  return true;
}

bool ReportJson::text(const QJsonObject& input, const char* key, std::string& output, std::string& error)
{
  if (!input.value(key).isString())
  {
    error = std::string("Missing report text: ") + key;
    return false;
  }
  output = input.value(key).toString().toStdString();
  return true;
}

bool ReportJson::integer(const QJsonObject& input, const char* key, int& output, std::string& error)
{
  const auto value = input.value(key);
  const double number = value.toDouble();
  if (!value.isDouble() || !std::isfinite(number) || std::floor(number) != number ||
      number < std::numeric_limits<int>::min() || number > std::numeric_limits<int>::max())
  {
    error = std::string("Invalid report integer: ") + key;
    return false;
  }
  output = static_cast<int>(number);
  return true;
}
