#ifndef CUBITSIM_JOBMONITORPROVIDERINTERFACE_HPP
#define CUBITSIM_JOBMONITORPROVIDERINTERFACE_HPP

#include "Framework/System/ProcessIdentity.hpp"
#include <optional>
#include <string>
#include <vector>

enum class JobRunState
{
  NotStarted,
  Running,
  Finished,
  Killed,
  Failed,
  Waiting,
  Unknown
};

struct JobMonitorEntry
{
  std::string applicationId;
  std::string commandPrefix;
  int jobId = 0;
  std::string name;
  std::string executionId;
  JobRunState state = JobRunState::Unknown;
  std::optional<ProcessIdentity> solverProcess;
};

class JobMonitorProviderInterface
{
public:
  virtual ~JobMonitorProviderInterface() = default;
  // Owner-thread query only: copy runtime data, never update/complete jobs.
  // No borrowed solver handles or callbacks may escape into the sampling worker.
  virtual bool snapshot(std::vector<JobMonitorEntry>& output, std::string& error) const = 0;
};

#endif
