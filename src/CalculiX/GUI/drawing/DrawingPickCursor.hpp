#ifndef DRAWINGPICKCURSOR_HPP
#define DRAWINGPICKCURSOR_HPP

#include <QObject>
#include <QPointer>
#include <QCursor>
#include <QIcon>
#include <functional>
class QWidget;

class DrawingPickCursor : public QObject
{
public:
  DrawingPickCursor(QWidget* graphicsWidget, std::function<bool()> activeQuery, std::function<void()> clearHover, QObject* parent);
  ~DrawingPickCursor() override;
  void start();
  void stop();
  void refresh();
  void setGraphicsWidget(QWidget* widget);

protected:
  bool eventFilter(QObject* object, QEvent* event) override;

private:
  void scheduleRefresh();
  void restoreCursor();
  void applyCursor(QWidget* widget);
  QPointer<QWidget> graphicsWidget;
  QPointer<QWidget> cursorWidget;
  QCursor originalCursor;
  QCursor symbolCursor;
  QIcon symbolIcon;
  std::function<bool()> activeQuery;
  std::function<void()> clearHover;
  qreal cursorRatio = 0;
  bool originalExplicit = false;
  bool started = false;
  bool changingCursor = false;
  bool refreshPending = false;
};

#endif
