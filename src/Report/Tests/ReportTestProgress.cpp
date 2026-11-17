#include "ReportTestProgress.hpp"

ReportTestProgress::ReportTestProgress() = default;
ReportTestProgress::~ReportTestProgress() = default;

bool ReportTestProgress::update(ReportProgressStage phase, const std::string& stage, std::size_t completed, std::size_t total, std::string& error)
{
  stages.push_back(stage);
  phases.push_back(phase);
  if (onUpdate)
  {
    onUpdate();
  }
  if (total > 0 && completed > total)
  {
    error = "Invalid progress count.";
    return false;
  }
  if (!cancelStage.empty() && stage.find(cancelStage) != std::string::npos && completed >= cancelAfter)
  {
    error = "Report cancelled by regression sink.";
    return false;
  }
  return true;
}

void ReportTestProgress::finish() noexcept
{
  finished = true;
}
