#ifndef CORERESULTSVTKPIPELINE_HPP
#define CORERESULTSVTKPIPELINE_HPP

#include "CoreResultsVtkStructs.hpp"
#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <exception>
#include <limits>
#include <memory>
#include <mutex>
#include <string>

/** One reusable pool per complete conversion. No VTK/Cubit API calls occur in
 * workers: work writes exclusive preallocated numeric ranges. The byte budget
 * bounds pending work/scratch estimates, not resident datasets or mesh indexes.
 * The caller keeps every captured source/destination alive until run returns.
 */
class CoreResultsVtkPipeline
{
public:
  CoreResultsVtkPipeline(const CoreResultsVtkOptions& options, std::atomic<bool>& cancelled)
    : options(options), cancelled(cancelled), pool(options.workers)
  {
  }

  ~CoreResultsVtkPipeline()
  {
    finish();
  }

  CoreResultsVtkPipeline(const CoreResultsVtkPipeline&) = delete;
  CoreResultsVtkPipeline& operator=(const CoreResultsVtkPipeline&) = delete;

  static bool valid(const CoreResultsVtkOptions& options, std::string& error)
  {
    if (options.workers == 0 || options.workers > std::numeric_limits<std::size_t>::max() / 2 || options.chunk_rows == 0 || options.chunk_bytes < 256 || options.pending_bytes < options.chunk_bytes)
    {
      error = "Invalid VTK worker count or conversion chunk/budget settings.";
      return false;
    }
    return true;
  }

  template<class Work, class Poll>
  bool run(std::size_t rows, std::size_t bytes_per_row, Work work, Poll poll, std::string& error)
  {
    struct Task
    {
      std::atomic<bool> finished{false};
      std::mutex mutex;
      std::condition_variable ready;
      bool success = true;
      std::size_t end = 0;
      std::string error;
    };
    error.clear();
    if (joined || bytes_per_row > options.chunk_bytes)
    {
      error = "VTK task exceeds the chunk budget or its pool is already closed.";
      return false;
    }
    const std::size_t stride = std::min(options.chunk_rows, options.chunk_bytes / std::max<std::size_t>(1, bytes_per_row));
    const std::size_t estimate = std::max<std::size_t>(256, stride * std::max<std::size_t>(1, bytes_per_row));
    const std::size_t limit = std::min(options.workers * 2, options.pending_bytes / estimate);
    if (stride == 0 || limit == 0)
    {
      error = "VTK task cannot fit in the pending-work budget.";
      return false;
    }
    std::deque<std::shared_ptr<Task>> pending;
    std::size_t next = 0;
    std::size_t completed = 0;
    bool success = true;
    try
    {
      // Avoid dispatch/wakeup costs for one-worker runs and small single chunks.
      // Keep direct work bounded by stride so progress/cancellation still poll.
      const bool direct = options.workers == 1 ||
        (rows <= stride && rows <= (64 * 1024) / std::max<std::size_t>(1, bytes_per_row));
      if (direct)
      {
        while (next < rows && !cancelled.load(std::memory_order_relaxed))
        {
          if (!poll(static_cast<double>(next) / static_cast<double>(rows)))
          {
            cancelled.store(true, std::memory_order_relaxed);
            break;
          }
          const auto end = next + std::min(stride, rows - next);
          if (!work(next, end, cancelled, error))
          {
            if (error.empty())
            {
              error = "VTK conversion worker failed.";
            }
            success = false;
            break;
          }
          next = end;
        }
      }
      else
      {
        while (!cancelled.load(std::memory_order_relaxed) && (next < rows || !pending.empty()))
        {
          if (!poll(rows == 0 ? 1.0 : static_cast<double>(completed) / static_cast<double>(rows)))
          {
            cancelled.store(true, std::memory_order_relaxed);
            break;
          }
          if (next < rows && pending.size() < limit)
          {
            const std::size_t begin = next;
            next += std::min(stride, rows - next);
            auto task = std::make_shared<Task>();
            task->end = next;
            pending.push_back(task);
            boost::asio::post(pool, [task, begin, work, this]()
            {
              try
              {
                if (!cancelled.load(std::memory_order_relaxed))
                {
                  task->success = work(begin, task->end, cancelled, task->error);
                }
              }
              catch (const std::exception& failure)
              {
                task->success = false;
                task->error = failure.what();
              }
              catch (...)
              {
                task->success = false;
                task->error = "Unexpected VTK conversion worker failure.";
              }
              {
                std::lock_guard<std::mutex> lock(task->mutex);
                task->finished.store(true, std::memory_order_release);
              }
              task->ready.notify_one();
            });
            continue;
          }
          const auto task = pending.front();
          while (!task->finished.load(std::memory_order_acquire))
          {
            if (!poll(rows == 0 ? 1.0 : static_cast<double>(completed) / static_cast<double>(rows)))
            {
              cancelled.store(true, std::memory_order_relaxed);
              break;
            }
            std::unique_lock<std::mutex> lock(task->mutex);
            task->ready.wait_for(lock, std::chrono::milliseconds(20), [&]()
            {
              return task->finished.load(std::memory_order_acquire) || cancelled.load(std::memory_order_relaxed);
            });
          }
          if (cancelled.load(std::memory_order_relaxed))
          {
            break;
          }
          if (!task->success)
          {
            error = task->error.empty() ? "VTK conversion worker failed." : task->error;
            success = false;
            break;
          }
          completed = task->end;
          pending.pop_front();
        }
      }
      if (success && !cancelled.load(std::memory_order_relaxed) && !poll(1.0))
      {
        cancelled.store(true, std::memory_order_relaxed);
      }
    }
    catch (const std::exception& failure)
    {
      error = failure.what();
      success = false;
    }
    catch (...)
    {
      error = "Unexpected VTK task scheduling failure.";
      success = false;
    }
    if (!success || cancelled.load(std::memory_order_relaxed))
    {
      cancelled.store(true, std::memory_order_relaxed);
      pool.stop();
      finish();
      // Workers may have failed while cancellation was requested. Inspect them
      // only after join, and retain a real failure instead of calling it cancel.
      for (const auto& task : pending)
      {
        if (task->finished.load(std::memory_order_acquire) && !task->success && error.empty())
        {
          error = task->error.empty() ? "VTK conversion worker failed." : task->error;
        }
      }
      return false;
    }
    return true;
  }

  void finish()
  {
    if (!joined)
    {
      pool.join();
      joined = true;
    }
  }

private:
  const CoreResultsVtkOptions& options;
  std::atomic<bool>& cancelled;
  boost::asio::thread_pool pool;
  bool joined = false;
};

#endif // CORERESULTSVTKPIPELINE_HPP
