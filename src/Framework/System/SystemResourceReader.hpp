#ifndef CUBITSIM_SYSTEMRESOURCEREADER_HPP
#define CUBITSIM_SYSTEMRESOURCEREADER_HPP

#include "ProcessIdentity.hpp"
#include <chrono>
#include <memory>
#include <optional>

struct MachineResourceSample
{
  std::optional<double> cpuPercent;
  std::optional<std::uint64_t> physicalUsedBytes;
  std::optional<std::uint64_t> physicalTotalBytes;
  unsigned logicalProcessors = 0;
};

struct ProcessResourceCounters
{
  ProcessIdentity process;
  std::chrono::steady_clock::time_point sampledAt;
  double cpuSeconds = 0;
  std::optional<std::uint64_t> residentBytes;
};

class SystemResourceReader
{
public:
  SystemResourceReader();
  ~SystemResourceReader();
  SystemResourceReader(const SystemResourceReader&) = delete;
  SystemResourceReader& operator=(const SystemResourceReader&) = delete;
  MachineResourceSample sample_machine();
  // Both identity and counters are read from the same native process instance.
  static std::optional<ProcessResourceCounters> read_process(std::uint64_t pid);
  static std::uint64_t current_pid() noexcept;

private:
  struct State;
  std::unique_ptr<State> state;
};

#endif
