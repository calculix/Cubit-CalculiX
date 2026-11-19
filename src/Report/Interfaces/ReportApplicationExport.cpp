#include "ReportApplicationInterface.hpp"
#include "Report/Core/ReportSession.hpp"
#include "Report/Core/ReportUnitValidation.hpp"
#include "Report/Core/ReportLogo.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/Document/ReportPageDecoration.hpp"
#include "Report/Document/ReportPdfExporter.hpp"
#include "Report/Document/ReportOdtExporter.hpp"
#include "Report/GUI/Progress/ReportNativeProgress.hpp"
#include <QScopedValueRollback>
#include <utility>
#include <QFileInfo>
#include <QSaveFile>

bool ReportApplicationInterface::export_document(const std::string& format, const std::string& filename,
  std::string& error, bool overwrite, ReportProgress* borrowedProgress)
{
  if (generating)
  {
    error = "Wait for the current Report operation to finish.";
    return false;
  }
  if ((format != "pdf" && format != "odt" && format != "both") ||
      QString::fromStdString(filename).trimmed().isEmpty())
  {
    error = "Report export requires a valid format and a non-empty destination.";
    return false;
  }
  const auto snapshot = document();
  if (!snapshot || session().configuration_changed || session().pending_source)
  {
    error = "Generate the selected source or update its changed configuration before exporting.";
    return false;
  }
  const bool pdf = format == "pdf" || format == "both";
  const bool odt = format == "odt" || format == "both";
  std::string pdfPath = filename;
  std::string odtPath = filename;
  if (format == "both")
  {
    pdfPath += ".pdf";
    odtPath += ".odt";
  }
  std::vector<std::string> targets;
  if (pdf)
  {
    targets.push_back(pdfPath);
  }
  if (odt)
  {
    targets.push_back(odtPath);
  }
  const auto preflight = [&]()
  {
    for (const auto& target : targets)
    {
      const QFileInfo file(QString::fromStdString(target));
      if (file.isDir())
      {
        error = "Report destination is a directory: " + target;
        return false;
      }
      if (!overwrite && (file.exists() || file.isSymLink()))
      {
        error = "Report destination already exists; use overwrite to replace it: " + target;
        return false;
      }
    }
    return true;
  };
  if (!preflight())
  {
    return false;
  }
  QScopedValueRollback<bool> operation(generating, true);
  auto documentLayout = prepared_layout();
  std::vector<ReportProgressStage> stages = {ReportProgressStage::Validation};
  if (pdf && !documentLayout)
  {
    stages.insert(stages.end(), {ReportProgressStage::PreviewContent,
      ReportProgressStage::Pagination, ReportProgressStage::Contents});
  }
  if (pdf)
  {
    stages.push_back(ReportProgressStage::ExportPdf);
  }
  if (odt)
  {
    stages.push_back(ReportProgressStage::ExportOdt);
    stages.push_back(ReportProgressStage::ExportPackage);
  }
  stages.push_back(ReportProgressStage::ExportCommit);
  ReportNativeProgress native(guiAvailable, "Exporting report", std::move(stages));
  ReportProgress* progress = borrowedProgress;
  if (!progress)
  {
    progress = &native;
  }
  if (!ReportProgress::advance(progress, ReportProgressStage::Validation,
        "Validating report export", 0, 1, error) ||
      !ReportUnitValidation::document(*snapshot, error) ||
      !ReportPageDecoration::validate_templates(snapshot->configuration, error))
  {
    return false;
  }
  if (snapshot->configuration.company_logo && !ReportLogo::validate(*snapshot->configuration.company_logo, error))
  {
    return false;
  }
  std::unique_ptr<QSaveFile> pdfOutput;
  std::unique_ptr<QSaveFile> odtOutput;
  const auto stageFile = [&](const std::string& path, std::unique_ptr<QSaveFile>& output)
  {
    auto candidate = std::make_unique<QSaveFile>(QString::fromStdString(path));
    candidate->setDirectWriteFallback(false);
    if (!candidate->open(QIODevice::WriteOnly))
    {
      error = "Cannot stage report output " + path + ": " + candidate->errorString().toStdString();
      return false;
    }
    output = std::move(candidate);
    return true;
  };
  // Open both temporary outputs before encoding either format.
  if ((pdf && !stageFile(pdfPath, pdfOutput)) || (odt && !stageFile(odtPath, odtOutput)) ||
      !ReportProgress::advance(progress, ReportProgressStage::Validation,
        "Report export validated", 1, 1, error))
  {
    return false;
  }
  std::shared_ptr<ReportDocumentLayout> candidateLayout;
  if (pdf && !documentLayout)
  {
    // Public layout() rejects busy operations and would create a second bar.
    if (!prepare_layout(*snapshot, *progress, candidateLayout, error))
    {
      return false;
    }
    documentLayout = candidateLayout;
  }
  if (pdf && !ReportPdfExporter().encode(*documentLayout, *pdfOutput, error, progress))
  {
    return false;
  }
  if (odt && !ReportOdtExporter().encode(*snapshot, *odtOutput, error, progress))
  {
    return false;
  }
  if (!ReportProgress::advance(progress, ReportProgressStage::ExportCommit,
        "Publishing exported report", 0, 1, error) || !preflight())
  {
    return false;
  }
  // Publication is deliberately non-cancellable. Two filesystem renames are
  // not jointly atomic; describe exactly which output committed on failure.
  if (pdfOutput && !pdfOutput->commit())
  {
    error = "Cannot commit PDF output " + pdfPath + ": " + pdfOutput->errorString().toStdString();
    return false;
  }
  if (odtOutput && !odtOutput->commit())
  {
    error = "Cannot commit ODT output " + odtPath + ": " + odtOutput->errorString().toStdString();
    if (pdfOutput)
    {
      error = "PDF exported to " + pdfPath + "; ODT commit failed: " + error;
    }
    return false;
  }
  if (candidateLayout)
  {
    publish_layout(std::move(candidateLayout));
  }
  progress->finish();
  error.clear();
  return true;
}
