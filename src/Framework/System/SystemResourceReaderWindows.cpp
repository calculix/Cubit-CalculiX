#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <psapi.h>
#include <pdh.h>
#include <pdhmsg.h>
#include "SystemResourceReader.hpp"
#include <algorithm>
#include <limits>

namespace
{
std::uint64_t ticks(const FILETIME& value)
{
  return (static_cast<std::uint64_t>(value.dwHighDateTime) << 32) | value.dwLowDateTime;
}

struct ProcessHandle
{
  HANDLE value;
  ~ProcessHandle()
  {
    if (value != nullptr)
    {
      CloseHandle(value);
    }
  }
};
}

struct SystemResourceReader::State
{
  PDH_HQUERY query = nullptr;
  PDH_HCOUNTER cpu = nullptr;
  bool primed = false;
  State()
  {
    if (PdhOpenQueryW(nullptr, 0, &query) == ERROR_SUCCESS)
    {
      if (PdhAddEnglishCounterW(query, L"\\Processor Information(_Total)\\% Processor Time", 0, &cpu) != ERROR_SUCCESS)
      {
        cpu = nullptr;
      }
    }
  }
  ~State()
  {
    if (query != nullptr)
    {
      PdhCloseQuery(query);
    }
  }
};

SystemResourceReader::SystemResourceReader() : state(std::make_unique<State>())
{
}

SystemResourceReader::~SystemResourceReader() = default;

MachineResourceSample SystemResourceReader::sample_machine()
{
  MachineResourceSample sample;
  sample.logicalProcessors = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
  MEMORYSTATUSEX memory = {};
  memory.dwLength = sizeof(memory);
  if (GlobalMemoryStatusEx(&memory))
  {
    sample.physicalTotalBytes = memory.ullTotalPhys;
    sample.physicalUsedBytes = memory.ullTotalPhys - memory.ullAvailPhys;
  }
  if (state->cpu != nullptr && PdhCollectQueryData(state->query) == ERROR_SUCCESS)
  {
    PDH_FMT_COUNTERVALUE value = {};
    if (state->primed && PdhGetFormattedCounterValue(state->cpu, PDH_FMT_DOUBLE, nullptr, &value) == ERROR_SUCCESS &&
        (value.CStatus == PDH_CSTATUS_VALID_DATA || value.CStatus == PDH_CSTATUS_NEW_DATA))
    {
      sample.cpuPercent = std::clamp(value.doubleValue, 0.0, 100.0);
    }
    state->primed = true;
  }
  else
  {
    state->primed = false;
  }
  return sample;
}

std::optional<ProcessResourceCounters> SystemResourceReader::read_process(std::uint64_t pid)
{
  if (pid == 0 || pid > (std::numeric_limits<DWORD>::max)())
  {
    return std::nullopt;
  }
  ProcessHandle handle{OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, static_cast<DWORD>(pid))};
  FILETIME created = {}, exited = {}, kernel = {}, user = {};
  DWORD exitCode = 0;
  if (handle.value == nullptr || !GetExitCodeProcess(handle.value, &exitCode) || exitCode != STILL_ACTIVE ||
      !GetProcessTimes(handle.value, &created, &exited, &kernel, &user))
  {
    return std::nullopt;
  }
  ProcessResourceCounters sample;
  sample.process = {pid, ticks(created)};
  sample.cpuSeconds = static_cast<double>(ticks(kernel) + ticks(user)) / 10000000.0;
  PROCESS_MEMORY_COUNTERS memory = {};
  memory.cb = sizeof(memory);
  if (GetProcessMemoryInfo(handle.value, &memory, sizeof(memory)))
  {
    sample.residentBytes = memory.WorkingSetSize;
  }
  sample.sampledAt = std::chrono::steady_clock::now();
  return sample;
}

std::uint64_t SystemResourceReader::current_pid() noexcept
{
  return GetCurrentProcessId();
}
