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
  QJsonArray columnsJson, cellsJson;
  for (const auto& quantity : column_quantities)
  {
    columnsJson.append(ReportJson::quantity(quantity));
  }
  for (const auto& row : cell_quantities)
  {
    QJsonArray cells;
    for (const auto& quantity : row)
    {
      cells.append(ReportJson::quantity(quantity));
    }
    cellsJson.append(cells);
  }
  return {{"column_quantities", columnsJson}, {"cell_quantities", cellsJson}, {"title", QString::fromStdString(title)}, {"columns", ReportJson::strings(columns)},
    {"units", ReportJson::strings(units)}, {"rows", data}};
}

bool ReportTable::validate(std::string& error) const
{
  if (columns.empty() || columns.size() > 32 || (!units.empty() && units.size() != columns.size()) || rows.size() > 100000)
  {
    error = "Report table has invalid dimensions: " + title;
    return false;
  }
  if ((!column_quantities.empty() && column_quantities.size() != columns.size()) ||
      (!cell_quantities.empty() && cell_quantities.size() != rows.size()))
  {
    error = "Report quantity metadata does not match its table: " + title;
    return false;
  }
  for (const auto& quantity : column_quantities)
  {
    if (quantity && !ReportJson::validate_quantity(*quantity, error))
    {
      return false;
    }
  }
  for (std::size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
  {
    const auto& row = rows[rowIndex];
    if (row.size() != columns.size())
    {
      error = "Report table row width differs from its column count: " + title;
      return false;
    }
    if (!cell_quantities.empty())
    {
      const auto& quantities = cell_quantities[rowIndex];
      if (quantities.size() != columns.size())
      {
        error = "Report cell quantity count differs from its row width: " + title;
        return false;
      }
      for (const auto& quantity : quantities)
      {
        if (quantity && !ReportJson::validate_quantity(*quantity, error))
        {
          return false;
        }
      }
    }
    for (std::size_t column = 0; column < row.size(); ++column)
    {
      const ReportQuantity* quantity = nullptr;
      if (!column_quantities.empty() && column_quantities[column])
      {
        quantity = &*column_quantities[column];
      }
      if (!cell_quantities.empty() && cell_quantities[rowIndex][column])
      {
        quantity = &*cell_quantities[rowIndex][column];
      }
      const bool absent = row[column].isString() && row[column].toString().isEmpty();
      if (quantity && ((!row[column].isDouble() && !absent) ||
          (!units.empty() && !units[column].empty() && units[column] != quantity->unit)))
      {
        error = "Report physical cell or unit label conflicts with its quantity: " + title;
        return false;
      }
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
  if (!input.value("column_quantities").isArray() || !input.value("cell_quantities").isArray())
  {
    error = "Missing report table quantity metadata.";
    return false;
  }
  for (const auto value : input.value("column_quantities").toArray())
  {
    std::optional<ReportQuantity> quantity;
    if (!ReportJson::quantity(value, quantity, error))
    {
      return false;
    }
    candidate.column_quantities.push_back(std::move(quantity));
  }
  for (const auto value : input.value("cell_quantities").toArray())
  {
    if (!value.isArray())
    {
      error = "Invalid report cell quantities.";
      return false;
    }
    std::vector<std::optional<ReportQuantity>> quantities;
    for (const auto cell : value.toArray())
    {
      std::optional<ReportQuantity> quantity;
      if (!ReportJson::quantity(cell, quantity, error))
      {
        return false;
      }
      quantities.push_back(std::move(quantity));
    }
    candidate.cell_quantities.push_back(std::move(quantities));
  }
  if (!candidate.validate(error))
  {
    return false;
  }
  output = std::move(candidate);
  return true;
}
