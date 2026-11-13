#ifndef POSTPROCESSOR_RESULTTABLES_HPP
#define POSTPROCESSOR_RESULTTABLES_HPP
#include "Framework/Results/VtkResultsData.hpp"
#include <string>
#include <vector>
#include <optional>
#include <array>
class vtkDataSet;
namespace Postprocessor
{
  namespace TableDefaults
  {
    inline constexpr int sizing_sample_rows = 1000;
    inline constexpr std::array<int,3> active_frame_color = {225,238,255};
  }
  struct TableReference
  {
    std::string source_id, key;
    bool field = false;
    std::size_t frame = 0; // Zero-based active-frame marker; never a history filter.
  };
  struct ResultTableDescriptor
  {
    std::string key, name, category;
    bool global = false;
  };
  struct ResultTablePart
  {
    vtkDataSet* mesh = nullptr;
    SimResults::Field field;
    // Logical row interval [first_row, first_row+count), in frame/source order.
    std::size_t dataset = 0, first_row = 0, count = 0;
    std::optional<std::size_t> frame;
    std::shared_ptr<vtkTable> table;
    // Union payload column -> this segment's native/spatial column; -1 means
    // unavailable. Segment metadata references immutable inputs, not copied rows.
    std::vector<int> column_map;
  };
  // Read-only borrowed VTK access, retained by the immutable owner snapshot.
  // Frame histories prepend Frame (1-based), Step, Increment and Time. Global
  // tables retain their supplied schema. Spatial/native segments borrow existing
  // arrays/tables; no combined vtkTable or copied history rows are materialized.
  struct ResultTableData
  {
    std::shared_ptr<const SimResults::VtkResultsData> owner;
    std::shared_ptr<vtkTable> table;
    std::vector<ResultTablePart> parts;
    std::vector<std::string> columns;
    std::vector<bool> numeric;
    std::string title, status;
    std::size_t rows = 0;
    bool frame_columns = false;
    std::optional<std::size_t> active_frame;
  };
  std::vector<ResultTableDescriptor> result_tables(const SimResults::VtkResultsData& data, std::optional<std::size_t> frame);
  ResultTableData result_table_data(std::shared_ptr<const SimResults::VtkResultsData> data, const TableReference& reference);
  // Binary-search the frame owning a logical row. Globals have no frame.
  std::optional<std::size_t> result_table_row_frame(const ResultTableData& data, std::size_t row);
  std::string result_table_cell(const ResultTableData& data, std::size_t row, std::size_t column);
}
#endif
