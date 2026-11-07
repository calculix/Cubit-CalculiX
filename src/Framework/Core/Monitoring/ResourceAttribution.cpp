#include "ResourceAttribution.hpp"
#include <algorithm>
#include <cmath>
#include <set>

namespace
{
ResourceAttribution attribute(const ResourceSnapshot& snapshot, bool cpu, bool includeCubit, bool includeJobs)
{
  ResourceAttribution result;
  double capacity = 100;
  std::optional<double> used;
  if (cpu)
  {
    used = snapshot.machine.cpuPercent;
    result.cubitMeasured = snapshot.cubit.cpuPercent;
  }
  else
  {
    if (snapshot.machine.physicalTotalBytes)
    {
      capacity = static_cast<double>(*snapshot.machine.physicalTotalBytes);
    }
    else
    {
      capacity = 0;
    }
    if (snapshot.machine.physicalUsedBytes)
    {
      used = static_cast<double>(*snapshot.machine.physicalUsedBytes);
    }
    if (snapshot.cubit.residentBytes)
    {
      result.cubitMeasured = static_cast<double>(*snapshot.cubit.residentBytes);
    }
  }
  result.jobsIncomplete = !snapshot.providerErrors.empty();
  std::set<std::pair<std::uint64_t, std::uint64_t>> seen;
  for (const auto& entry : snapshot.jobs)
  {
    if (entry.job.state != JobRunState::Running && entry.job.state != JobRunState::Waiting)
    {
      continue;
    }
    if (!entry.job.solverProcess)
    {
      result.jobsIncomplete = true;
      continue;
    }
    const auto identity = *entry.job.solverProcess;
    if (!seen.emplace(identity.pid, identity.birthToken).second)
    {
      continue;
    }
    std::optional<double> measured = entry.usage.cpuPercent;
    if (!cpu)
    {
      measured.reset();
      if (entry.usage.residentBytes)
      {
        measured = static_cast<double>(*entry.usage.residentBytes);
      }
    }
    if (measured && std::isfinite(*measured) && *measured >= 0)
    {
      result.jobsMeasured += *measured;
    }
    else
    {
      result.jobsIncomplete = true;
    }
  }
  const bool cubitValid = result.cubitMeasured && std::isfinite(*result.cubitMeasured) && *result.cubitMeasured >= 0;
  result.incomplete = (includeCubit && !cubitValid) || (includeJobs && result.jobsIncomplete);
  if (!used || !std::isfinite(*used) || *used < 0 || capacity <= 0)
  {
    return result;
  }
  const double machineUsed = std::clamp(*used, 0.0, capacity);
  const double cubit = includeCubit && cubitValid ? *result.cubitMeasured : 0;
  const double jobs = includeJobs ? result.jobsMeasured : 0;
  const double sum = cubit + jobs;
  const double scale = sum > machineUsed && sum > 0 ? machineUsed / sum : 1.0;
  result.available = true;
  result.adjusted = scale < 1.0;
  result.cubit = cubit * scale / capacity;
  result.jobs = jobs * scale / capacity;
  result.otherShared = std::max(0.0, machineUsed - sum * scale) / capacity;
  return result;
}
}

ResourceAttribution memory_attribution(const ResourceSnapshot& snapshot, bool includeCubit, bool includeJobs)
{
  return attribute(snapshot, false, includeCubit, includeJobs);
}

ResourceAttribution cpu_attribution(const ResourceSnapshot& snapshot, bool includeCubit, bool includeJobs)
{
  return attribute(snapshot, true, includeCubit, includeJobs);
}
