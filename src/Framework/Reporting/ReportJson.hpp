#pragma once
#include <QJsonObject>
#include <QJsonArray>
#include <map>
#include <string>
#include <vector>
#include <optional>
#include "ReportQuantity.hpp"

namespace ReportJson
{
  QJsonValue quantity(const std::optional<ReportQuantity>& value);
  bool quantity(const QJsonValue& input, std::optional<ReportQuantity>& output, std::string& error);
  bool validate_quantity(const ReportQuantity& value, std::string& error);
  QJsonObject strings(const std::map<std::string, std::string>& values);
  bool strings(const QJsonValue& input, std::map<std::string, std::string>& output, std::string& error);
  QJsonArray strings(const std::vector<std::string>& values);
  bool strings(const QJsonValue& input, std::vector<std::string>& output, std::string& error);
  bool text(const QJsonObject& input, const char* key, std::string& output, std::string& error);
  bool integer(const QJsonObject& input, const char* key, int& output, std::string& error);
}
