#ifndef POSTPROCESSOR_RESULTTABLES_HPP
#define POSTPROCESSOR_RESULTTABLES_HPP
#include "Framework/Results/VtkResultsData.hpp"
#include <string>
#include <vector>
class vtkDataSet;
namespace Postprocessor
{
  struct TableReference
  {
    std::string source_id, key;
    bool field = false;
    std::size_t frame = 0;
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
    std::size_t dataset = 0, first_row = 0, count = 0;
  };
  // Read-only borrowed VTK access, retained by the immutable owner snapshot.
  // Spatial views adapt existing arrays; no duplicate vtkTable is materialized.
  struct ResultTableData
  {
    std::shared_ptr<const SimResults::VtkResultsData> owner;
    std::shared_ptr<vtkTable> table;
    std::vector<ResultTablePart> parts;
    std::vector<std::string> columns;
    std::vector<bool> numeric;
    std::string title, status;
    std::size_t rows = 0;
  };
  std::vector<ResultTableDescriptor> result_tables(const SimResults::VtkResultsData& data, std::optional<std::size_t> frame);
  ResultTableData result_table_data(std::shared_ptr<const SimResults::VtkResultsData> data, const TableReference& reference);
  std::string result_table_cell(const ResultTableData& data, std::size_t row, std::size_t column);
}
#endif
