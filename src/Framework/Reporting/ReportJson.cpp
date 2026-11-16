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


QJsonValue ReportJson::quantity(const std::optional<ReportQuantity>& value)
{
  if (!value)
  {
    return QJsonValue(QJsonValue::Null);
  }
  QJsonArray dimension;
  for (double exponent : value->dimension)
  {
    dimension.append(exponent);
  }
  return QJsonObject{{"quantity", QString::fromStdString(value->quantity)},
    {"unit", QString::fromStdString(value->unit)}, {"dimension", dimension}, {"role", static_cast<int>(value->role)}};
}

bool ReportJson::validate_quantity(const ReportQuantity& value, std::string& error)
{
  if (value.quantity.empty() || value.unit.empty() || value.unit.size() > 4096 ||
      static_cast<int>(value.role) < 0 || static_cast<int>(value.role) > static_cast<int>(SimUnits::UnitRole::Angle))
  {
    error = "Invalid report quantity metadata.";
    return false;
  }
  for (double exponent : value.dimension)
  {
    if (!std::isfinite(exponent))
    {
      error = "Report quantity dimensions must be finite.";
      return false;
    }
  }
  return true;
}

bool ReportJson::quantity(const QJsonValue& input, std::optional<ReportQuantity>& output, std::string& error)
{
  if (input.isNull())
  {
    output.reset();
    return true;
  }
  if (!input.isObject())
  {
    error = "Expected report quantity metadata.";
    return false;
  }
  const auto object = input.toObject();
  ReportQuantity candidate;
  int role = 0;
  if (!text(object, "quantity", candidate.quantity, error) || !text(object, "unit", candidate.unit, error) ||
      !integer(object, "role", role, error) || role < 0 || role > static_cast<int>(SimUnits::UnitRole::Angle) ||
      !object.value("dimension").isArray() || object.value("dimension").toArray().size() != 7)
  {
    error = "Invalid report quantity metadata.";
    return false;
  }
  candidate.role = static_cast<SimUnits::UnitRole>(role);
  const auto dimensions = object.value("dimension").toArray();
  for (int i = 0; i < dimensions.size(); ++i)
  {
    if (!dimensions[i].isDouble() || !std::isfinite(dimensions[i].toDouble()))
    {
      error = "Report quantity dimensions must be finite numbers.";
      return false;
    }
    candidate.dimension[static_cast<std::size_t>(i)] = dimensions[i].toDouble();
  }
  if (!validate_quantity(candidate, error))
  {
    return false;
  }
  output = std::move(candidate);
  return true;
}
