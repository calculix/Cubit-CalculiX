#include "DrawingPickCursor.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include <QApplication>
#include <QEvent>
#include <QScopedValueRollback>
#include <QTimer>
#include <QWidget>
#include <QPixmap>
#include <utility>

DrawingPickCursor::DrawingPickCursor(QWidget* graphicsWidget, std::function<bool()> activeQuery, std::function<void()> clearHover, QObject* parent)
  : QObject(parent), graphicsWidget(graphicsWidget), activeQuery(std::move(activeQuery)), clearHover(std::move(clearHover))
{
  CalculiXGuiResourcesInterface resources;
  symbolIcon = resources.getIcon2("CCXPickSymbols");
}

DrawingPickCursor::~DrawingPickCursor()
{
  stop();
}

void DrawingPickCursor::start()
{
  if (!started && QApplication::instance() != nullptr)
  {
    QApplication::instance()->installEventFilter(this);
    started = true;
    refresh();
  }
}

void DrawingPickCursor::stop()
{
  if (started && clearHover)
  {
    clearHover();
  }
  started = false;
  if (QApplication::instance() != nullptr)
  {
    QApplication::instance()->removeEventFilter(this);
  }
  restoreCursor();
}

void DrawingPickCursor::setGraphicsWidget(QWidget* widget)
{
  if (graphicsWidget != widget)
  {
    if (started && clearHover)
    {
      clearHover();
    }
    restoreCursor();
    graphicsWidget = widget;
  }
}

void DrawingPickCursor::restoreCursor()
{
  const QScopedValueRollback<bool> guard(changingCursor, true);
  if (cursorWidget != nullptr)
  {
    if (originalExplicit)
    {
      cursorWidget->setCursor(originalCursor);
    }
    else
    {
      cursorWidget->unsetCursor();
    }
  }
  cursorWidget = nullptr;
}

void DrawingPickCursor::applyCursor(QWidget* widget)
{
  const qreal ratio = widget->devicePixelRatioF();
  if (cursorWidget == widget && cursorRatio == ratio)
  {
    if (widget->cursor().shape() == Qt::BitmapCursor && widget->cursor().pixmap().cacheKey() == symbolCursor.pixmap().cacheKey())
    {
      return;
    }
  }
  if (cursorWidget != widget)
  {
    restoreCursor();
    originalExplicit = widget->testAttribute(Qt::WA_SetCursor);
    originalCursor = widget->cursor();
  }
  if (cursorRatio != ratio)
  {
    const QPixmap image = symbolIcon.pixmap(QSize(24, 24), ratio);
    if (image.isNull())
    {
      return;
    }
    // Keep the arrow at its authored size; Qt supplies the physical pixels for HiDPI.
    symbolCursor = QCursor(image, 3, 2);
    cursorRatio = ratio;
  }
  cursorWidget = widget;
  const QScopedValueRollback<bool> guard(changingCursor, true);
  widget->setCursor(symbolCursor);
}

void DrawingPickCursor::refresh()
{
  if (!started || graphicsWidget == nullptr || !graphicsWidget->isVisible() || !graphicsWidget->window()->isActiveWindow() || !activeQuery || !activeQuery())
  {
    if (started && clearHover)
    {
      clearHover();
    }
    restoreCursor();
    return;
  }
  auto* target = QApplication::widgetAt(QCursor::pos());
  if (target == nullptr || (target != graphicsWidget && !graphicsWidget->isAncestorOf(target)))
  {
    if (clearHover)
    {
      clearHover();
    }
    restoreCursor();
    return;
  }
  // Preserve Cubit's temporary drag/navigation cursors; resume the badge after release.
  if (QApplication::mouseButtons() == Qt::NoButton)
  {
    applyCursor(target);
  }
}

void DrawingPickCursor::scheduleRefresh()
{
  if (!started || refreshPending)
  {
    return;
  }
  refreshPending = true;
  QTimer::singleShot(0, this, [this]()
  {
    refreshPending = false;
    refresh();
  });
}

bool DrawingPickCursor::eventFilter(QObject* object, QEvent* event)
{
  if (!started || changingCursor)
  {
    return false;
  }
  if (event->type() == QEvent::CursorChange && object == cursorWidget)
  {
    // Remember native cursor updates so leaving this mode restores the latest cursor.
    if (cursorWidget != nullptr)
    {
      originalExplicit = cursorWidget->testAttribute(Qt::WA_SetCursor);
      originalCursor = cursorWidget->cursor();
    }
    scheduleRefresh();
  }
  else if (event->type() == QEvent::MouseButtonPress)
  {
    if (clearHover)
    {
      clearHover();
    }
  }
  else if (event->type() == QEvent::MouseMove)
  {
    refresh();
  }
  else if (event->type() == QEvent::Enter || event->type() == QEvent::Leave || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::KeyRelease || event->type() == QEvent::WindowActivate || event->type() == QEvent::WindowDeactivate || event->type() == QEvent::Hide || event->type() == QEvent::ScreenChangeInternal)
  {
    scheduleRefresh();
  }
  return false;
}
