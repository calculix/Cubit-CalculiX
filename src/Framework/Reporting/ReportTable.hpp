#pragma once
#include <QJsonObject>
#include <QJsonValue>
#include <string>
#include <vector>

class ReportTable
{
public:
  ReportTable();
  std::string title;
  std::vector<std::string> columns;
  std::vector<std::string> units;
  // Numbers remain numeric; text is escaped only by the document renderer.
  std::vector<std::vector<QJsonValue>> rows;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportTable& output, std::string& error);
  bool validate(std::string& error) const;
};
