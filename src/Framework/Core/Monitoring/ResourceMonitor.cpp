#include "ResourceMonitor.hpp"
#include <algorithm>
#include <cmath>
#include <utility>

ResourceMonitor::~ResourceMonitor()
{
  // Join before destroying the reader/history captured by the worker.
  if (pending.valid())
  {
    pending.wait();
  }
}

bool ResourceMonitor::request(std::vector<JobMonitorEntry> jobs, std::vector<std::string> errors)
{
  if (pending.valid())
  {
    return false;
  }
  pending = std::async(std::launch::async, [this, jobs = std::move(jobs), errors = std::move(errors)]() mutable
  {
    return sample(std::move(jobs), std::move(errors));
  });
  return true;
}

std::optional<ResourceSnapshot> ResourceMonitor::take_snapshot()
{
  if (!pending.valid() || pending.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
  {
    return std::nullopt;
  }
  return pending.get();
}

ProcessResourceSample ResourceMonitor::process_sample(const ProcessIdentity& process, unsigned processors,
  std::map<std::pair<std::uint64_t, std::uint64_t>, ProcessResourceCounters>& next)
{
  ProcessResourceSample sample;
  const auto current = SystemResourceReader::read_process(process.pid);
  if (!current || current->process.birthToken != process.birthToken)
  {
    return sample;
  }
  sample.residentBytes = current->residentBytes;
  const auto key = std::make_pair(process.pid, process.birthToken);
  const auto found = previous.find(key);
  if (found != previous.end() && processors > 0)
  {
    const double elapsed = std::chrono::duration<double>(current->sampledAt - found->second.sampledAt).count();
    const double cpu = current->cpuSeconds - found->second.cpuSeconds;
    // Percentage of total online-machine capacity, not percentage of one core.
    if (elapsed > 0 && cpu >= 0 && std::isfinite(cpu))
    {
      sample.cpuPercent = std::clamp(100.0 * cpu / (elapsed * processors), 0.0, 100.0);
    }
  }
  next.insert_or_assign(key, *current);
  return sample;
}

ResourceSnapshot ResourceMonitor::sample(std::vector<JobMonitorEntry> jobs, std::vector<std::string> errors)
{
  if (!reader)
  {
    reader = std::make_unique<SystemResourceReader>();
  }
  ResourceSnapshot snapshot;
  snapshot.machine = reader->sample_machine();
  std::map<std::pair<std::uint64_t, std::uint64_t>, ProcessResourceCounters> next;
  const auto cubit = SystemResourceReader::read_process(SystemResourceReader::current_pid());
  if (cubit)
  {
    snapshot.cubit = process_sample(cubit->process, snapshot.machine.logicalProcessors, next);
  }
  for (auto& entry : jobs)
  {
    JobResourceSample job;
    if (entry.solverProcess)
    {
      job.usage = process_sample(*entry.solverProcess, snapshot.machine.logicalProcessors, next);
    }
    job.job = std::move(entry);
    snapshot.jobs.push_back(std::move(job));
  }
  previous.swap(next);
  snapshot.providerErrors = std::move(errors);
  snapshot.sampledAt = std::chrono::steady_clock::now();
  return snapshot;
}
