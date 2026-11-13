#include "PostprocessorPlotChart.hpp"
#include "PostprocessorPlotRendering.hpp"
#include <utility>
#include "PostprocessorPlotInteraction.hpp"
#include <vtkAutoInit.h>
#include <QVTKInteractor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkRenderer.h>
#include <vtkChartXY.h>
#include <vtkAxis.h>
#include <vtkPlot.h>
#include <vtkPlotPoints.h>
#include <vtkTable.h>
#include <vtkDoubleArray.h>
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPen>
#include <QLineF>
#include <QTimer>
#include <QShowEvent>
#include <QScopedValueRollback>
#include <cmath>
#include <algorithm>
#include <limits>
VTK_MODULE_INIT(vtkRenderingContextOpenGL2);
namespace
{
  class PlotCursorOverlay : public QWidget
  {
  public:
    PlotCursorOverlay(std::function<std::optional<QLineF>()> line,QWidget* parent)
      : QWidget(parent),current(std::move(line))
    {
      setAttribute(Qt::WA_TransparentForMouseEvents);
      setAttribute(Qt::WA_NoSystemBackground);
    }
  protected:
    void paintEvent(QPaintEvent*) override
    {
      const auto line = current();
      if (line)
      {
        QPainter painter(this);
        const auto& rgb = Postprocessor::PlotDefaults::cursor_color;
        painter.setPen(QPen(QColor(rgb[0],rgb[1],rgb[2]),1,Qt::DashLine));
        painter.drawLine(*line);
      }
    }
  private:
    std::function<std::optional<QLineF>()> current;
  };
}

PostprocessorPlotChart::PostprocessorPlotChart(std::function<bool(const Postprocessor::PlotAxes&,bool)> send,
  std::function<std::uint64_t()> revision,QWidget* parent)
  : QVTKOpenGLNativeWidget(static_cast<vtkGenericOpenGLRenderWindow*>(nullptr),parent)
{
  setObjectName("Postprocessor.PlotChart");
  setFocusPolicy(Qt::StrongFocus);
  auto format = QVTKOpenGLNativeWidget::defaultFormat();
  format.setSwapInterval(1);
  setFormat(format);
  window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  setRenderWindow(window);
  context = vtkSmartPointer<vtkContextView>::New();
  context->SetRenderWindow(window);
  context->SetInteractor(interactor());
  chart = vtkSmartPointer<vtkChartXY>::New();
  chart->SetShowLegend(Postprocessor::PlotDefaults::legend_visible);
  chart->GetAxis(vtkAxis::BOTTOM)->SetGridVisible(Postprocessor::PlotDefaults::grid_visible);
  chart->GetAxis(vtkAxis::LEFT)->SetGridVisible(Postprocessor::PlotDefaults::grid_visible);
  const auto& background = Postprocessor::PlotDefaults::background;
  context->GetRenderer()->SetBackground(background[0],background[1],background[2]);
  chart->SetIgnoreNanInBounds(true);
  context->GetScene()->AddItem(chart);
  cursor = new PlotCursorOverlay([this]() -> std::optional<QLineF>
  {
    return plot_cursor_line(chart,markerX,height(),devicePixelRatioF());
  },this);
  interaction = std::make_unique<PostprocessorPlotInteraction>([this]()
  {
    return current_axes();
  },[this]()
  {
    return axes;
  },[this]()
  {
    auto* x = chart->GetAxis(vtkAxis::BOTTOM);
    auto* y = chart->GetAxis(vtkAxis::LEFT);
    return QSizeF(std::abs(x->GetPoint2()[0]-x->GetPoint1()[0])/devicePixelRatioF(),
      std::abs(y->GetPoint2()[1]-y->GetPoint1()[1])/devicePixelRatioF());
  },std::move(revision),[this,send](const Postprocessor::PlotAxes& value,bool journal)
  {
    if (!send(value,journal))
    {
      return false;
    }
    apply_axes(value);
    return true;
  },[this]()
  {
    request_frame();
  },this);
  connect(this,&QOpenGLWidget::frameSwapped,this,[this]()
  {
    presenting = false;
    if (dirty || interaction->pending())
    {
      request_frame();
    }
  });
}
PostprocessorPlotChart::~PostprocessorPlotChart() = default;
void PostprocessorPlotChart::cancel()
{
  interaction->cancel();
}
void PostprocessorPlotChart::set_data(const Postprocessor::PlotDefinition& definition,const Postprocessor::PlotData& data)
{
  populate_plot_chart(chart,definition,data);
  apply_axes(definition.axes);
  request_frame();
}
void PostprocessorPlotChart::apply_axes(const Postprocessor::PlotAxes& value)
{
  axes = value;
  for (int index = 0; index < 2; ++index)
  {
    auto* axis = chart->GetAxis(index == 0 ? vtkAxis::BOTTOM : vtkAxis::LEFT);
    const auto& range = index == 0 ? axes.x : axes.y;
    axis->SetBehavior(range ? vtkAxis::FIXED : vtkAxis::AUTO);
    if (range)
    {
      axis->SetRange((*range)[0],(*range)[1]);
    }
  }
  chart->RecalculateBounds();
  request_frame();
}
Postprocessor::PlotAxes PostprocessorPlotChart::current_axes() const
{
  Postprocessor::PlotAxes result;
  std::array<double,2> x{}, y{};
  chart->GetAxis(vtkAxis::BOTTOM)->GetRange(x.data());
  chart->GetAxis(vtkAxis::LEFT)->GetRange(y.data());
  result.x = x;
  result.y = y;
  if (!Postprocessor::valid_plot_axes(result))
  {
    result.x = std::array<double,2>{0,1};
    result.y = std::array<double,2>{0,1};
  }
  return result;
}
void PostprocessorPlotChart::set_marker(std::optional<double> value)
{
  if (markerX != value)
  {
    markerX = value;
    request_frame();
  }
}
void PostprocessorPlotChart::request_frame()
{
  dirty = true;
  if (queued || presenting || rendering)
  {
    return;
  }
  queued = true;
  QTimer::singleShot(0,this,[this]()
  {
    queued = false;
    render_frame();
  });
}
void PostprocessorPlotChart::render_frame()
{
  if (!dirty || !isVisible() || !isValid() || presenting || rendering)
  {
    return;
  }
  QScopedValueRollback<bool> guard(rendering,true);
  interaction->flush_preview();
  dirty = false;
  presenting = true;
  window->Render();
  cursor->setGeometry(rect());
  cursor->show();
  cursor->raise();
  cursor->update();
  update();
}
void PostprocessorPlotChart::initializeGL()
{
  QVTKOpenGLNativeWidget::initializeGL();
  presenting = false;
  request_frame();
}
void PostprocessorPlotChart::resizeGL(int width,int height)
{
  QVTKOpenGLNativeWidget::resizeGL(width,height);
  presenting = false;
  request_frame();
}
bool PostprocessorPlotChart::event(QEvent* event)
{
  if (event->type() == QEvent::ShortcutOverride)
  {
    const auto* key = static_cast<QKeyEvent*>(event);
    const bool graphicsFunction = key->key() >= Qt::Key_F1 && key->key() <= Qt::Key_F12;
    const bool graphicsControl = key->modifiers().testFlag(Qt::ControlModifier) &&
      (key->key() == Qt::Key_R || key->key() == Qt::Key_G || key->key() == Qt::Key_D ||
        key->key() == Qt::Key_Z || key->key() == Qt::Key_V || key->key() == Qt::Key_H);
    if (graphicsFunction || graphicsControl)
    {
      event->accept();
      return true;
    }
  }
  if (interaction && interaction->handle(event))
  {
    event->accept();
    return true;
  }
  switch (event->type())
  {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::Wheel:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::NativeGesture:
    case QEvent::Gesture:
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::ContextMenu:
      event->accept();
      return true;
    default:
      return QVTKOpenGLNativeWidget::event(event);
  }
}
void PostprocessorPlotChart::showEvent(QShowEvent* event)
{
  QVTKOpenGLNativeWidget::showEvent(event);
  presenting = false;
  request_frame();
}
