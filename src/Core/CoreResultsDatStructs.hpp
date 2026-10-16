#ifndef CORERESULTSDATSTRUCTS_HPP
#define CORERESULTSDATSTRUCTS_HPP

#include <atomic>
#include <chrono>
#include <cstddef>
#include <ios>
#include <string>
#include <vector>

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
};

#endif // CORERESULTSDATSTRUCTS_HPP
