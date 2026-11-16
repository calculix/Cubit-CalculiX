#pragma once
#include <cstddef>
#include <string>

enum class ReportProgressStage
{
  Source,
  Model,
  Results,
  Images,
  Validation,
  PreviewContent,
  Pagination,
  Contents
};

// Borrowed for one synchronous operation. Never serialized with a report.
class ReportProgress
{
public:
  ReportProgress();
  virtual ~ReportProgress();
  // Counts describe the current stage, not estimated elapsed time.
  // total == 0 means the stage has no known work count.
  virtual bool update(ReportProgressStage phase, const std::string& stage,
    std::size_t completed, std::size_t total, std::string& error) = 0;
  static bool advance(ReportProgress* progress, ReportProgressStage phase, const std::string& stage,
    std::size_t completed, std::size_t total, std::string& error);
  // Workflow-stage completion, not an estimate of elapsed/remaining time.
  // Model callbacks have local file/table counts, not an aggregate phase total.
  // 100% is reserved for publication of the document AND prepared preview.
  static double fraction(ReportProgressStage phase, std::size_t completed, std::size_t total,
    ReportProgressStage first = ReportProgressStage::Source);
};
