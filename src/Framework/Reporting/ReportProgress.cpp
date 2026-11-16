#include "ReportProgress.hpp"
#include <algorithm>

ReportProgress::ReportProgress() = default;
ReportProgress::~ReportProgress() = default;

bool ReportProgress::advance(ReportProgress* progress, ReportProgressStage phase, const std::string& stage,
  std::size_t completed, std::size_t total, std::string& error)
{
  if (static_cast<int>(phase) < 0 || phase > ReportProgressStage::Contents || (total > 0 && completed > total))
  {
    error = "Invalid Report progress phase or work count.";
    return false;
  }
  if (progress == nullptr)
  {
    return true;
  }
  return progress->update(phase, stage, completed, total, error);
}

double ReportProgress::fraction(ReportProgressStage phase, std::size_t completed, std::size_t total,
  ReportProgressStage first)
{
  const int start = static_cast<int>(first);
  const int index = static_cast<int>(phase);
  const int count = static_cast<int>(ReportProgressStage::Contents) + 1;
  if (start < 0 || start >= count || index < start || index >= count)
  {
    return 0.0;
  }
  double local = 0.0;
  if (total > 0 && phase != ReportProgressStage::Model)
  {
    local = static_cast<double>((std::min)(completed, total)) / static_cast<double>(total);
  }
  return (std::min)(0.99, (static_cast<double>(index - start) + local) / static_cast<double>(count - start));
}
