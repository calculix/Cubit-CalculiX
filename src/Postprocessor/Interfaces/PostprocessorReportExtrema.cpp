#include "PostprocessorReportExtrema.hpp"
#include <vtkDataSet.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <future>
#include <thread>
#include <utility>
#include <exception>

PostprocessorReportExtrema::PostprocessorReportExtrema() = default;

void PostprocessorReportExtrema::accumulate(const std::vector<double>& values,
  std::size_t offset, const std::atomic<bool>& stop)
{
  for (std::size_t index = 0; index < values.size(); ++index)
  {
    if (index % 1024 == 0 && stop.load(std::memory_order_relaxed))
    {
      return;
    }
    const double value = values[index];
    if (!std::isfinite(value))
    {
      continue;
    }
    const auto sample = offset + index;
    if (!available || value < minimum || (value == minimum && sample < minimum_sample))
    {
      minimum = value;
      minimum_sample = sample;
    }
    if (!available || value > maximum || (value == maximum && sample < maximum_sample))
    {
      maximum = value;
      maximum_sample = sample;
    }
    available = true;
  }
}

void PostprocessorReportExtrema::merge(const PostprocessorReportExtrema& other)
{
  if (!other.available)
  {
    return;
  }
  if (!available || other.minimum < minimum ||
      (other.minimum == minimum && other.minimum_sample < minimum_sample))
  {
    minimum = other.minimum;
    minimum_sample = other.minimum_sample;
  }
  if (!available || other.maximum > maximum ||
      (other.maximum == maximum && other.maximum_sample < maximum_sample))
  {
    maximum = other.maximum;
    maximum_sample = other.maximum_sample;
  }
  available = true;
}

bool PostprocessorReportExtrema::collect(vtkDataSet* mesh, const SimResults::Field& raw,
  const Postprocessor::FieldSelection& selection, ReportProgress* progress,
  PostprocessorReportExtrema& output, std::string& error)
{
  if (!mesh)
  {
    error = "Report extrema require a retained result mesh.";
    return false;
  }
  std::size_t samples = static_cast<std::size_t>(mesh->GetNumberOfPoints());
  if (raw.association == SimResults::Association::Element)
  {
    samples = static_cast<std::size_t>(mesh->GetNumberOfCells());
  }
  constexpr std::size_t chunkSize = 65536;
  const std::size_t workers = (std::max)(std::size_t(1),
    (std::min)(std::size_t(4), static_cast<std::size_t>(std::thread::hardware_concurrency())));
  std::atomic<bool> stop{false};
  std::vector<std::future<PostprocessorReportExtrema>> tasks;
  PostprocessorReportExtrema candidate;
  bool ok = true;
  // Futures are always drained while stop and task-owned inputs are alive.
  const auto drain = [&]()
  {
    for (auto& task : tasks)
    {
      while (task.wait_for(std::chrono::milliseconds(20)) != std::future_status::ready)
      {
        if (ok && !ReportProgress::advance(progress, ReportProgressStage::Results,
            "Reducing block component samples", 0, 0, error))
        {
          ok = false;
          stop.store(true, std::memory_order_relaxed);
        }
      }
      try
      {
        candidate.merge(task.get());
      }
      catch (const std::exception& failure)
      {
        if (ok)
        {
          error = "Report extrema worker failed: " + std::string(failure.what());
        }
        ok = false;
        stop.store(true, std::memory_order_relaxed);
      }
    }
    tasks.clear();
  };
  try
  {
    for (std::size_t start = 0; start < samples && ok; start += chunkSize)
    {
      if (!ReportProgress::advance(progress, ReportProgressStage::Results,
          "Reading block component samples", 0, 0, error))
      {
        ok = false;
        break;
      }
      const auto count = (std::min)(chunkSize, samples - start);
      std::vector<double> values;
      if (!Postprocessor::color_values_range(mesh, raw, selection, start, count, values, error))
      {
        ok = false;
        break;
      }
      if (workers == 1 || samples <= chunkSize)
      {
        PostprocessorReportExtrema partial;
        partial.accumulate(values, start, stop);
        candidate.merge(partial);
      }
      else
      {
        tasks.push_back(std::async(std::launch::async,
          [values = std::move(values), start, &stop]()
          {
            PostprocessorReportExtrema partial;
            partial.accumulate(values, start, stop);
            return partial;
          }));
        if (tasks.size() >= workers)
        {
          drain();
        }
      }
    }
  }
  catch (const std::exception& failure)
  {
    error = "Report extrema preparation failed: " + std::string(failure.what());
    ok = false;
  }
  if (!ok)
  {
    stop.store(true, std::memory_order_relaxed);
  }
  drain();
  if (!ok)
  {
    return false;
  }
  output = candidate;
  error.clear();
  return true;
}
