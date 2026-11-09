#pragma once
#include <QJsonObject>
#include <map>
#include <string>
#include <vector>

class ReportOption
{
public:
  ReportOption();
  std::string key;
  std::string label;
  std::string kind;
  std::string default_value;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportOption& output, std::string& error);
};
