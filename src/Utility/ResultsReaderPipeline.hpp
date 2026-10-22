#ifndef RESULTSREADERPIPELINE_HPP
#define RESULTSREADERPIPELINE_HPP

#include "AppUtil.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"
#include "ResultReadTimings.hpp"

#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <deque>
#include <exception>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace ResultsReaderPipeline
{
constexpr std::size_t max_batch_lines = 4096;
constexpr std::size_t max_batch_bytes = 2 * 1024 * 1024;
constexpr std::size_t max_pending_bytes = 64 * 1024 * 1024;
constexpr std::size_t max_line_bytes = 64 * 1024;
constexpr std::size_t max_fields = 128;

inline std::size_t line_storage(const std::vector<std::string>& fields)
{
  std::size_t bytes = sizeof(fields) + fields.capacity() * sizeof(std::string);
  for (const auto& field : fields)
  {
    bytes += field.capacity() + 1;
  }
  return bytes;
}

// Include numeric output and lookup rows in the batch estimate. The extra six
// components cover the existing stress/strain derived calculations.
inline std::size_t result_row_storage(std::size_t components, std::size_t mapping_columns)
{
  return sizeof(std::vector<double>) + (components + 6) * sizeof(double)
      + sizeof(std::vector<int>) + mapping_columns * sizeof(int);
}

inline bool ids_fit(std::size_t existing, std::size_t incoming)
{
  const auto maximum = static_cast<std::size_t>(std::numeric_limits<int>::max());
  return existing <= maximum && incoming <= maximum - existing;
}

// Bound the raw input allocation even for a corrupt file without newlines.
// gcount() includes the delimiter when one was extracted, but not at EOF.
inline bool read_line(std::ifstream& file, std::string& line)
{
  std::array<char, max_line_bytes + 2> buffer;
  file.getline(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  if (file.fail())
  {
    return false;
  }
  auto count = file.gcount();
  if (!file.eof() && count > 0)
  {
    --count;
  }
  line.assign(buffer.data(), static_cast<std::size_t>(count));
  return true;
}

inline bool file_size(std::ifstream& file, std::streamoff& size)
{
  file.seekg(0, std::ios::end);
  const auto end = file.tellg();
  if (end == std::streampos(-1))
  {
    return false;
  }
  size = static_cast<std::streamoff>(end);
  file.seekg(0, std::ios::beg);
  return static_cast<bool>(file);
}

inline bool poll(std::ifstream& file, std::streamoff size, ProgressTool* progressbar, std::chrono::steady_clock::time_point& previous)
{
  const auto now = std::chrono::steady_clock::now();
  if (now - previous < std::chrono::milliseconds(100))
  {
    return true;
  }
  previous = now;
  double fraction = 0.0;
  if (size > 0)
  {
    // Do not call tellg() at EOF: it would set failbit on the input stream.
    const auto position = file.eof() ? std::streampos(-1) : file.tellg();
    fraction = position == std::streampos(-1) ? 0.99 : static_cast<double>(static_cast<std::streamoff>(position)) / static_cast<double>(size);
    if (fraction > 0.99)
    {
      fraction = 0.99;
    }
  }
  progressbar->percent(fraction);
  progressbar->check_interrupt();
  return !CubitInterface::app_util().get()->interrupt();
}

inline bool interrupted(ProgressTool* progressbar)
{
  progressbar->check_interrupt();
  return CubitInterface::app_util().get()->interrupt();
}

// Only next(), append(), and poll() access the active reader, on its calling
// thread. A task owns its input/output until finished publishes them.
template <typename Task, typename Next, typename Parse, typename Append, typename Poll>
bool run(std::size_t workers, const char* reader_name, Next next, Parse parse, Append append, Poll poll)
{
  if (workers == 0 || workers > std::numeric_limits<std::size_t>::max() / 2)
  {
    PRINT_ERROR("%s reader has an invalid worker count.\n", reader_name);
    return false;
  }

  // Cap estimated input/output storage independently of the selected thread count.
  const std::size_t maximum_pending = std::min(workers * 2, max_pending_bytes / max_batch_bytes);
  bool success = true;
  bool finished = false;

  try
  {
    auto cancelled = std::make_shared<std::atomic<bool>>(false);
    std::deque<std::shared_ptr<Task>> pending;
    boost::asio::thread_pool pool(workers);
    try
    {
      while (success && (!finished || !pending.empty()))
      {
        if (!poll())
        {
          success = false;
          break;
        }

        if (!finished && pending.size() < maximum_pending)
        {
          auto task = std::make_shared<Task>();
          if (!next(task->input, finished))
          {
            success = false;
            break;
          }
          if (!finished)
          {
            pending.push_back(task);
            boost::asio::post(pool, [task, cancelled, parse]()
            {
              try
              {
                task->output = parse(task->input, *cancelled);
              }
              catch (const std::exception& error)
              {
                task->output.valid = false;
                task->output.error = error.what();
              }
              catch (...)
              {
                task->output.valid = false;
                task->output.error = "Unexpected worker failure.";
              }
              // Release token storage even if an earlier task delays commit.
              decltype(task->input.lines)().swap(task->input.lines);
              task->finished.store(true, std::memory_order_release);
            });
            continue;
          }
        }

        if (pending.empty())
        {
          continue;
        }
        const auto task = pending.front();
        while (!task->finished.load(std::memory_order_acquire))
        {
          if (!poll())
          {
            success = false;
            break;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if (!success)
        {
          break;
        }
        if (!task->output.valid)
        {
          PRINT_ERROR("%s parsing failed near line %zu: %s\n", reader_name, task->input.first_line, task->output.error.c_str());
          success = false;
          break;
        }
        if (!append(task->output))
        {
          success = false;
          break;
        }
        pending.pop_front();
      }
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("%s reading failed: %s\n", reader_name, error.what());
      success = false;
    }
    catch (...)
    {
      PRINT_ERROR("Unexpected failure while reading %s results.\n", reader_name);
      success = false;
    }

    if (!success)
    {
      cancelled->store(true, std::memory_order_relaxed);
      pool.stop();
    }
    // Never access cleared reader state while workers are still running.
    pool.join();
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("%s thread pool failed: %s\n", reader_name, error.what());
    success = false;
  }
  catch (...)
  {
    PRINT_ERROR("Unexpected %s thread pool failure.\n", reader_name);
    success = false;
  }
  return success;
}
// Only the calling thread assembles batches; stored offsets survive string growth.
template <typename Batch, typename State>
void append_raw_record(Batch& batch, const State& state, const std::string& raw)
{
  if (batch.records.empty())
  {
    batch.data_id = state.data_id;
    batch.first_line = state.line_number;
    batch.result_type = state.result_type;
    batch.components = state.components;
  }
  batch.records.push_back({batch.text.size(), raw.size(), state.line_number});
  batch.text.append(raw);
}

template <typename Task, typename Next, typename Parse, typename Append, typename Poll>
bool run_raw(std::size_t workers, const char* reader_name, ResultReadTimings::Phases& phases, Next next, Parse parse, Append append, Poll poll)
{
  if (workers == 0 || workers > std::numeric_limits<std::size_t>::max() / 2)
  {
    PRINT_ERROR("%s reader has an invalid worker count.\n", reader_name);
    return false;
  }

  // Cap estimated input/output storage independently of the selected thread count.
  const std::size_t maximum_pending = std::min(workers * 2, max_pending_bytes / max_batch_bytes);
  bool success = true;
  bool finished = false;

  try
  {
    auto cancelled = std::make_shared<std::atomic<bool>>(false);
    std::deque<std::shared_ptr<Task>> pending;
    boost::asio::thread_pool pool(workers);
    try
    {
      while (success && (!finished || !pending.empty()))
      {
        if (!poll())
        {
          success = false;
          break;
        }

        if (!finished && pending.size() < maximum_pending)
        {
          auto task = std::make_shared<Task>();
          const auto scan_begin = ResultReadTimings::Clock::now();
          const bool read_success = next(task->input, finished);
          phases.scan_ns += ResultReadTimings::elapsed(scan_begin);
          if (!read_success)
          {
            success = false;
            break;
          }
          if (!finished)
          {
            pending.push_back(task);
            boost::asio::post(pool, [task, cancelled, parse]()
            {
              try
              {
                task->output = parse(task->input, *cancelled);
              }
              catch (const std::exception& error)
              {
                task->output.valid = false;
                task->output.error = error.what();
              }
              catch (...)
              {
                task->output.valid = false;
                task->output.error = "Unexpected worker failure.";
              }
              // No character views escape parse(); release raw input before commit.
              std::string().swap(task->input.text);
              decltype(task->input.records)().swap(task->input.records);
              task->finished.store(true, std::memory_order_release);
            });
            continue;
          }
        }

        if (pending.empty())
        {
          continue;
        }
        const auto task = pending.front();
        const auto wait_begin = ResultReadTimings::Clock::now();
        while (!task->finished.load(std::memory_order_acquire))
        {
          if (!poll())
          {
            success = false;
            break;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        phases.wait_ns += ResultReadTimings::elapsed(wait_begin);
        if (!success)
        {
          break;
        }
        if (!task->output.valid)
        {
          PRINT_ERROR("%s parsing failed near line %zu: %s\n", reader_name, task->input.first_line, task->output.error.c_str());
          success = false;
          break;
        }
        phases.worker_parse_ns += task->output.parse_ns;
        phases.worker_derived_ns += task->output.derived_ns;
        const auto commit_begin = ResultReadTimings::Clock::now();
        const bool append_success = append(task->output);
        phases.commit_ns += ResultReadTimings::elapsed(commit_begin);
        if (!append_success)
        {
          success = false;
          break;
        }
        pending.pop_front();
      }
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("%s reading failed: %s\n", reader_name, error.what());
      success = false;
    }
    catch (...)
    {
      PRINT_ERROR("Unexpected failure while reading %s results.\n", reader_name);
      success = false;
    }

    if (!success)
    {
      cancelled->store(true, std::memory_order_relaxed);
      pool.stop();
    }
    // Never access cleared reader state while workers are still running.
    pool.join();
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("%s thread pool failed: %s\n", reader_name, error.what());
    success = false;
  }
  catch (...)
  {
    PRINT_ERROR("Unexpected %s thread pool failure.\n", reader_name);
    success = false;
  }
  return success;
}
}

#endif // RESULTSREADERPIPELINE_HPP
