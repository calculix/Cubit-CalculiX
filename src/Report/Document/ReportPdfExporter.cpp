#include "ReportPdfExporter.hpp"
#include "ReportDocumentLayout.hpp"
#include <QPdfWriter>
#include <QPainter>
#include <QSaveFile>
#include <QPageSize>
#include <QPageLayout>

ReportPdfExporter::ReportPdfExporter() = default;

bool ReportPdfExporter::write(const ReportDocumentLayout& layout, const std::string& filename, std::string& error) const
{
  QSaveFile output(QString::fromStdString(filename));
  output.setDirectWriteFallback(false);
  if (layout.page_count() == 0 || !output.open(QIODevice::WriteOnly))
  {
    error = "Cannot open PDF output or no generated document is available.";
    return false;
  }
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
    for (int page = 0; page < layout.page_count(); ++page)
    {
      if ((page > 0 && !writer.newPage()) || !layout.paint_page(painter, page, error))
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
  if (!output.commit())
  {
    error = "Cannot commit the PDF output.";
    return false;
  }
  error.clear();
  return true;
}
