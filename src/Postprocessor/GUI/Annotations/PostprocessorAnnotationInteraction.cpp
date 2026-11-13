#include "PostprocessorAnnotationInteraction.hpp"
#include <utility>
#include "PostprocessorAnnotationDisplay.hpp"
#include "PostprocessorAnnotationMenu.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWidget>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <locale>

PostprocessorAnnotationInteraction::PostprocessorAnnotationInteraction(QWidget* widget,
  PostprocessorAnnotationDisplay* overlay, const std::string& view,
  std::weak_ptr<PostprocessorApplicationInterface> active, Dispatch submit, Dispatch silent, std::function<void()> update)
  : surface(widget), display(overlay), viewId(view), application(std::move(active)),
    submitCommand(std::move(submit)), silentCommand(std::move(silent)), redraw(std::move(update))
{
}
bool PostprocessorAnnotationInteraction::owns_label() const
{
  const auto active = application.lock();
  return active && active->display_revision() == geometry && owns_definition();
}
bool PostprocessorAnnotationInteraction::owns_definition() const
{
  const auto active = application.lock();
  if (!active || label.empty())
  {
    return false;
  }
  const auto view = active->session().views.find(viewId);
  return view != active->session().views.end() && view->second.annotations.revision == revision &&
    (label == "@minimum" || label == "@maximum" || view->second.annotations.labels.count(label) != 0);
}
bool PostprocessorAnnotationInteraction::send(const std::array<double,2>& position, bool journal)
{
  if (!owns_definition())
  {
    return false;
  }
  std::ostringstream coordinates;
  coordinates.imbue(std::locale::classic());
  coordinates << std::setprecision(17) << position[0] << ' ' << position[1];
  const auto command = QStringLiteral("post label position ")+annotation_quote(label)+" "+
    QString::fromStdString(coordinates.str())+" view "+annotation_quote(viewId);
  std::string error;
  const bool accepted = journal ? submitCommand(command,error) : silentCommand(command,error);
  if (accepted)
  {
    previewed = previewed || !journal;
    if (const auto active = application.lock())
    {
      const auto view = active->session().views.find(viewId);
      if (view != active->session().views.end())
      {
        revision = view->second.annotations.revision;
      }
    }
  }
  redraw();
  return accepted;
}
void PostprocessorAnnotationInteraction::cancel()
{
  pending = false;
  if (previewed && owns_definition())
  {
    send(baseline,false);
  }
  label.clear();
  moved = false;
  previewed = false;
}
bool PostprocessorAnnotationInteraction::preview_pending() const
{
  return pending;
}
void PostprocessorAnnotationInteraction::flush_preview()
{
  if (pending)
  {
    pending = false;
    if (!owns_label())
    {
      cancel();
      return;
    }
    if (!send(target,false))
    {
      label.clear();
    }
  }
}
bool PostprocessorAnnotationInteraction::handle(QEvent* event)
{
  if (!label.empty() && !owns_label())
  {
    cancel();
  }
  if (event->type() == QEvent::Hide || event->type() == QEvent::WindowDeactivate || event->type() == QEvent::FocusOut)
  {
    cancel();
    return false;
  }
  if (event->type() == QEvent::KeyPress && !label.empty() && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
  {
    cancel();
    return true;
  }
  if (event->type() == QEvent::ShortcutOverride && !label.empty() && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape)
  {
    return true;
  }
  if (event->type() == QEvent::MouseButtonPress)
  {
    const auto* mouse = static_cast<QMouseEvent*>(event);
    if (!label.empty())
    {
      return true;
    }
    if (mouse->button() != Qt::LeftButton || mouse->modifiers() != Qt::NoModifier)
    {
      return false;
    }
    const auto hit = display->label_at(mouse->position());
    const auto active = application.lock();
    if (hit.empty() || !active)
    {
      return false;
    }
    const auto view = active->session().views.find(viewId);
    if (view == active->session().views.end())
    {
      return false;
    }
    const auto& annotations = view->second.annotations;
    const auto found = annotations.labels.find(hit);
    if (hit == "@minimum" || hit == "@maximum")
    {
      baseline = hit == "@minimum" ? annotations.minimum_position : annotations.maximum_position;
    }
    else if (found != annotations.labels.end())
    {
      baseline = found->second.position;
    }
    else
    {
      return false;
    }
    label = hit;
    surface->setFocus(Qt::MouseFocusReason);
    target = baseline;
    origin = mouse->position();
    revision = annotations.revision;
    geometry = active->display_revision();
    moved = false;
    previewed = false;
    pending = false;
    return true;
  }
  if (label.empty())
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
    target = {std::clamp(baseline[0]+delta.x()/std::max(1,surface->width()),0.0,1.0),
      std::clamp(baseline[1]-delta.y()/std::max(1,surface->height()),0.0,1.0)};
    moved = moved || delta.manhattanLength() >= 2;
    pending = moved;
    redraw();
    return true;
  }
  if (event->type() == QEvent::MouseButtonRelease)
  {
    if (static_cast<QMouseEvent*>(event)->button() != Qt::LeftButton)
    {
      return true;
    }
    pending = false;
    if (moved && !send(target,true))
    {
      cancel();
    }
    label.clear();
    moved = false;
    previewed = false;
    return true;
  }
  return event->type() == QEvent::Wheel || event->type() == QEvent::ContextMenu;
}
