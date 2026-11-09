#include "ReportTable.hpp"
#include <utility>
#include "ReportJson.hpp"
#include <cmath>

ReportTable::ReportTable() = default;

QJsonObject ReportTable::to_json() const
{
  QJsonArray data;
  for (const auto& row : rows)
  {
    QJsonArray cells;
    for (const auto& cell : row)
    {
      cells.append(cell);
    }
    data.append(cells);
  }
  return {{"title", QString::fromStdString(title)}, {"columns", ReportJson::strings(columns)},
    {"units", ReportJson::strings(units)}, {"rows", data}};
}

bool ReportTable::validate(std::string& error) const
{
  if (columns.empty() || columns.size() > 32 || (!units.empty() && units.size() != columns.size()) || rows.size() > 100000)
  {
    error = "Report table has invalid dimensions: " + title;
    return false;
  }
  for (const auto& row : rows)
  {
    if (row.size() != columns.size())
    {
      error = "Report table row width differs from its column count: " + title;
      return false;
    }
    for (const auto& cell : row)
    {
      if ((!cell.isString() && !cell.isDouble()) || (cell.isDouble() && !std::isfinite(cell.toDouble())))
      {
        error = "Report tables support finite numeric and text cells only.";
        return false;
      }
    }
  }
  return true;
}

bool ReportTable::from_json(const QJsonObject& input, ReportTable& output, std::string& error)
{
  ReportTable candidate;
  if (!ReportJson::text(input, "title", candidate.title, error) ||
      !ReportJson::strings(input.value("columns"), candidate.columns, error) ||
      !ReportJson::strings(input.value("units"), candidate.units, error) || !input.value("rows").isArray())
  {
    return false;
  }
  for (const auto value : input.value("rows").toArray())
  {
    if (!value.isArray())
    {
      error = "Invalid report table row.";
      return false;
    }
    std::vector<QJsonValue> row;
    for (const auto cell : value.toArray())
    {
      row.push_back(cell);
    }
    candidate.rows.push_back(std::move(row));
  }
  if (!candidate.validate(error))
  {
    return false;
  }
  output = std::move(candidate);
  return true;
}
