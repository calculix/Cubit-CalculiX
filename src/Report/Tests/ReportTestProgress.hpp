#pragma once
#include "Framework/Reporting/ReportProgress.hpp"
#include <vector>

class ReportTestProgress final : public ReportProgress
{
public:
  ReportTestProgress();
  ~ReportTestProgress() override;
  bool update(ReportProgressStage phase, const std::string& stage, std::size_t completed, std::size_t total, std::string& error) override;
  std::vector<std::string> stages;
  std::vector<ReportProgressStage> phases;
  std::string cancelStage;
  std::size_t cancelAfter = 0;
};
