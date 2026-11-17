#pragma once
#include "Framework/Reporting/ReportDocument.hpp"
#include <QSizeF>
#include <QRectF>
#include <memory>
#include <vector>

class ReportProgress;
class ReportLayoutBatch;
class QPainter;
class QTextDocument;

class ReportDocumentLayout
{
public:
  ReportDocumentLayout();
  ~ReportDocumentLayout();
  bool prepare(const ReportDocument& document, std::string& error, ReportProgress* progress = nullptr);
  int page_count() const noexcept;
  QSizeF page_size() const noexcept;
  bool paint_page(QPainter& painter, int page, std::string& error) const;
  const std::vector<std::pair<std::string, int>>& contents() const noexcept;
  const ReportDocument& report() const noexcept;

private:
  friend class ReportLayoutBatch;
  bool prepare_single(const ReportDocument& document, std::string& error,
    ReportProgress* progress, bool contentsOnly);
  bool resolve_contents(const std::vector<std::pair<std::string, int>>& pages, std::string& error);
  std::vector<std::unique_ptr<ReportLayoutBatch>> batches;
  std::vector<std::pair<int, std::string>> toc_fields;
  int total_pages = 0;
  std::unique_ptr<QTextDocument> text;
  ReportDocument snapshot;
  QSizeF paper;
  QSizeF body;
  std::vector<std::pair<std::string, int>> section_pages;
};
