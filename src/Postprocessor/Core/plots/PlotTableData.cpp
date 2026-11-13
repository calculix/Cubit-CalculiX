#include "PlotData.hpp"
#include <utility>
#include "../tables/ResultTables.hpp"
#include <algorithm>
#include <cmath>
#include <locale>
#include <set>
#include <sstream>
namespace
{
  bool numeric(const std::string& text, double& value)
  {
    std::istringstream input(text);
    input.imbue(std::locale::classic());
    input >> value;
    return input && (input >> std::ws).eof() && std::isfinite(value);
  }
}
namespace Postprocessor
{
  bool plot_table_data(std::shared_ptr<const SimResults::VtkResultsData> data, const PlotTableReference& reference,
    PlotCurveData& output, std::string& error)
  {
    error.clear();
    output.samples.clear();
    output.status.clear();
    output.x_title.clear();
    output.y_title.clear();
    output.time_axis = output.frame_axis = false;
    const auto table = result_table_data(data,TableReference{reference.source_id,reference.key,false,0});
    if (!table.status.empty())
    {
      output.status = table.status;
      return true;
    }
    if (table.rows > PlotDefaults::maximum_samples)
    {
      error = "The table exceeds the per-curve sample limit; no rows were truncated.";
      return false;
    }
    const auto column = [&](const std::string& name) -> std::optional<std::size_t>
    {
      std::optional<std::size_t> found;
      for (std::size_t index = 0; index < table.columns.size(); ++index)
      {
        if (table.columns[index] == name)
        {
          if (found)
          {
            return std::nullopt;
          }
          found = index;
        }
      }
      return found;
    };
    const auto x = column(reference.x), y = column(reference.y);
    const auto filter = reference.filter_column.empty() ? std::optional<std::size_t>{} : column(reference.filter_column);
    const auto valid = column("Valid");
    if (std::count(table.columns.begin(),table.columns.end(),"Valid") > 1)
    {
      output.status = "The row-validity column is ambiguous.";
      return true;
    }
    if (!x || !y || *x >= table.numeric.size() || *y >= table.numeric.size() || !table.numeric[*x] || !table.numeric[*y] ||
        (!reference.filter_column.empty() && !filter))
    {
      output.status = "Numeric X/Y columns or the filter column are missing or ambiguous.";
      return true;
    }
    output.x_title = reference.x;
    output.y_title = reference.y;
    output.time_axis = reference.x == "Time";
    output.frame_axis = reference.x == "Frame";
    std::set<std::size_t> seenFrames;
    bool identity = false;
    std::vector<std::size_t> identityColumns;
    std::optional<std::string> identityValue;
    for (const auto& name : table.columns)
    {
      identity = identity || name == "NodeId" || name == "ElementId" || name == "IntegrationPointId" || name == "SlaveElementId";
    }
    for (std::size_t index = 0; index < table.columns.size(); ++index)
    {
      const auto& name = table.columns[index];
      if (name == "NodeId" || name == "ElementId" || name == "IntegrationPointId" || name == "SlaveElementId" || name == "SlaveFace")
      {
        identityColumns.push_back(index);
      }
    }
    for (std::size_t row = 0; row < table.rows; ++row)
    {
      // Compare exact source cell text, never convert identifiers through double.
      if (filter && result_table_cell(table,row,*filter) != reference.filter_value)
      {
        continue;
      }
      if (output.time_axis || output.frame_axis)
      {
        std::string current;
        for (auto index : identityColumns)
        {
          const auto cell = result_table_cell(table,row,index);
          current += std::to_string(cell.size())+":"+cell;
        }
        if (identityValue && *identityValue != current)
        {
          error = "Multiple entity histories remain; choose an exact filter or an entity-history plot.";
          return false;
        }
        identityValue = current;
      }
      PlotSample sample;
      sample.frame = result_table_row_frame(table,row);
      if (sample.frame && *sample.frame < data->increments.size())
      {
        const auto& frame = data->increments[*sample.frame];
        sample.time = frame.time;
        sample.step = frame.step;
        sample.increment = frame.increment;
        if (identity && (output.time_axis || output.frame_axis) && !seenFrames.insert(*sample.frame).second)
        {
          error = "Multiple entity records remain per frame; choose an exact filter or use an entity-history plot.";
          return false;
        }
      }
      sample.valid = numeric(result_table_cell(table,row,*x),sample.x) && numeric(result_table_cell(table,row,*y),sample.y);
      if (valid)
      {
        double flag = 0;
        sample.valid = sample.valid && numeric(result_table_cell(table,row,*valid),flag) && flag > 0;
      }
      output.samples.push_back(sample);
    }
    if (output.samples.empty())
    {
      output.status = "No rows match the filter.";
    }
    return true;
  }
}
