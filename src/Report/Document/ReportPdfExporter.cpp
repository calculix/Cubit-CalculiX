#include "ReportPdfExporter.hpp"
#include "ReportDocumentLayout.hpp"
#include "Framework/Reporting/ReportProgress.hpp"
#include <QPdfWriter>
#include <QPainter>
#include <QSaveFile>
#include <QPageSize>
#include <QPageLayout>

ReportPdfExporter::ReportPdfExporter() = default;

bool ReportPdfExporter::encode(const ReportDocumentLayout& layout, QIODevice& output,
  std::string& error, ReportProgress* progress) const
{
  const int pages = layout.page_count();
  if (pages <= 0 || !output.isWritable() || output.pos() != 0)
  {
    error = "PDF export needs a prepared report and writable output.";
    return false;
  }
  // Destroy the writer before returning: its destructor writes the PDF trailer.
  {
    QPdfWriter writer(&output);
    writer.setResolution(72);
    writer.setTitle(QString::fromStdString(layout.report().title));
    writer.setCreator("cubit-sim Report");
    writer.setPageSize(QPageSize(layout.page_size(), QPageSize::Point));
    writer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Point);
    QPainter painter;
    if (!painter.begin(&writer))
    {
      error = "Cannot initialize the PDF painter.";
      return false;
    }
    for (int page = 0; page < pages; ++page)
    {
      const auto stage = "Writing PDF page " + std::to_string(page + 1) + " of " + std::to_string(pages);
      if (!ReportProgress::advance(progress, ReportProgressStage::ExportPdf, stage, page, pages, error))
      {
        painter.end();
        return false;
      }
      if (page > 0 && !writer.newPage())
      {
        error = "Cannot start PDF page " + std::to_string(page + 1) + ".";
        painter.end();
        return false;
      }
      if (!layout.paint_page(painter, page, error))
      {
        painter.end();
        return false;
      }
      if (!ReportProgress::advance(progress, ReportProgressStage::ExportPdf, stage, page + 1, pages, error))
      {
        painter.end();
        return false;
      }
    }
    if (!painter.end())
    {
      error = "Cannot finish PDF painting.";
      return false;
    }
  }
  if (!ReportProgress::advance(progress, ReportProgressStage::ExportPdf,
        "PDF encoding complete", pages, pages, error))
  {
    return false;
  }
  error.clear();
  return true;
}

bool ReportPdfExporter::write(const ReportDocumentLayout& layout, const std::string& filename,
  std::string& error, ReportProgress* progress) const
{
  QSaveFile output(QString::fromStdString(filename));
  output.setDirectWriteFallback(false);
  if (!output.open(QIODevice::WriteOnly))
  {
    error = "Cannot open PDF output: " + output.errorString().toStdString();
    return false;
  }
  if (!encode(layout, output, error, progress) ||
      !ReportProgress::advance(progress, ReportProgressStage::ExportCommit, "Publishing PDF", 0, 1, error))
  {
    return false;
  }
  if (!output.commit())
  {
    error = "Cannot commit PDF output: " + output.errorString().toStdString();
    return false;
  }
  if (progress)
  {
    progress->finish();
  }
  error.clear();
  return true;
}
