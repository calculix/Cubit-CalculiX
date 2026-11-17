#include "PostprocessorLegendInteraction.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWidget>
#include <algorithm>
#include <iomanip>
#include <locale>
#include <sstream>
#include <utility>

PostprocessorLegendInteraction::PostprocessorLegendInteraction(QWidget* widget,PostprocessorScene* display,
  const std::string& view,std::weak_ptr<PostprocessorApplicationInterface> active,Dispatch submit,Dispatch silent,
  std::function<void()> update,std::function<void(const std::string&)> error)
  : surface(widget),scene(display),viewId(view),application(std::move(active)),submitCommand(std::move(submit)),
    silentCommand(std::move(silent)),redraw(std::move(update)),report(std::move(error))
{
}
bool PostprocessorLegendInteraction::owns_legend() const
{
  const auto active = application.lock();
  if (!active || !hit || active->session().active_view_id != viewId)
  {
    return false;
  }
  const auto display = active->session().representations.find(hit->representation_id);
  return display != active->session().representations.end() &&
    active->display_matches_source(display->second,hit->source_id,viewId) &&
    Postprocessor::same_coloring(display->second.coloring,expected);
}
bool PostprocessorLegendInteraction::send(const std::optional<std::array<double,2>>& position,bool journal)
{
  if (!owns_legend())
  {
    return false;
  }
  QString command = QStringLiteral("post legend position ");
  if (position)
  {
    std::ostringstream coordinates;
    coordinates.imbue(std::locale::classic());
    coordinates << std::setprecision(17) << (*position)[0] << ' ' << (*position)[1];
    command += QString::fromStdString(coordinates.str());
  }
  else
  {
    command += QStringLiteral("automatic");
  }
  command += QStringLiteral(" source ")+Postprocessor::plot_quote(sourceName);
  std::string error;
  bool accepted = false;
  if (journal)
  {
    accepted = submitCommand(command,error);
  }
  else
  {
    accepted = silentCommand(command,error);
  }
  if (accepted)
  {
    expected.legend_position = position;
    previewed = previewed || !journal;
  }
  else if (!error.empty())
  {
    report(error);
  }
  redraw();
  return accepted;
}
void PostprocessorLegendInteraction::cancel()
{
  pending = false;
  if (previewed && owns_legend())
  {
    send(baseline.legend_position,false);
  }
  hit.reset();
  moved = false;
  previewed = false;
}
void PostprocessorLegendInteraction::discard()
{
  pending = false;
  hit.reset();
  moved = false;
  previewed = false;
}
bool PostprocessorLegendInteraction::preview_pending() const
{
  return pending;
}
void PostprocessorLegendInteraction::flush_preview()
{
  if (pending)
  {
    pending = false;
    if (!send(target,false))
    {
      cancel();
    }
  }
}
bool PostprocessorLegendInteraction::handle(QEvent* event)
{
  if (hit && !owns_legend())
  {
    cancel();
  }
  if (event->type() == QEvent::Hide || event->type() == QEvent::WindowDeactivate || event->type() == QEvent::FocusOut)
  {
    cancel();
    return false;
  }
  if (hit && (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride) &&
      static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
  {
    if (event->type() == QEvent::KeyPress)
    {
      cancel();
    }
    return true;
  }
  if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
  {
    const auto* mouse = static_cast<QMouseEvent*>(event);
    if (mouse->button() != Qt::LeftButton || mouse->modifiers() != Qt::NoModifier)
    {
      return hit.has_value();
    }
    const auto active = application.lock();
    if (!active || active->session().active_view_id != viewId)
    {
      return false;
    }
    const auto found = scene->legend_at(mouse->position().x()/std::max(1,surface->width()),
      1.0-mouse->position().y()/std::max(1,surface->height()));
    if (!found)
    {
      return false;
    }
    const auto display = active->session().representations.find(found->representation_id);
    const auto source = active->session().sources.find(found->source_id);
    if (display == active->session().representations.end() || source == active->session().sources.end() ||
        !active->display_matches_source(display->second,found->source_id,viewId) ||
        Postprocessor::plot_quote(source->second.name).isEmpty())
    {
      return false;
    }
    if (event->type() == QEvent::MouseButtonDblClick)
    {
      const auto quoted = Postprocessor::plot_quote(source->second.name);
      cancel();
      std::string error;
      if (!submitCommand(QStringLiteral("post properties legend source ")+quoted,error))
      {
        report(error);
      }
      redraw();
      return true;
    }
    surface->setFocus(Qt::MouseFocusReason);
    hit = found;
    sourceName = source->second.name;
    baseline = display->second.coloring;
    expected = baseline;
    origin = mouse->position();
    target = {found->frame[0],found->frame[1]};
    pending = false;
    moved = false;
    previewed = false;
    return true;
  }
  if (!hit)
  {
    return false;
  }
  if (event->type() == QEvent::MouseMove)
  {
    const auto* mouse = static_cast<QMouseEvent*>(event);
    if (!mouse->buttons().testFlag(Qt::LeftButton))
    {
      cancel();
      return true;
    }
    const auto delta = mouse->position()-origin;
    target = {std::clamp(hit->frame[0]+delta.x()/std::max(1,surface->width()),0.0,1.0-hit->frame[2]),
      std::clamp(hit->frame[1]-delta.y()/std::max(1,surface->height()),0.0,1.0-hit->frame[3])};
    moved = moved || delta.manhattanLength() >= 2;
    pending = moved;
    redraw();
    return true;
  }
  if (event->type() == QEvent::MouseButtonRelease)
  {
    if (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)
    {
      pending = false;
      if (moved && !send(target,true))
      {
        cancel();
      }
      hit.reset();
      moved = false;
      previewed = false;
    }
    return true;
  }
  return event->type() == QEvent::Wheel || event->type() == QEvent::ContextMenu;
}
