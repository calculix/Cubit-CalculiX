#ifndef CORERESULTSFRDSTRUCTS_HPP
#define CORERESULTSFRDSTRUCTS_HPP

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <string>
#include <vector>

struct FrdReadBatch
{
  int data_id = -1;
  std::size_t first_line = 0;
  std::string result_type;
  std::vector<std::string> components;
  std::vector<std::vector<std::string>> lines;
};

struct FrdParsedBatch
{
  bool valid = false;
  std::uint64_t parse_ns = 0;
  std::uint64_t derived_ns = 0;
  std::string error;
  int data_id = -1;
  std::size_t first_line = 0;
  std::vector<std::string> components;
  std::vector<std::vector<double>> values;
  std::vector<std::vector<int>> node_data;
};

struct FrdReadTask
{
  FrdReadBatch input;
  FrdParsedBatch output;
  std::atomic<bool> finished{false};
};

struct FrdReadState
{
  bool finished = false;
  bool found_end_marker = false;
  bool has_pending_line = false;
  std::size_t line_number = 0;
  std::string pending_line;
  int data_id = -1;
  std::string result_type;
  std::vector<std::string> components;
  std::streamoff file_size = 0;
  std::chrono::steady_clock::time_point progress_time{};
};

struct FrdRawRecord
{
  std::size_t offset = 0;
  std::size_t length = 0;
  std::size_t source_line = 0;
};

struct FrdRawReadBatch
{
  int data_id = -1;
  std::size_t first_line = 0;
  std::string result_type;
  std::vector<std::string> components;
  std::string text;
  std::vector<FrdRawRecord> records;
};

struct FrdRawReadTask
{
  FrdRawReadBatch input;
  FrdParsedBatch output;
  std::atomic<bool> finished{false};
};

#endif // CORERESULTSFRDSTRUCTS_HPP
