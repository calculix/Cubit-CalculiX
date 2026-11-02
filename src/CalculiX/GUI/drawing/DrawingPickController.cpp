#include "DrawingPickController.hpp"
#include "DrawingPickCursor.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "GUI/application/ModelTree.hpp"
#include "CubitMessage.hpp"
#include "GUI/loads/LoadsForcesTree.hpp"
#include "GUI/loads/LoadsPressuresTree.hpp"
#include "GUI/loads/LoadsHeatfluxesTree.hpp"
#include "GUI/loads/LoadsGravityTree.hpp"
#include "GUI/loads/LoadsCentrifugalTree.hpp"
#include "GUI/loads/LoadsTrajectoryTree.hpp"
#include "GUI/loads/LoadsFilmTree.hpp"
#include "GUI/loads/LoadsRadiationTree.hpp"
#include "GUI/loads/LoadsSurfaceTractionTree.hpp"
#include "GUI/loads/LoadsBodyHeatfluxTree.hpp"
#include "GUI/bcs/BCsDisplacementsTree.hpp"
#include "GUI/bcs/BCsTemperaturesTree.hpp"
#include "GUI/sections/OrientationsTree.hpp"
#include "GUI/interactions/ConstraintsTree.hpp"
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QWidget>


DrawingPickController::DrawingPickController(ModelTree* tree, QWidget* graphicsWidget) : QObject(tree), tree(tree), graphicsWidget(graphicsWidget)
{
  cursor = new DrawingPickCursor(graphicsWidget, []() { return CalculiXDrawInterface().picking_active(); }, []() { CalculiXDrawInterface().clear_hover(); }, this);
}

DrawingPickController::~DrawingPickController()
{
  stop();
}

void DrawingPickController::start()
{
  if (started)
  {
    return;
  }
  CalculiXDrawInterface drawing;
  drawing.set_pick_callback([controller = QPointer<DrawingPickController>(this)](const CalculiXDrawSelection& selection)
  {
    if (controller != nullptr)
    {
      controller->applyPick(selection);
    }
  });
  started = true;
  if (QApplication::instance() != nullptr)
  {
    QApplication::instance()->installEventFilter(this);
  }
  cursor->start();
}

void DrawingPickController::stop()
{
  ++activationSerial;
  if (QApplication::instance() != nullptr)
  {
    QApplication::instance()->removeEventFilter(this);
  }
  cursor->stop();
  if (started)
  {
    CalculiXDrawInterface drawing;
    drawing.set_pick_callback({});
    started = false;
  }
}

void DrawingPickController::refreshCursor(QWidget* graphicsWidget)
{
  if (this->graphicsWidget != graphicsWidget)
  {
    ++activationSerial;
    this->graphicsWidget = graphicsWidget;
  }
  cursor->setGraphicsWidget(graphicsWidget);
  cursor->refresh();
}

void DrawingPickController::applyPick(const CalculiXDrawSelection& selection)
{
  if (tree == nullptr)
  {
    return;
  }
  if (!tree->selectDrawingEntity(selection))
  {
    tree->clearSelection();
    tree->updatePropertySelection();
    PRINT_WARNING("The picked CalculiX feature is no longer present in the model tree.\n");
  }
}

bool DrawingPickController::isRenderWidget(QWidget* widget) const
{
  if (widget == nullptr || graphicsWidget == nullptr || !widget->isVisible() ||
      !widget->window()->isActiveWindow() ||
      (widget != graphicsWidget && !graphicsWidget->isAncestorOf(widget)))
  {
    return false;
  }
  // Inspect Qt class names without including or linking VTK widget headers.
  return widget->inherits("QVTKOpenGLNativeWidget") || widget->inherits("QVTKOpenGLStereoWidget") || widget->inherits("QVTKWidget");
}

bool DrawingPickController::eventFilter(QObject* object, QEvent* event)
{
  if (!started || event->type() != QEvent::MouseButtonDblClick)
  {
    return false;
  }
  auto* mouse = static_cast<QMouseEvent*>(event);
  if (mouse->button() != Qt::LeftButton || mouse->buttons() != Qt::LeftButton ||
      mouse->modifiers() != Qt::NoModifier || !CalculiXDrawInterface().picking_active())
  {
    return false;
  }
  auto* widget = qobject_cast<QWidget*>(object);
  if (!isRenderWidget(widget))
  {
    return false;
  }
  return activateAt(widget, mouse->position());
}

bool DrawingPickController::activateAt(QWidget* renderWidget, const QPointF& position)
{
  if (renderWidget->width() <= 0 || renderWidget->height() <= 0)
  {
    return false;
  }
  const double xFraction = position.x() / renderWidget->width();
  const double yFraction = position.y() / renderWidget->height();
  CalculiXDrawInterface drawing;
  CalculiXDrawSelection selection;
  std::string error;
  if (!drawing.pick_symbol(-1, xFraction, yFraction, selection, error))
  {
    PRINT_ERROR("CalculiX symbol activation failed: %s\n", error.c_str());
    return false;
  }
  if (!selection.active || selection.entity.feature == DrawFeature::Topology)
  {
    return false;
  }
  drawing.clear_hover();
  const auto serial = ++activationSerial;
  const QPointer<QWidget> widget(renderWidget);
  QTimer::singleShot(0, this, [this, widget, serial, selection, xFraction, yFraction]()
  {
    if (!started || serial != activationSerial || !isRenderWidget(widget.data()) || !CalculiXDrawInterface().picking_active())
    {
      return;
    }
    // Recheck the hit after native mouse processing, before opening a panel.
    CalculiXDrawSelection current;
    std::string error;
    if (!CalculiXDrawInterface().pick_symbol(-1, xFraction, yFraction, current, error))
    {
      PRINT_ERROR("CalculiX symbol activation failed: %s\n", error.c_str());
      return;
    }
    if (!current.active || current.entity.feature != selection.entity.feature || current.entity.id != selection.entity.id || current.stepId != selection.stepId)
    {
      return;
    }
    activateFeature(current);
  });
  return true;
}

void DrawingPickController::activateFeature(const CalculiXDrawSelection& selection)
{
  if (tree == nullptr || !selection.active || !CalculiXDrawInterface().picking_active())
  {
    return;
  }
  if (!tree->selectDrawingEntity(selection))
  {
    tree->clearSelection();
    tree->updatePropertySelection();
    return;
  }
  const int id = selection.entity.id;
  switch (selection.entity.feature)
  {
    case DrawFeature::Force:
    {
      LoadsForcesTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Pressure:
    {
      LoadsPressuresTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Heatflux:
    {
      LoadsHeatfluxesTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Gravity:
    {
      LoadsGravityTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Centrifugal:
    {
      LoadsCentrifugalTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Trajectory:
    {
      LoadsTrajectoryTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Film:
    {
      LoadsFilmTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Radiation:
    {
      LoadsRadiationTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::SurfaceTraction:
    {
      LoadsSurfaceTractionTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::BodyHeatflux:
    {
      LoadsBodyHeatfluxTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Displacement:
    {
      BCsDisplacementsTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Temperature:
    {
      BCsTemperaturesTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Orientation:
    {
      OrientationsTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Equation:
    {
      ConstraintsTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::EquationGroup:
    {
      ConstraintsTree::open_modify(id, *tree);
      break;
    }
    case DrawFeature::Topology:
    {
      break;
    }
  }
}
