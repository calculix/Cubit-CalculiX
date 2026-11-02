#ifndef DRAWINGPICKCONTROLLER_HPP
#define DRAWINGPICKCONTROLLER_HPP

#include "Core/drawing/CoreDrawTypes.hpp"
#include <QObject>
#include <QPointer>
#include <QPointF>
#include <cstddef>
class ModelTree;
class QWidget;
class DrawingPickCursor;

class DrawingPickController : public QObject
{
public:
  DrawingPickController(ModelTree* tree, QWidget* graphicsWidget);
  ~DrawingPickController() override;
  void start();
  void stop();
  void refreshCursor(QWidget* graphicsWidget);

protected:
  bool eventFilter(QObject* object, QEvent* event) override;

private:
  bool isRenderWidget(QWidget* widget) const;
  bool activateAt(QWidget* renderWidget, const QPointF& position);
  void activateFeature(const CalculiXDrawSelection& selection);
  void applyPick(const CalculiXDrawSelection& selection);
  QPointer<ModelTree> tree;
  QPointer<QWidget> graphicsWidget;
  std::size_t activationSerial = 0;
  DrawingPickCursor* cursor = nullptr;
  bool started = false;
};

#endif
