#include "SystemResourceReader.hpp"
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <exception>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

struct SystemResourceReader::State
{
  std::uint64_t total = 0;
  std::uint64_t idle = 0;
  bool primed = false;
};

SystemResourceReader::SystemResourceReader() : state(std::make_unique<State>())
{
}

SystemResourceReader::~SystemResourceReader() = default;

MachineResourceSample SystemResourceReader::sample_machine()
{
  MachineResourceSample sample;
  const long processors = sysconf(_SC_NPROCESSORS_ONLN);
  if (processors > 0)
  {
    sample.logicalProcessors = static_cast<unsigned>(processors);
  }
  std::ifstream stat("/proc/stat");
  std::string line;
  if (std::getline(stat, line))
  {
    std::istringstream fields(line);
    std::string label;
    fields >> label;
    std::uint64_t ticks[8] = {};
    bool valid = label == "cpu";
    for (auto& value : ticks)
    {
      valid = static_cast<bool>(fields >> value) && valid;
    }
    if (valid)
    {
      // guest/guest_nice are already included in user/nice: do not add twice.
      std::uint64_t total = 0;
      for (const auto value : ticks)
      {
        total += value;
      }
      const auto idle = ticks[3] + ticks[4];
      if (state->primed && total > state->total && idle >= state->idle && idle - state->idle <= total - state->total)
      {
        sample.cpuPercent = 100.0 * (1.0 - static_cast<double>(idle - state->idle) / static_cast<double>(total - state->total));
      }
      state->total = total;
      state->idle = idle;
      state->primed = true;
    }
    else
    {
      state->primed = false;
    }
  }
  std::ifstream meminfo("/proc/meminfo");
  std::optional<std::uint64_t> totalMemory, available;
  while (std::getline(meminfo, line))
  {
    std::istringstream fields(line);
    std::string key, unit;
    std::uint64_t value = 0;
    if (fields >> key >> value >> unit && unit == "kB" && value <= (std::numeric_limits<std::uint64_t>::max)() / 1024)
    {
      if (key == "MemTotal:")
      {
        totalMemory = value * 1024;
      }
      else if (key == "MemAvailable:")
      {
        available = value * 1024;
      }
    }
  }
  sample.physicalTotalBytes = totalMemory;
  if (totalMemory && available && *available <= *totalMemory)
  {
    sample.physicalUsedBytes = *totalMemory - *available;
  }
  return sample;
}

std::optional<ProcessResourceCounters> SystemResourceReader::read_process(std::uint64_t pid)
{
  if (pid == 0)
  {
    return std::nullopt;
  }
  std::ifstream stat("/proc/" + std::to_string(pid) + "/stat");
  std::ostringstream contents;
  contents << stat.rdbuf();
  const auto line = contents.str();
  if (line.empty())
  {
    return std::nullopt;
  }
  // comm may contain spaces and parentheses; fields after its LAST ')' are fixed.
  const auto closing = line.rfind(')');
  if (closing == std::string::npos)
  {
    return std::nullopt;
  }
  std::istringstream fields(line.substr(closing + 1));
  std::vector<std::string> values;
  std::string value;
  while (fields >> value)
  {
    values.push_back(value);
  }
  if (values.size() < 22 || values[0] == "Z" || values[0] == "X")
  {
    return std::nullopt;
  }
  const long frequency = sysconf(_SC_CLK_TCK);
  const long pageSize = sysconf(_SC_PAGESIZE);
  if (frequency <= 0)
  {
    return std::nullopt;
  }
  try
  {
    ProcessResourceCounters sample;
    // values[0] is field 3: utime=14, stime=15, starttime=22, rss=24.
    sample.process = {pid, std::stoull(values[19])};
    sample.cpuSeconds = (static_cast<double>(std::stoull(values[11])) + static_cast<double>(std::stoull(values[12]))) / frequency;
    const auto pages = std::stoll(values[21]);
    if (pages >= 0 && pageSize > 0 && static_cast<std::uint64_t>(pages) <= (std::numeric_limits<std::uint64_t>::max)() / static_cast<std::uint64_t>(pageSize))
    {
      sample.residentBytes = static_cast<std::uint64_t>(pages) * static_cast<std::uint64_t>(pageSize);
    }
    sample.sampledAt = std::chrono::steady_clock::now();
    return sample;
  }
  catch (const std::exception&)
  {
    return std::nullopt;
  }
}

std::uint64_t SystemResourceReader::current_pid() noexcept
{
  return static_cast<std::uint64_t>(getpid());
}
