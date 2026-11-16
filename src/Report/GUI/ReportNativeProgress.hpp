#pragma once
#include "Framework/Reporting/ReportProgress.hpp"
#include <chrono>
class ProgressTool;

class ReportNativeProgress final : public ReportProgress
{
public:
  explicit ReportNativeProgress(bool guiAvailable, ReportProgressStage first = ReportProgressStage::Source);
  void finish() noexcept;
  ~ReportNativeProgress() override;
  bool update(ReportProgressStage phase, const std::string& stage, std::size_t completed, std::size_t total, std::string& error) override;

private:
  void close() noexcept;
  ReportProgressStage firstStage;
  double overall = 0.0;
  ProgressTool* tool = nullptr;
  bool started = false;
  bool interrupted = false;
  std::string currentStage;
  std::string information;
  std::chrono::steady_clock::time_point lastUpdate;
};
