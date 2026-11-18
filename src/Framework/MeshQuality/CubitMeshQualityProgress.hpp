#pragma once
#include "Framework/Reporting/ReportProgress.hpp"
#include <chrono>
class ProgressTool;
// Native archive progress is independent of Report's generation scope. The
// archive has no reliable total until the mesh is discovered: use busy mode.
class CubitMeshQualityProgress final : public ReportProgress
{
public:
  CubitMeshQualityProgress();
  ~CubitMeshQualityProgress() override;
  bool update(ReportProgressStage phase, const std::string& stage,
    std::size_t completed, std::size_t total, std::string& error) override;
private:
  ProgressTool* tool = nullptr;
  bool started = false;
  std::chrono::steady_clock::time_point lastLog;
  std::string lastStage;
};
