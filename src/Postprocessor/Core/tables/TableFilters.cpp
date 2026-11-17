#include "TableFilterCodec.hpp"
#include "ResultTables.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <QJsonObject>
#include <algorithm>
#include <cmath>
#include <locale>
#include <set>
#include <sstream>
#include <utility>
namespace Postprocessor
{
  bool same_table_filters(const std::vector<TableFilter>& a,const std::vector<TableFilter>& b)
  {
    if (a.size() != b.size())
    {
      return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i)
    {
      if (a[i].column != b[i].column || a[i].value != b[i].value || a[i].mode != b[i].mode ||
          a[i].minimum != b[i].minimum || a[i].maximum != b[i].maximum || a[i].unit != b[i].unit)
      {
        return false;
      }
    }
    return true;
  }
  bool table_filter_range_supported(const ResultTableData& data,std::size_t column)
  {
    if (column >= data.columns.size() || column >= data.numeric.size() || !data.numeric[column])
    {
      return false;
    }
    const auto& name = data.columns[column];
    const auto ends = [&](const std::string& suffix)
    {
      return name.size() >= suffix.size() && name.compare(name.size()-suffix.size(),suffix.size(),suffix) == 0;
    };
    // Identifier predicates must never round through double, including 64-bit IDs.
    return name != "Identifier" && name != "IP" && name != "Dataset" && !ends("Id") && !ends("ID");
  }
  bool validate_table_filters(const ResultTableData& data,std::vector<TableFilter>& filters,std::string& error)
  {
    error.clear();
    std::set<std::string> columns;
    if (filters.size() > 64)
    {
      error = "At most 64 column filters are supported.";
      return false;
    }
    for (auto& filter : filters)
    {
      const auto found = std::find(data.columns.begin(),data.columns.end(),filter.column);
      if (filter.column.empty() || filter.column.size() > 4096 || filter.value.size() > 65536 ||
          filter.value.find('\0') != std::string::npos || !columns.insert(filter.column).second ||
          std::count(data.columns.begin(),data.columns.end(),filter.column) != 1 ||
          (filter.mode != TableFilterMode::Equals && filter.mode != TableFilterMode::Range))
      {
        error = "Filter columns must exist, be unique and have bounded text values.";
        return false;
      }
      const auto column = static_cast<std::size_t>(found-data.columns.begin());
      if (filter.mode == TableFilterMode::Range && (!table_filter_range_supported(data,column) ||
          !std::isfinite(filter.minimum) || !std::isfinite(filter.maximum) || filter.minimum > filter.maximum))
      {
        error = "Range filters require a numeric non-identifier column and finite increasing bounds. Use exact text for IDs.";
        return false;
      }
      if (column < data.column_units.size() && !data.column_units[column].unit.empty())
      {
        const auto& quantity = data.column_units[column];
        if (filter.unit.empty())
        {
          filter.unit = result_table_column_unit(data,column);
        }
        if (quantity.quantity.empty() && filter.unit != quantity.unit)
        {
          error = "This column has no quantity metadata for converting a filter unit.";
          return false;
        }
        double ignored = 0;
        if (!framework_units().to_si(0,filter.unit,quantity.dimension,quantity.role,ignored,error))
        {
          return false;
        }
      }
      else if (!filter.unit.empty())
      {
        error = "A unit cannot be applied to an identifier or an unknown-unit column.";
        return false;
      }
    }
    return true;
  }
  std::vector<PreparedTableFilter> prepare_table_filters(const ResultTableData& data,const std::vector<TableFilter>& filters)
  {
    std::vector<PreparedTableFilter> prepared;
    auto checked = filters;
    std::string error;
    if (!validate_table_filters(data,checked,error))
    {
      // A changed/unavailable schema must not silently drop a saved predicate.
      prepared.emplace_back();
      return prepared;
    }
    for (const auto& filter : checked)
    {
      PreparedTableFilter item;
      item.filter = filter;
      const auto found = std::find(data.columns.begin(),data.columns.end(),filter.column);
      if (found != data.columns.end() && std::count(data.columns.begin(),data.columns.end(),filter.column) == 1)
      {
        item.column = static_cast<std::size_t>(found-data.columns.begin());
        if (!filter.unit.empty() && item.column < data.column_units.size())
        {
          item.presentation = data.presentation_units;
          if (!item.presentation)
          {
            item.presentation = SimUnits::WorkspaceUnitContext("postprocessor");
          }
          item.presentation->display_units[data.column_units[item.column].quantity] = filter.unit;
        }
      }
      prepared.push_back(std::move(item));
    }
    return prepared;
  }
  bool prepared_table_row_matches(const ResultTableData& data,std::size_t row,const std::vector<PreparedTableFilter>& filters)
  {
    if (row >= data.rows)
    {
      return false;
    }
    for (const auto& item : filters)
    {
      if (item.column >= data.columns.size())
      {
        return false;
      }
      const SimUnits::WorkspaceUnitContext* presentation = nullptr;
      if (item.presentation)
      {
        presentation = &*item.presentation;
      }
      const auto cell = result_table_cell(data,row,item.column,presentation);
      const auto& filter = item.filter;
      if (filter.mode == TableFilterMode::Equals)
      {
        if (cell != filter.value)
        {
          return false;
        }
      }
      else
      {
        double number = 0;
        std::istringstream input(cell);
        input.imbue(std::locale::classic());
        input >> number;
        if (!input || !(input >> std::ws).eof() || !std::isfinite(number) || number < filter.minimum || number > filter.maximum)
        {
          return false;
        }
      }
    }
    return true;
  }
  bool table_row_matches(const ResultTableData& data,std::size_t row,const std::vector<TableFilter>& filters)
  {
    return prepared_table_row_matches(data,row,prepare_table_filters(data,filters));
  }
  QJsonArray table_filters_to_json(const std::vector<TableFilter>& filters)
  {
    QJsonArray result;
    for (const auto& filter : filters)
    {
      result.append(QJsonObject{{"column",QString::fromStdString(filter.column)},
        {"value",QString::fromStdString(filter.value)},{"mode",static_cast<int>(filter.mode)},
        {"minimum",filter.minimum},{"maximum",filter.maximum},{"unit",QString::fromStdString(filter.unit)}});
    }
    return result;
  }
  bool table_filters_from_json(const QJsonValue& json,std::vector<TableFilter>& filters)
  {
    filters.clear();
    if (json.isUndefined())
    {
      return true;
    }
    if (!json.isArray() || json.toArray().size() > 64)
    {
      return false;
    }
    std::set<std::string> names;
    for (const auto& item : json.toArray())
    {
      const auto object = item.toObject();
      const auto mode = object.value("mode");
      if (!item.isObject() || !object.value("column").isString() || !object.value("value").isString() ||
          !object.value("unit").isString() || !mode.isDouble() || (mode.toDouble() != 0 && mode.toDouble() != 1) ||
          !object.value("minimum").isDouble() || !object.value("maximum").isDouble())
      {
        return false;
      }
      TableFilter filter;
      filter.column = object.value("column").toString().toStdString();
      filter.value = object.value("value").toString().toStdString();
      filter.unit = object.value("unit").toString().toStdString();
      filter.mode = static_cast<TableFilterMode>(mode.toInt());
      filter.minimum = object.value("minimum").toDouble();
      filter.maximum = object.value("maximum").toDouble();
      if (filter.column.empty() || filter.column.size() > 4096 || filter.value.size() > 65536 || filter.unit.size() > 256 ||
          filter.column.find('\0') != std::string::npos || filter.value.find('\0') != std::string::npos ||
          filter.unit.find('\0') != std::string::npos || !names.insert(filter.column).second ||
          !std::isfinite(filter.minimum) || !std::isfinite(filter.maximum) || filter.minimum > filter.maximum)
      {
        return false;
      }
      filters.push_back(filter);
    }
    return true;
  }
}
