#pragma once
#include <cstddef>
#include <string>
#include <vector>

enum class ReportProgressStage
{
  Source,
  Model,
  Results,
  Images,
  Validation,
  PreviewContent,
  Pagination,
  Contents,
  ExportPdf,
  ExportOdt,
  ExportPackage,
  ExportCommit
};

// Borrowed for one synchronous operation. Never serialized with a report.
class ReportProgress
{
public:
  ReportProgress();
  virtual ~ReportProgress();
  // Notification after successful publication. Cannot cancel or fail.
  virtual void finish() noexcept;
  // Counts describe the current stage, not estimated elapsed time.
  // total == 0 means the stage has no known work count.
  virtual bool update(ReportProgressStage phase, const std::string& stage,
    std::size_t completed, std::size_t total, std::string& error) = 0;
  static bool advance(ReportProgress* progress, ReportProgressStage phase, const std::string& stage,
    std::size_t completed, std::size_t total, std::string& error);
  // Workflow-stage completion, not an estimate of elapsed/remaining time.
  // Model callbacks have local file/table counts, not an aggregate phase total.
  // 100% is reserved for document/preview or export-file publication.
  static double fraction(ReportProgressStage phase, std::size_t completed, std::size_t total,
    ReportProgressStage first = ReportProgressStage::Source);
  // Explicit operation order: omitted phases have no reserved percentage.
  static double fraction(ReportProgressStage phase, std::size_t completed,
    std::size_t total, const std::vector<ReportProgressStage>& order);
};
