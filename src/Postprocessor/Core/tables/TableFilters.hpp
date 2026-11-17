#pragma once
#include <string>
#include <vector>
#include <optional>
#include <cstddef>
#include <limits>
#include "Framework/Core/units/WorkspaceUnitContext.hpp"
namespace Postprocessor
{
  struct ResultTableData;
  enum class TableFilterMode
  {
    Equals, Range
  };
  struct TableFilter
  {
    std::string column, value;
    TableFilterMode mode = TableFilterMode::Equals;
    double minimum = 0, maximum = 0;
    // Frozen presentation unit for physical columns; identifiers stay exact text.
    std::string unit;
  };
  struct PreparedTableFilter
  {
    TableFilter filter;
    std::size_t column = std::numeric_limits<std::size_t>::max();
    std::optional<SimUnits::WorkspaceUnitContext> presentation;
  };
  std::vector<PreparedTableFilter> prepare_table_filters(const ResultTableData& data,const std::vector<TableFilter>& filters);
  bool prepared_table_row_matches(const ResultTableData& data,std::size_t row,const std::vector<PreparedTableFilter>& filters);
  bool table_filter_range_supported(const ResultTableData& data,std::size_t column);
  bool same_table_filters(const std::vector<TableFilter>& a,const std::vector<TableFilter>& b);
  bool validate_table_filters(const ResultTableData& data,std::vector<TableFilter>& filters,std::string& error);
  // AND across columns, inclusive finite ranges, exact text for identifiers.
  // Row order and source storage are never modified.
  bool table_row_matches(const ResultTableData& data,std::size_t row,const std::vector<TableFilter>& filters);
}
