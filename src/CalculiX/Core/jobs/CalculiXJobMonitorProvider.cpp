#include "CalculiXJobMonitorProvider.hpp"
#include "CalculiXJobsInterface.hpp"

bool CalculiXJobMonitorProvider::snapshot(std::vector<JobMonitorEntry>& output, std::string& error) const
{
  return CalculiXJobsInterface().get_monitor_snapshot(output, error);
}
