#include <type_traits>
#include "ReportDocumentLayout.hpp"
#include "Framework/Reporting/ReportProgress.hpp"
#include "CubitMessage.hpp"
#include <QElapsedTimer>
#include "ReportFigureCaption.hpp"
#include "ReportPageDecoration.hpp"
#include "Report/Core/ReportLogo.hpp"
#include <utility>
#include <QGuiApplication>
#include <QThread>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTextImageFormat>
#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QImage>
#include <QUrl>
#include <functional>
#include <cmath>
#include <algorithm>
#include <set>

ReportDocumentLayout::ReportDocumentLayout() = default;
ReportDocumentLayout::~ReportDocumentLayout() = default;

bool ReportDocumentLayout::prepare(const ReportDocument& document, std::string& error, ReportProgress* progress)
{
  auto* app = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
  if (app == nullptr || QThread::currentThread() != app->thread())
  {
    error = "Report layout requires the Cubit GUI application thread.";
    return false;
  }
  if (!document.validate(error))
  {
    return false;
  }
  QElapsedTimer timer;
  timer.start();
  std::size_t totalCells = 0, totalImages = 0, totalRows = 0, totalTables = 0;
  std::function<void(const ReportSection&)> count = [&](const ReportSection& section)
  {
    for (const auto& table : section.tables)
    {
      totalCells += (table.rows.size() + 1) * table.columns.size();
      totalRows += table.rows.size();
      ++totalTables;
    }
    totalImages += section.images.size();
    for (const auto& child : section.children)
    {
      count(child);
    }
  };
  for (const auto& section : document.sections)
  {
    count(section);
  }
  PRINT_INFO("Report preview: %llu tables, %llu rows, %llu cells, %llu images.\n",
    static_cast<unsigned long long>(totalTables), static_cast<unsigned long long>(totalRows),
    static_cast<unsigned long long>(totalCells), static_cast<unsigned long long>(totalImages));
  std::size_t completedCells = 0, completedImages = 0;
  auto candidate = std::make_unique<QTextDocument>();
  candidate->setUndoRedoEnabled(false);
  // Suppress repeated pagination while constructing tables. The temporary
  // document is discarded atomically on failure or cancellation.
  candidate->setLayoutEnabled(false);
  const auto& settings = document.configuration.values;
  auto setting = [&](const std::string& key, const std::string& fallback)
  {
    const auto value = settings.find(key);
    if (value == settings.end()) {
      return fallback;
    } else {
      return value->second;
    }
  };
  const QSizeF page = [&]() -> QSizeF {
    if (setting("page_size", "A4") == "Letter") {
      return QSizeF(612, 792);
    } else {
      return QSizeF(595.28, 841.89);
    }
  }();
  if (!ReportPageDecoration::validate_templates(document.configuration, error) ||
      (document.configuration.company_logo && !ReportLogo::validate(*document.configuration.company_logo, error)))
  {
    return false;
  }
  const QSizeF content = ReportPageDecoration::body_rect(document, page).size();
  candidate->setDefaultFont(QFont("Arial", 9));
  candidate->setDocumentMargin(0);
  candidate->setPageSize(content);
  QTextOption option;
  option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  candidate->setDefaultTextOption(option);
  const int precision = QString::fromStdString(setting("precision", "5")).toInt();
  QTextCursor cursor(candidate.get());
  cursor.beginEditBlock();
  auto paragraph = [&](const QString& value, int heading, bool pageBreak)
  {
    if (cursor.position() != 0)
    {
      cursor.insertBlock();
    }
    QTextBlockFormat block;
    block.setTopMargin([&]() -> int {
      if (heading) {
        return 12;
      } else {
        return 3;
      }
    }());
    block.setBottomMargin([&]() -> int {
      if (heading) {
        return 7;
      } else {
        return 5;
      }
    }());
    block.setHeadingLevel(heading);
    block.setNonBreakableLines(false);
    if (pageBreak)
    {
      block.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
    }
    cursor.setBlockFormat(block);
    QTextCharFormat format;
    format.setFontPointSize([&]() -> std::common_type_t<decltype(20), int> {
      if (heading == 1) {
        return 20;
      } else {
        return [&]() -> int {
          if (heading == 2) {
            return 13;
          } else {
            return 9;
          }
        }();
      }
    }());
    format.setFontWeight([&]() -> std::common_type_t<decltype(QFont::Bold), decltype(QFont::Normal)> {
      if (heading) {
        return QFont::Bold;
      } else {
        return QFont::Normal;
      }
    }());
    format.setForeground([&]() -> std::common_type_t<decltype(QColor("#18334c")), decltype(QColor("#253442"))> {
      if (heading) {
        return QColor("#18334c");
      } else {
        return QColor("#253442");
      }
    }());
    cursor.setCharFormat(format);
    cursor.insertText(value);
  };
  if (setting("cover", "true") == "true")
  {
    paragraph("ENGINEERING REPORT", 2, false);
    paragraph(QString::fromStdString(document.title), 1, false);
    paragraph(QString::fromStdString(setting("project", "")), 0, false);
    paragraph("Author: " + QString::fromStdString(setting("author", "")), 0, false);
    paragraph("Revision: " + QString::fromStdString(setting("revision", "")), 0, false);
    paragraph("Generated: " + QString::fromStdString(document.generated_at), 0, false);
    paragraph("Source: " + QString::fromStdString(document.source.workspace_id) + ([&]() -> QString {
      if (document.source.kind == "model") {
        return " / current model";
      } else {
        return " / job " + QString::number(document.source.job_id);
      }
    }()), 0, false);
    paragraph(QString::fromStdString(document.source.provenance), 0, false);
  }
  // Insert real headings first, then resolve physical pages and replace
  // fixed-width, monospaced TOC page fields without changing their geometry.
  std::vector<std::pair<std::string, int>> headings;
  std::vector<std::pair<int, std::string>> toc;
  std::set<std::string> renderedNotes;
  if (setting("contents", "true") == "true")
  {
    paragraph("Contents", 1, cursor.position() != 0);
    for (const auto& section : document.sections)
    {
      paragraph(QString::fromStdString(section.title) + "  ", 0, false);
      QTextCharFormat digits = cursor.charFormat();
      digits.setFontFamily("Courier New");
      cursor.setCharFormat(digits);
      const int field = cursor.position();
      cursor.insertText("0000");
      toc.push_back({field, section.id});
    }
  }
  std::function<bool(const ReportSection&, int)> appendSection = [&](const ReportSection& section, int level)
  {
    paragraph(QString::fromStdString(section.title), [&]() -> int {
      if (level == 0) {
        return 1;
      } else {
        return 2;
      }
    }(), level == 0 && cursor.position() != 0);
    headings.push_back({section.id, cursor.block().position()});
    for (const auto& line : section.paragraphs)
    {
      paragraph(QString::fromStdString(line), 0, false);
    }
    for (const auto& sourceTable : section.tables)
    {
      if (!ReportProgress::advance(progress, ReportProgressStage::PreviewContent, "Building preview tables", completedCells + completedImages, totalCells + totalImages, error))
      {
        return false;
      }
      paragraph(QString::fromStdString(sourceTable.title), 2, false);
      QTextTableFormat format;
      format.setHeaderRowCount(1);
      format.setBorder(0.3);
      format.setBorderBrush(QColor("#c9d6df"));
      format.setCellPadding(4);
      format.setCellSpacing(0);
      format.setWidth(QTextLength(QTextLength::PercentageLength, 100));
      // Append rows in bounded batches instead of allocating one huge table.
      auto* table = cursor.insertTable(1, static_cast<int>(sourceTable.columns.size()), format);
      for (std::size_t col = 0; col < sourceTable.columns.size(); ++col)
      {
        auto cell = table->cellAt(0, static_cast<int>(col));
        auto cellFormat = cell.format();
        cellFormat.setBackground(QColor("#edf3f7"));
        cell.setFormat(cellFormat);
        QString title = QString::fromStdString(sourceTable.columns[col]);
        if (col < sourceTable.units.size() && !sourceTable.units[col].empty())
        {
          title += " [" + QString::fromStdString(sourceTable.units[col]) + "]";
        }
        cell.firstCursorPosition().insertText(title);
        ++completedCells;
      }
      for (std::size_t row = 0; row < sourceTable.rows.size(); ++row)
      {
        if (row % 128 == 0)
        {
          const auto batch = (std::min)(std::size_t(128), sourceTable.rows.size() - row);
          table->insertRows(table->rows(), static_cast<int>(batch));
        }
        for (std::size_t col = 0; col < sourceTable.columns.size(); ++col)
        {
          const auto& value = sourceTable.rows[row][col];
          QString display;
          if (value.isDouble())
          {
            display = QString::number(value.toDouble(), 'g', precision);
          }
          else
          {
            display = value.toString();
          }
          table->cellAt(static_cast<int>(row) + 1, static_cast<int>(col)).firstCursorPosition().insertText(display);
          ++completedCells;
          if (completedCells % 256 == 0 &&
              !ReportProgress::advance(progress, ReportProgressStage::PreviewContent, "Building preview tables", completedCells + completedImages, totalCells + totalImages, error))
          {
            return false;
          }
        }
      }
      cursor = table->lastCursorPosition();
      cursor.movePosition(QTextCursor::NextBlock);
    }
    for (const auto& image : section.images)
    {
      if (!ReportProgress::advance(progress, ReportProgressStage::PreviewContent, "Adding preview images", completedCells + completedImages, totalCells + totalImages, error))
      {
        return false;
      }
      const QImage pixels = QImage::fromData(QByteArray::fromBase64(QByteArray::fromStdString(image.base64_data)));
      const QUrl url("report-image:" + QString::fromStdString(image.id));
      candidate->addResource(QTextDocument::ImageResource, url, pixels);
      paragraph("", 0, false);
      QTextImageFormat format;
      format.setName(url.toString());
      const double scale = std::min(content.width() / std::max(1, pixels.width()), 300.0 / std::max(1, pixels.height()));
      format.setWidth(pixels.width() * scale);
      format.setHeight(pixels.height() * scale);
      cursor.insertImage(format);
      paragraph(ReportFigureCaption::text(image, document.configuration), 0, false);
      if (!ReportProgress::advance(progress, ReportProgressStage::PreviewContent, "Adding preview images", completedCells + ++completedImages, totalCells + totalImages, error))
      {
        return false;
      }
    }
    const auto note = document.notes.find(section.id);
    if (note != document.notes.end() && !note->second.empty())
    {
      renderedNotes.insert(note->first);
      paragraph("Notes", 2, false);
      paragraph(QString::fromStdString(note->second), 0, false);
    }
    for (const auto& child : section.children)
    {
      if (!appendSection(child, level + 1))
      {
        return false;
      }
    }
    return true;
  };
  for (const auto& section : document.sections)
  {
    if (!appendSection(section, 0))
    {
      return false;
    }
  }
  for (const auto& note : document.notes)
  {
    if (renderedNotes.count(note.first) == 0 && !note.second.empty())
    {
      paragraph("Retained note / " + QString::fromStdString(note.first), 2, false);
      paragraph(QString::fromStdString(note.second), 0, false);
    }
  }
  paragraph("Sources and diagnostics", 1, cursor.position() != 0);
  headings.push_back({"report.diagnostics", cursor.block().position()});
  paragraph(QString::fromStdString(document.source.provenance), 0, false);
  if (!document.source.snapshot_fingerprint.empty())
  {
    paragraph("Reporting snapshot SHA-256: " + QString::fromStdString(document.source.snapshot_fingerprint), 0, false);
  }
  for (std::size_t i = 0; i < document.source.input_files.size(); ++i)
  {
    paragraph(QString::fromStdString(document.source.input_files[i]), 0, false);
    paragraph("SHA-256: " + QString::fromStdString(document.source.fingerprints[i]), 0, false);
  }
  for (const auto& issue : document.diagnostics)
  {
    paragraph(QString::fromStdString(issue.severity + " / " + issue.section_id + ": " + issue.message), 0, false);
  }
  cursor.endEditBlock();
  PRINT_INFO("Report preview content constructed in %lld ms.\n", static_cast<long long>(timer.elapsed()));
  std::vector<std::pair<std::string, int>> pages;
  if (!ReportProgress::advance(progress, ReportProgressStage::Pagination, "Paginating report preview", 0, 1, error))
  {
    return false;
  }
  timer.restart();
  // Qt pagination is synchronous; progress cannot claim page-by-page completion.
  candidate->setLayoutEnabled(true);
  candidate->documentLayout()->documentSize();
  PRINT_INFO("Report preview pagination: %d pages in %lld ms.\n", candidate->pageCount(), static_cast<long long>(timer.elapsed()));
  if (!ReportProgress::advance(progress, ReportProgressStage::Pagination, "Paginating report preview", 1, 1, error))
  {
    return false;
  }
  if (candidate->pageCount() > 9999)
  {
    error = "Report exceeds the supported preview size (9999 pages). Narrow the selected content.";
    return false;
  }
  for (const auto& heading : headings)
  {
    const auto block = candidate->findBlock(heading.second);
    const auto bounds = candidate->documentLayout()->blockBoundingRect(block);
    pages.push_back({heading.first, static_cast<int>(std::floor(bounds.top() / content.height()))});
  }
  // Four-digit fields have stable character counts. Use monospaced digits so
  // replacing 0000 cannot change line wrapping or invalidate heading positions.
  if (!ReportProgress::advance(progress, ReportProgressStage::Contents, "Updating report contents", 0, toc.size(), error))
  {
    return false;
  }
  candidate->setLayoutEnabled(false);
  cursor.beginEditBlock();
  for (auto& item : toc)
  {
    const auto found = std::find_if(pages.begin(), pages.end(), [&](const auto& value)
    {
      return value.first == item.second;
    });
    if (found != pages.end())
    {
      // Subsequent construction appends text; equal-width replacements preserve offsets.
      QTextCursor field(candidate.get());
      field.setPosition(item.first);
      field.setPosition(item.first + 4, QTextCursor::KeepAnchor);
      field.insertText(QString::number(found->second + 1).rightJustified(4, '0'));
    }
  }
  // Content edits are complete before the prepared layout becomes visible.
  cursor.endEditBlock();
  candidate->setLayoutEnabled(true);
  if (!ReportProgress::advance(progress, ReportProgressStage::Contents, "Updating report contents", toc.size(), toc.size(), error))
  {
    return false;
  }
  text = std::move(candidate);
  paper = page;
  body = content;
  snapshot = document;
  section_pages = std::move(pages);
  error.clear();
  return true;
}

int ReportDocumentLayout::page_count() const noexcept
{
  if (text == nullptr) {
    return 0;
  } else {
    return text->pageCount();
  }
}

QSizeF ReportDocumentLayout::page_size() const noexcept
{
  return paper;
}

bool ReportDocumentLayout::paint_page(QPainter& painter, int page, std::string& error) const
{
  if (text == nullptr || page < 0 || page >= text->pageCount())
  {
    error = "Report preview page is unavailable.";
    return false;
  }
  painter.save();
  painter.fillRect(QRectF(QPointF(), paper), Qt::white);
  ReportPageDecoration::paint(painter, snapshot, paper, page + 1, text->pageCount());
  painter.translate(ReportPageDecoration::body_rect(snapshot, paper).topLeft());
  painter.setClipRect(QRectF(0, 0, body.width(), body.height()));
  painter.translate(0, -page * body.height());
  QAbstractTextDocumentLayout::PaintContext context;
  context.clip = QRectF(0, page * body.height(), body.width(), body.height());
  text->documentLayout()->draw(&painter, context);
  painter.restore();
  error.clear();
  return true;
}

const std::vector<std::pair<std::string, int>>& ReportDocumentLayout::contents() const noexcept
{
  return section_pages;
}

const ReportDocument& ReportDocumentLayout::report() const noexcept
{
  return snapshot;
}
