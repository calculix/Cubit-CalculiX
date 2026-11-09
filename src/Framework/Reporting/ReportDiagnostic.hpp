#pragma once
#include <QJsonObject>
#include <map>
#include <string>
#include <vector>

class ReportDiagnostic
{
public:
  ReportDiagnostic();
  std::string section_id;
  std::string message;
  std::string severity;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportDiagnostic& output, std::string& error);
};
