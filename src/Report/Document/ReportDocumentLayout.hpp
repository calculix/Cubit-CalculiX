#pragma once
#include "Framework/Reporting/ReportDocument.hpp"
#include <QSizeF>
#include <QRectF>
#include <memory>
#include <vector>

class QPainter;
class QTextDocument;

class ReportDocumentLayout
{
public:
  ReportDocumentLayout();
  ~ReportDocumentLayout();
  bool prepare(const ReportDocument& document, std::string& error);
  int page_count() const noexcept;
  QSizeF page_size() const noexcept;
  bool paint_page(QPainter& painter, int page, std::string& error) const;
  const std::vector<std::pair<std::string, int>>& contents() const noexcept;
  const ReportDocument& report() const noexcept;

private:
  std::unique_ptr<QTextDocument> text;
  ReportDocument snapshot;
  QSizeF paper;
  QSizeF body;
  std::vector<std::pair<std::string, int>> section_pages;
};
