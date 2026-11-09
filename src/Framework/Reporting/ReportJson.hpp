#pragma once
#include <QJsonObject>
#include <QJsonArray>
#include <map>
#include <string>
#include <vector>

namespace ReportJson
{
  QJsonObject strings(const std::map<std::string, std::string>& values);
  bool strings(const QJsonValue& input, std::map<std::string, std::string>& output, std::string& error);
  QJsonArray strings(const std::vector<std::string>& values);
  bool strings(const QJsonValue& input, std::vector<std::string>& output, std::string& error);
  bool text(const QJsonObject& input, const char* key, std::string& output, std::string& error);
  bool integer(const QJsonObject& input, const char* key, int& output, std::string& error);
}
