#ifndef CUBITSIM_RESOURCEMONITOR_HPP
#define CUBITSIM_RESOURCEMONITOR_HPP

#include "Framework/Interfaces/JobMonitorProviderInterface.hpp"
#include "Framework/System/SystemResourceReader.hpp"
#include <future>
#include <map>

struct ProcessResourceSample
{
  std::optional<double> cpuPercent;
  std::optional<std::uint64_t> residentBytes;
};

struct JobResourceSample
{
  JobMonitorEntry job;
  ProcessResourceSample usage;
};

struct ResourceSnapshot
{
  std::chrono::steady_clock::time_point sampledAt;
  MachineResourceSample machine;
  ProcessResourceSample cubit;
  std::vector<JobResourceSample> jobs;
  std::vector<std::string> providerErrors;
};

class ResourceMonitor
{
public:
  ~ResourceMonitor();
  // Owner-thread API. At most one worker is outstanding; no request backlog.
  bool request(std::vector<JobMonitorEntry> jobs, std::vector<std::string> errors);
  std::optional<ResourceSnapshot> take_snapshot();

private:
  ResourceSnapshot sample(std::vector<JobMonitorEntry> jobs, std::vector<std::string> errors);
  ProcessResourceSample process_sample(const ProcessIdentity& process, unsigned processors,
    std::map<std::pair<std::uint64_t, std::uint64_t>, ProcessResourceCounters>& next);
  std::unique_ptr<SystemResourceReader> reader;
  std::map<std::pair<std::uint64_t, std::uint64_t>, ProcessResourceCounters> previous;
  std::future<ResourceSnapshot> pending;
};

#endif
