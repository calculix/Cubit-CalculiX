#include "CoreJobs.hpp"
#include "Framework/System/SystemResourceReader.hpp"
#include "CubitMessage.hpp"
#include <QUuid>
#include <exception>
#include <utility>

void CoreJobs::capture_monitor_execution(int job_id, std::uint64_t pid)
{
  monitorExecutions.erase(job_id);
  try
  {
    MonitorExecution execution;
    execution.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
    const auto process = SystemResourceReader::read_process(pid);
    if (process)
    {
      execution.process = process->process;
    }
    monitorExecutions.emplace(job_id, std::move(execution));
  }
  catch (const std::exception& exception)
  {
    // Monitoring failure must never turn a successfully launched job into failure.
    PRINT_WARNING("Cannot track resource usage for job %d: %s\n", job_id, exception.what());
  }
}

void CoreJobs::clear_monitor_executions() noexcept
{
  monitorExecutions.clear();
}

bool CoreJobs::get_monitor_snapshot(std::vector<JobMonitorEntry>& output, std::string& error) const
{
  error.clear();
  try
  {
    std::vector<JobMonitorEntry> candidate;
    for (const auto& row : jobs_data)
    {
      if (row.size() < 5)
      {
        error = "Invalid job record during resource monitoring.";
        return false;
      }
      JobMonitorEntry entry;
      entry.applicationId = "calculix";
      entry.commandPrefix = "ccx";
      entry.jobId = std::stoi(row[0]);
      entry.name = row[1];
      switch (std::stoi(row[3]))
      {
        case -1: entry.state = JobRunState::NotStarted; break;
        case 1: entry.state = JobRunState::Running; break;
        case 2: entry.state = JobRunState::Finished; break;
        case 3: entry.state = JobRunState::Killed; break;
        case 4: entry.state = JobRunState::Failed; break;
        case 5: entry.state = JobRunState::Waiting; break;
        default: entry.state = JobRunState::Unknown; break;
      }
      const auto execution = monitorExecutions.find(entry.jobId);
      if (execution != monitorExecutions.end())
      {
        entry.executionId = execution->second.id;
        if ((entry.state == JobRunState::Running || entry.state == JobRunState::Waiting) &&
            execution->second.process && row[4] == std::to_string(execution->second.process->pid))
        {
          entry.solverProcess = execution->second.process;
        }
      }
      // Saved model PIDs never create a live association: only launches above do.
      candidate.push_back(std::move(entry));
    }
    output.swap(candidate);
    return true;
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
    return false;
  }
}
