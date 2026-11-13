#include "PostprocessorPlotInteraction.hpp"
#include <utility>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <algorithm>
#include <cmath>
PostprocessorPlotInteraction::PostprocessorPlotInteraction(std::function<Postprocessor::PlotAxes()> current,
  std::function<Postprocessor::PlotAxes()> definition,std::function<QSizeF()> area,
  std::function<std::uint64_t()> token,std::function<bool(const Postprocessor::PlotAxes&,bool)> dispatch,
  std::function<void()> update,QObject* parent)
  : QObject(parent),currentAxes(std::move(current)),definitionAxes(std::move(definition)),plotArea(std::move(area)),
    currentRevision(std::move(token)),send(std::move(dispatch)),redraw(std::move(update))
{
  wheelTimer.setSingleShot(true);
  wheelTimer.setInterval(Postprocessor::PlotDefaults::wheel_commit_ms);
  connect(&wheelTimer,&QTimer::timeout,this,[this]()
  {
    finish();
  });
}
void PostprocessorPlotInteraction::begin()
{
  baseline = currentAxes();
  baselineState = definitionAxes();
  target = baseline;
  revision = currentRevision();
  active = true;
  dirty = changed = previewed = false;
}
bool PostprocessorPlotInteraction::dispatch(const Postprocessor::PlotAxes& axes,bool journal)
{
  if (!active || currentRevision() != revision || !Postprocessor::valid_plot_axes(axes))
  {
    return false;
  }
  const bool accepted = send(axes,journal);
  if (accepted)
  {
    revision = currentRevision();
    previewed = previewed || !journal;
  }
  return accepted;
}
void PostprocessorPlotInteraction::cancel()
{
  wheelTimer.stop();
  dirty = false;
  if (active && previewed && currentRevision() == revision)
  {
    dispatch(baselineState,false);
  }
  active = changed = previewed = wheel = false;
}
void PostprocessorPlotInteraction::finish()
{
  wheelTimer.stop();
  dirty = false;
  if (active && changed && !dispatch(target,true))
  {
    cancel();
  }
  active = changed = previewed = wheel = false;
}
bool PostprocessorPlotInteraction::pending() const
{
  return dirty;
}
void PostprocessorPlotInteraction::flush_preview()
{
  if (dirty)
  {
    dirty = false;
    if (!dispatch(target,false))
    {
      cancel();
    }
  }
}
bool PostprocessorPlotInteraction::handle(QEvent* event)
{
  if (active && currentRevision() != revision)
  {
    cancel();
  }
  if (event->type() == QEvent::FocusOut || event->type() == QEvent::Hide || event->type() == QEvent::WindowDeactivate)
  {
    cancel();
    return false;
  }
  if (event->type() == QEvent::ShortcutOverride)
  {
    const auto key = static_cast<QKeyEvent*>(event)->key();
    return (key == Qt::Key_F && static_cast<QKeyEvent*>(event)->modifiers() == Qt::NoModifier) || key == Qt::Key_Escape;
  }
  if (event->type() == QEvent::KeyPress)
  {
    const auto key = static_cast<QKeyEvent*>(event)->key();
    if (key == Qt::Key_Escape)
    {
      cancel();
      return true;
    }
    if (key == Qt::Key_F && static_cast<QKeyEvent*>(event)->modifiers() == Qt::NoModifier && !static_cast<QKeyEvent*>(event)->isAutoRepeat())
    {
      cancel();
      send(Postprocessor::PlotAxes{},true);
      return true;
    }
  }
  if (event->type() == QEvent::MouseButtonPress && static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
  {
    cancel();
    begin();
    origin = static_cast<QMouseEvent*>(event)->position();
    return true;
  }
  if (event->type() == QEvent::MouseMove && active && !wheel)
  {
    const auto* mouse = static_cast<QMouseEvent*>(event);
    if (!mouse->buttons().testFlag(Qt::LeftButton))
    {
      cancel();
      return true;
    }
    if (!baseline.x || !baseline.y)
    {
      return true;
    }
    const auto delta = mouse->position()-origin;
    const auto size = plotArea();
    const double dx = -delta.x()/std::max(1.0,size.width())*((*baseline.x)[1]-(*baseline.x)[0]);
    const double dy = delta.y()/std::max(1.0,size.height())*((*baseline.y)[1]-(*baseline.y)[0]);
    target.x = std::array<double,2>{(*baseline.x)[0]+dx,(*baseline.x)[1]+dx};
    target.y = std::array<double,2>{(*baseline.y)[0]+dy,(*baseline.y)[1]+dy};
    changed = changed || delta.manhattanLength() >= 2;
    dirty = changed;
    redraw();
    return true;
  }
  if (event->type() == QEvent::MouseButtonRelease && active && !wheel &&
      static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
  {
    finish();
    return true;
  }
  if (event->type() == QEvent::Wheel)
  {
    const auto* input = static_cast<QWheelEvent*>(event);
    if (!active || !wheel)
    {
      cancel();
      begin();
      wheel = true;
    }
    if (!target.x || !target.y)
    {
      return true;
    }
    const double steps = input->angleDelta().y() != 0 ? input->angleDelta().y()/120.0 : input->pixelDelta().y()/40.0;
    const double factor = std::exp(-std::clamp(steps,-10.0,10.0)*0.12);
    for (auto* range : {&*target.x,&*target.y})
    {
      const double center = (*range)[0]*0.5+(*range)[1]*0.5;
      const double half = ((*range)[1]-(*range)[0])*0.5*factor;
      *range = {center-half,center+half};
    }
    changed = dirty = true;
    redraw();
    wheelTimer.start();
    return true;
  }
  return false;
}
