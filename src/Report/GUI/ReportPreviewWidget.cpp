#include "ReportPreviewWidget.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/Core/ReportUserOptions.hpp"
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>
#include <QResizeEvent>
#include <algorithm>
#include <cmath>
#include <utility>
#include <QScopedValueRollback>

namespace
{
const int margin = ReportUserOptions::preview_margin();
const int gap = ReportUserOptions::preview_gap();
}

ReportPreviewWidget::ReportPreviewWidget(QWidget* parent)
  : QAbstractScrollArea(parent), zoomPercent(ReportUserOptions::initial_preview_zoom()), zoomMode(ReportUserOptions::initial_preview_mode())
{
  setMinimumSize(240, 240);
  setFrameShape(QFrame::NoFrame);
  viewport()->setAutoFillBackground(false);
}

ReportPreviewWidget::~ReportPreviewWidget() = default;

void ReportPreviewWidget::set_preview_command(std::function<void(const std::string&)> callback)
{
  previewCommand = std::move(callback);
}

double ReportPreviewWidget::scale() const
{
  if (!layout)
  {
    return 1;
  }
  const auto size = layout->page_size();
  if (zoomMode == "width")
  {
    return std::max(0.05, (viewport()->width() - 2.0 * margin) / size.width());
  }
  if (zoomMode == "page")
  {
    return std::max(0.05, std::min((viewport()->width() - 2.0 * margin) / size.width(),
      (viewport()->height() - 2.0 * margin) / size.height()));
  }
  // Document geometry is in points. Qt painting/scrollbars use logical pixels;
  // devicePixelRatio is already handled by Qt and must not be multiplied again.
  return zoomPercent / 100.0 * logicalDpiX() / 72.0;
}

double ReportPreviewWidget::stride() const
{
  return layout ? layout->page_size().height() * scale() + gap : 1;
}

void ReportPreviewWidget::update_page_geometry(const QPointF& anchor)
{
  if (changingGeometry)
  {
    return;
  }
  QScopedValueRollback<bool> guard(changingGeometry, true);
  if (!layout)
  {
    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setRange(0, 0);
    return;
  }
  const double before = previousScale > 0 ? previousScale : scale();
  const auto paper = layout->page_size();
  const double oldStride = paper.height() * before + gap;
  const double oldY = verticalScrollBar()->value() + anchor.y() - margin;
  const int anchorPage = std::clamp(static_cast<int>(std::floor(std::max(0.0, oldY) / oldStride)), 0, layout->page_count() - 1);
  const double localY = (oldY - anchorPage * oldStride) / before;
  const double oldLeft = std::max<double>(margin, (viewport()->width() - paper.width() * before) / 2);
  const double localX = (horizontalScrollBar()->value() + anchor.x() - oldLeft) / before;
  // Scrollbar appearance changes viewport width. A second pass converges fit
  // modes without generating commands or rebuilding document pagination.
  for (int pass = 0; pass < 2; ++pass)
  {
    const double factor = scale();
    horizontalScrollBar()->setPageStep(viewport()->width());
    verticalScrollBar()->setPageStep(viewport()->height());
    // Ignore sub-pixel cancellation error so Fit width does not create a
    // spurious one-pixel horizontal scrollbar.
    horizontalScrollBar()->setRange(0, std::max(0, static_cast<int>(std::ceil(paper.width() * factor + 2 * margin - viewport()->width() - 0.01))));
    verticalScrollBar()->setRange(0, std::max(0, static_cast<int>(std::ceil(layout->page_count() * (paper.height() * factor + gap) - gap + 2 * margin - viewport()->height()))));
  }
  const double factor = scale();
  const double left = std::max<double>(margin, (viewport()->width() - paper.width() * factor) / 2);
  horizontalScrollBar()->setValue(static_cast<int>(std::lround(localX * factor + left - anchor.x())));
  verticalScrollBar()->setValue(static_cast<int>(std::lround(margin + anchorPage * stride() + localY * factor - anchor.y())));
  previousScale = factor;
  viewport()->update();
  if (positionChanged)
  {
    positionChanged(page(), percent());
  }
}

void ReportPreviewWidget::set_layout(std::shared_ptr<const ReportDocumentLayout> value)
{
  if (layout == value)
  {
    return;
  }
  const int visible = page();
  layout = std::move(value);
  previousScale = 0;
  update_page_geometry(QPointF(0, 0));
  if (layout)
  {
    requestedPage = -1;
    set_page(std::min(visible, layout->page_count() - 1));
  }
}

void ReportPreviewWidget::set_page(int value)
{
  if (!layout)
  {
    return;
  }
  value = std::clamp(value, 0, layout->page_count() - 1);
  if (requestedPage != value)
  {
    requestedPage = value;
    verticalScrollBar()->setValue(static_cast<int>(std::ceil(value * stride())));
  }
  viewport()->update();
}

void ReportPreviewWidget::set_zoom(double value)
{
  const int percent = std::clamp(static_cast<int>(std::lround(value * 100)), 1, 300);
  if (zoomPercent != percent || zoomMode != "percent")
  {
    zoomPercent = percent;
    zoomMode = "percent";
    update_page_geometry(viewport()->rect().center());
  }
}

void ReportPreviewWidget::set_view(int value, int percent, const std::string& mode, bool navigate, bool refit)
{
  const int visiblePage = page();
  const bool fittingPage = mode == "page" && (zoomMode != mode || refit);
  if (zoomPercent != percent || zoomMode != mode)
  {
    zoomPercent = percent;
    zoomMode = mode;
    update_page_geometry(viewport()->rect().center());
  }
  if (navigate || fittingPage)
  {
    requestedPage = -1;
    set_page(navigate ? value - 1 : visiblePage);
  }
}

int ReportPreviewWidget::page() const noexcept
{
  // Scrollbars have integer positions. The one-pixel tolerance prevents a
  // fractional page boundary from reporting the preceding page after a jump.
  return layout ? std::clamp(static_cast<int>(std::floor((verticalScrollBar()->value() + 1.0) / stride())), 0, layout->page_count() - 1) : 0;
}

int ReportPreviewWidget::percent() const noexcept
{
  return static_cast<int>(std::lround(scale() * 72.0 / logicalDpiX() * 100));
}

void ReportPreviewWidget::paintEvent(QPaintEvent*)
{
  QPainter painter(viewport());
  painter.fillRect(viewport()->rect(), palette().brush(QPalette::Mid));
  if (!layout)
  {
    painter.setPen(palette().color(QPalette::Text));
    painter.drawText(viewport()->rect(), Qt::AlignCenter, tr("Choose a source and generate a report."));
    return;
  }
  const double factor = scale();
  const double left = std::max<double>(margin, (viewport()->width() - layout->page_size().width() * factor) / 2) - horizontalScrollBar()->value();
  const int first = std::max(0, static_cast<int>((verticalScrollBar()->value() - margin) / stride()));
  const int last = std::min(layout->page_count() - 1, static_cast<int>((verticalScrollBar()->value() + viewport()->height()) / stride()) + 1);
  for (int index = first; index <= last; ++index)
  {
    painter.save();
    painter.translate(left, margin + index * stride() - verticalScrollBar()->value());
    painter.scale(factor, factor);
    std::string error;
    layout->paint_page(painter, index, error);
    painter.restore();
  }
}

void ReportPreviewWidget::resizeEvent(QResizeEvent* event)
{
  const double oldStride = layout ? layout->page_size().height() * (previousScale > 0 ? previousScale : scale()) + gap : 1;
  const int visible = layout ? std::clamp(static_cast<int>((verticalScrollBar()->value() + 1.0) / oldStride), 0, layout->page_count() - 1) : 0;
  QAbstractScrollArea::resizeEvent(event);
  update_page_geometry(QPointF(0, 0));
  if (layout && zoomMode == "page")
  {
    requestedPage = -1;
    set_page(visible);
  }
}

void ReportPreviewWidget::wheelEvent(QWheelEvent* event)
{
  if ((event->modifiers() & Qt::ControlModifier) && layout)
  {
    const int next = std::clamp(percent() + (event->angleDelta().y() > 0 ? 10 : -10), 1, 300);
    zoomMode = "percent";
    zoomPercent = next;
    update_page_geometry(event->position());
    if (previewCommand)
    {
      previewCommand("report preview zoom " + std::to_string(next));
    }
    event->accept();
    return;
  }
  QAbstractScrollArea::wheelEvent(event);
}

void ReportPreviewWidget::scrollContentsBy(int, int)
{
  viewport()->update();
  if (positionChanged)
  {
    positionChanged(page(), percent());
  }
}

void ReportPreviewWidget::set_position_changed(std::function<void(int, int)> callback)
{
  positionChanged = std::move(callback);
}
