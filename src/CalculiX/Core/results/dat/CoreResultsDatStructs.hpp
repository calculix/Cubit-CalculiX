#ifndef CORERESULTSDATSTRUCTS_HPP
#define CORERESULTSDATSTRUCTS_HPP

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <string>
#include <vector>

/** Non-spatial records retain reported identifiers and all contributions.
 * Numeric IDs are at most the reader's signed int range and are exactly
 * representable in double. Column names explicitly distinguish node labels,
 * element+face codes and face integration points; none imply a mesh-node map.
 */
struct DatResultTable
{
  std::string key;
  std::string name;
  std::string set_name;
  std::string slave_set;
  std::string master_set;
  int step = -1;
  int increment = -1;
  double time = 0.0;
  std::size_t source_line = 0;
  std::vector<std::string> columns;
  std::vector<std::vector<double>> rows;
};

struct DatReadBatch
{
  int data_id = -1;
  std::size_t first_line = 0;
  std::string result_type;
  std::vector<std::string> components;
  std::vector<std::vector<std::string>> lines;
};

struct DatParsedBatch
{
  bool valid = false;
  std::uint64_t parse_ns = 0;
  std::uint64_t derived_ns = 0;
  std::string error;
  int data_id = -1;
  std::size_t first_line = 0;
  std::vector<std::string> components;
  std::vector<std::vector<double>> values;
  std::vector<std::vector<int>> c1_data;
};

struct DatReadTask
{
  DatReadBatch input;
  DatParsedBatch output;
  std::atomic<bool> finished{false};
};

struct DatReadState
{
  bool finished = false;
  bool has_pending_line = false;
  std::size_t line_number = 0;
  std::string pending_line;
  int data_id = -1;
  std::string result_type;
  std::vector<std::string> components;
  std::streamoff file_size = 0;
  std::chrono::steady_clock::time_point progress_time{};
  int association = 0; // Explicit: 1 node, 2 integration point, 3 element.
  bool variable_components = false;
  int table_id = -1;
  std::size_t table_width = 0;
  bool summary_section = false;
};

struct DatRawRecord
{
  std::size_t offset = 0;
  std::size_t length = 0;
  std::size_t source_line = 0;
};

struct DatRawReadBatch
{
  int data_id = -1;
  std::size_t first_line = 0;
  std::string result_type;
  std::vector<std::string> components;
  std::string text;
  std::vector<DatRawRecord> records;
  int association = 0;
};

struct DatRawReadTask
{
  DatRawReadBatch input;
  DatParsedBatch output;
  std::atomic<bool> finished{false};
};

#endif // CORERESULTSDATSTRUCTS_HPP
