#pragma once
#include <QJsonObject>
#include <QJsonValue>
#include <string>
#include <vector>
#include <optional>
#include "ReportQuantity.hpp"

class ReportTable
{
public:
  ReportTable();
  std::string title;
  std::vector<std::string> columns;
  std::vector<std::string> units;
  // Cell metadata overrides column metadata for mixed property tables.
  // Absence makes no physical assertion (IDs, text, or provider-validated data).
  std::vector<std::optional<ReportQuantity>> column_quantities;
  std::vector<std::vector<std::optional<ReportQuantity>>> cell_quantities;
  // Numbers remain numeric; text is escaped only by the document renderer.
  std::vector<std::vector<QJsonValue>> rows;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportTable& output, std::string& error);
  bool validate(std::string& error) const;
};
