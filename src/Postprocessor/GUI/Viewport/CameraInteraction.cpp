#include <type_traits>
#include "CameraInteraction.hpp"
#include "CameraTimings.hpp"
#include "Postprocessor/Commands/views/CameraCommandText.hpp"
#include "Postprocessor/Core/views/CameraOperations.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QKeyEvent>
#include <QApplication>
#include <QMouseEvent>
#include <QScopedValueRollback>
#include <QWheelEvent>
#include <QWidget>
#include <cmath>
#include <algorithm>
#include <utility>

using namespace Postprocessor;

CameraInteraction::CameraInteraction(QWidget* widget, std::string view_id,
  std::weak_ptr<PostprocessorApplicationInterface> active, Dispatch submit, Dispatch silent,
  std::function<void()> update, std::function<void(const std::string&)> errors, CameraTimings* recorder,
  std::function<void()> timing_finished, std::function<void(const QPointF&)> click)
  : QObject(widget), surface(widget), viewId(std::move(view_id)), application(std::move(active)),
    submitCommand(std::move(submit)), silentCommand(std::move(silent)),
    redraw(std::move(update)), report(std::move(errors)), timings(recorder), timingFinished(std::move(timing_finished)), clicked(std::move(click))
{
  wheelTimer.setSingleShot(true);
  wheelTimer.setInterval(NavigationDefaults::wheel_commit_delay_ms);
  restoreTimer.setSingleShot(true);
  restoreTimer.setInterval(NavigationDefaults::recovery_interval_ms);
  connect(&wheelTimer, &QTimer::timeout, this, [this]()
  {
    finish();
  });
  connect(&restoreTimer, &QTimer::timeout, this, [this]()
  {
    restore();
  });
}

CameraInteraction::~CameraInteraction()
{
  cancel();
}

bool CameraInteraction::owns_camera() const
{
  const auto active = application.lock();
  if (!active || active->session().active_view_id != viewId)
  {
    return false;
  }
  const auto found = active->session().views.find(viewId);
  return found != active->session().views.end() && found->second.camera &&
    !found->second.fit_pending && found->second.camera_revision == cameraRevision;
}

void CameraInteraction::capture_scene_context()
{
  navigationDisplays.clear();
  navigationSources.clear();
  navigationFilters.clear();
  const auto app = application.lock();
  if (!app)
  {
    return;
  }
  const auto& state = app->session();
  navigationSource = state.active_source_id;
  navigationDisplay = state.active_representation_id;
  navigationSceneUnit = state.views.at(viewId).scene_unit;
  for (const auto& item : state.representations)
  {
    const auto& display = item.second;
    if (display.view_id != viewId)
    {
      continue;
    }
    navigationDisplays.emplace(item.first,display);
    const auto source = app->output_source(display.input);
    navigationSources[source] = app->result_data(source);
    if (display.input.owner == OutputOwner::Feature)
    {
      const auto filter = state.filters.find(display.input.object_id);
      if (filter != state.filters.end())
      {
        navigationFilters[filter->first] = filter->second.revision;
      }
    }
  }
}

bool CameraInteraction::scene_context_matches()
{
  const auto app = application.lock();
  if (!app || app->session().active_view_id != viewId)
  {
    return false;
  }
  if (app->display_revision() == sceneRevision)
  {
    return true;
  }
  const auto& state = app->session();
  const auto view = state.views.find(viewId);
  if (view == state.views.end() || view->second.scene_unit != navigationSceneUnit ||
      state.active_source_id != navigationSource || state.active_representation_id != navigationDisplay)
  {
    return false;
  }
  for (const auto& source : navigationSources)
  {
    if (app->result_data(source.first) != source.second)
    {
      return false;
    }
  }
  for (const auto& filter : navigationFilters)
  {
    const auto now = state.filters.find(filter.first);
    if (now == state.filters.end() || now->second.revision != filter.second)
    {
      return false;
    }
  }
  std::size_t count = 0;
  for (const auto& item : state.representations)
  {
    const auto& current = item.second;
    if (current.view_id != viewId)
    {
      continue;
    }
    ++count;
    const auto found = navigationDisplays.find(item.first);
    if (found == navigationDisplays.end())
    {
      return false;
    }
    const auto& original = found->second;
    if (current.input.owner != original.input.owner || current.input.object_id != original.input.object_id ||
        current.input.port != original.input.port || current.visible != original.visible || current.hidden_datasets != original.hidden_datasets ||
        !same_deformation(current.deformation,original.deformation) || !same_coloring(current.coloring,original.coloring) ||
        current.dataset_appearance.size() != original.dataset_appearance.size())
    {
      return false;
    }
    for (const auto& appearance : original.dataset_appearance)
    {
      const auto now = current.dataset_appearance.find(appearance.first);
      if (now == current.dataset_appearance.end() || !same_appearance(now->second,appearance.second))
      {
        return false;
      }
    }
  }
  if (count != navigationDisplays.size())
  {
    return false;
  }
  sceneRevision = app->display_revision();
  return true;
}

bool CameraInteraction::begin(Mode next, const QPointF& start)
{
  if (mode == Mode::Wheel)
  {
    finish();
  }
  else
  {
    cancel();
  }
  const auto active = application.lock();
  if (restorePending || !active || active->session().active_view_id != viewId ||
      !active->camera_navigation_ready(viewId))
  {
    return false;
  }
  const auto& view = active->session().views.at(viewId);
  baseline = *view.camera;
  target = baseline;
  cameraRevision = view.camera_revision;
  sceneRevision = active->display_revision();
  capture_scene_context();
  origin = start;
  point = start;
  size = surface->size();
  wheelSteps = 0.0;
  sensitivity = active->user_options().mouseSensitivity;
  reverseZoom = active->user_options().reverseZoom;
  if (next == Mode::Rotate)
  {
    const QPointF radial = start - QPointF(size.width() / 2.0, size.height() / 2.0);
    const double radius = std::min(size.width(), size.height()) * NavigationDefaults::spin_radius_fraction / 2;
    if (std::hypot(radial.x(), radial.y()) > radius)
    {
      next = Mode::Spin;
    }
  }
  dragStarted = next == Mode::Wheel;
  mode = next;
  if (timings != nullptr)
  {
    timings->begin(active->user_options().diagnosticsCamera);
  }
  return true;
}

bool CameraInteraction::send(const CameraDefinition& camera, bool journal)
{
  std::string error;
  QScopedValueRollback<bool> guard(dispatching, true);
  const auto active = application.lock();
  if (!active || active->session().active_view_id != viewId)
  {
    return false;
  }
  bool accepted = false;
  if (!journal)
  {
    // A gesture preview is an owner-thread update through the same application
    // boundary as the command handler. Avoid parsing a command for every frame.
    const auto started = [&]() -> std::common_type_t<decltype(timings->stamp()), decltype(CameraTimings::Stamp{})> {
      if (timings) {
        return timings->stamp();
      } else {
        return CameraTimings::Stamp{};
      }
    }();
    accepted = active->set_camera(camera, error);
    if (timings != nullptr)
    {
      timings->record(CameraTimingStage::ApplicationCameraUpdate, started);
    }
  }
  else
  {
    std::string command;
    const auto formattingStarted = [&]() -> std::common_type_t<decltype(timings->stamp()), decltype(CameraTimings::Stamp{})> {
      if (timings) {
        return timings->stamp();
      } else {
        return CameraTimings::Stamp{};
      }
    }();
    const bool formatted = format_camera_command(camera, command, error);
    if (timings != nullptr)
    {
      timings->record(CameraTimingStage::Formatting, formattingStarted);
    }
    if (!formatted)
    {
      report(error);
      return false;
    }
    const auto dispatchStarted = [&]() -> std::common_type_t<decltype(timings->stamp()), decltype(CameraTimings::Stamp{})> {
      if (timings) {
        return timings->stamp();
      } else {
        return CameraTimings::Stamp{};
      }
    }();
    CameraCommandTiming dispatchTiming(dispatchStarted.id != 0);
    accepted = submitCommand && submitCommand(QString::fromStdString(command), error);
    const auto dispatchResult = dispatchTiming.finish();
    if (timings != nullptr)
    {
      timings->record_dispatch(dispatchStarted, dispatchResult);
    }
  }
  if (!accepted)
  {
    report([&]() -> std::common_type_t<decltype("Cannot update the Postprocessor camera."), decltype(error)> {
      if (error.empty()) {
        return "Cannot update the Postprocessor camera.";
      } else {
        return error;
      }
    }());
    return false;
  }
  if (active->session().active_view_id != viewId)
  {
    return false;
  }
  const auto found = active->session().views.find(viewId);
  // Normal submission reports acceptance, so verify the authoritative result.
  if (found == active->session().views.end() || !found->second.camera ||
      found->second.fit_pending || !CameraOperations::equal(*found->second.camera, camera))
  {
    report("The camera command did not publish the requested camera.");
    return false;
  }
  cameraRevision = found->second.camera_revision;
  redraw();
  return true;
}

void CameraInteraction::preview()
{
  if (mode == Mode::None)
  {
    return;
  }
  const auto active = application.lock();
  if (!owns_camera() || !active || !scene_context_matches() ||
      !active->camera_navigation_ready(viewId))
  {
    cancel();
    return;
  }
  if (timings != nullptr)
  {
    timings->consume_input();
  }
  CameraDefinition candidate;
  std::string error;
  bool valid = false;
  const auto delta = point - origin;
  if (mode != Mode::Wheel && !dragStarted)
  {
    if (delta.manhattanLength() < QApplication::startDragDistance())
    {
      return;
    }
    dragStarted = true;
  }
  if (mode == Mode::Rotate)
  {
    valid = CameraOperations::rotate(baseline,
      -delta.x() * sensitivity * NavigationDefaults::drag_degrees / std::max(1, size.width()),
      -delta.y() * sensitivity * NavigationDefaults::drag_degrees / std::max(1, size.height()), candidate, error);
  }
  else if (mode == Mode::Pan)
  {
    valid = CameraOperations::pan_pixels(baseline, -delta.x() * sensitivity, delta.y() * sensitivity,
      std::max(1, size.height()), candidate, error);
  }
  else if (mode == Mode::Spin)
  {
    const QPointF center(size.width() / 2.0, size.height() / 2.0);
    const auto first = origin - center;
    const auto last = point - center;
    if (std::hypot(last.x(), last.y()) < 1e-6)
    {
      return;
    }
    const double angle = (std::atan2(last.y(), last.x()) - std::atan2(first.y(), first.x())) * 180 / std::acos(-1.0);
    valid = CameraOperations::roll(baseline, -angle * sensitivity, candidate, error);
  }
  else
  {
    const double sign = [&]() -> double {
      if (reverseZoom) {
        return -1;
      } else {
        return 1;
      }
    }();
    const double exponent = [&]() -> double {
      if (mode == Mode::Wheel) {
        return std::log(NavigationDefaults::wheel_factor) * wheelSteps;
      } else {
        return -delta.y() * NavigationDefaults::drag_zoom_exponent / std::max(1, size.height());
      }
    }();
    valid = CameraOperations::zoom(baseline, std::exp(sign * sensitivity * exponent), candidate, error);
  }
  if (!valid)
  {
    report(error);
    cancel();
    return;
  }
  if (CameraOperations::equal(candidate, target))
  {
    return;
  }
  if (send(candidate, false))
  {
    target = candidate;
  }
  else
  {
    cancel();
  }
}

void CameraInteraction::finish()
{
  pending = false;
  wheelTimer.stop();
  preview();
  if (mode == Mode::None)
  {
    return;
  }
  const auto active = application.lock();
  if (owns_camera() && active && !CameraOperations::equal(baseline, target))
  {
    if (active->user_options().journalCamera)
    {
      // Restore first so the final command is a real state change on replay.
      if (send(baseline, false))
      {
        send(target, true);
      }
    }
  }
  mode = Mode::None;
  if (timings != nullptr)
  {
    timings->end(false);
    if (timingFinished)
    {
      timingFinished();
    }
  }
}

void CameraInteraction::cancel()
{
  const bool hadGesture = mode != Mode::None;
  pending = false;
  wheelTimer.stop();
  if (mode != Mode::None)
  {
    restorePending = owns_camera() && !CameraOperations::equal(baseline, target);
    mode = Mode::None;
  }
  restore();
  if (timings != nullptr && hadGesture)
  {
    timings->end(true);
    if (timingFinished)
    {
      timingFinished();
    }
  }
}

void CameraInteraction::discard()
{
  wheelTimer.stop();
  restoreTimer.stop();
  pending = false;
  restorePending = false;
  mode = Mode::None;
  navigationDisplays.clear();
  navigationSources.clear();
  navigationFilters.clear();
}

bool CameraInteraction::active() const noexcept
{
  return mode != Mode::None || restorePending;
}

void CameraInteraction::restore()
{
  if (!restorePending)
  {
    return;
  }
  const auto active = application.lock();
  if (!owns_camera() || !active)
  {
    restorePending = false;
    return;
  }
  if (!active->camera_navigation_ready(viewId))
  {
    restoreTimer.start();
    return;
  }
  restorePending = false;
  send(baseline, false);
}

void CameraInteraction::synchronize()
{
  if (dispatching)
  {
    return;
  }
  const auto active = application.lock();
  if (mode != Mode::None && (!owns_camera() || !active || !scene_context_matches()))
  {
    cancel();
  }
  restore();
}

void CameraInteraction::fit()
{
  cancel();
  std::string error;
  const auto active = application.lock();
  if (active)
  {
    std::array<double,6> bounds;
    if (!active->visible_bounds(viewId,bounds,error))
    {
      if (!error.empty())
      {
        report(error);
      }
      // Skip dispatch so empty-view clicks/shortcuts produce no command output
      // or journal entry, including when camera journaling is enabled.
      return;
    }
  }
  const auto& dispatch = [&]() -> decltype((submitCommand)) {
    if (active && active->user_options().journalCamera) {
      return submitCommand;
    } else {
      return silentCommand;
    }
  }();
  if (!dispatch || !dispatch(QStringLiteral("post view fit"), error))
  {
    report(error);
  }
  else
  {
    redraw();
  }
}

bool CameraInteraction::handle(QEvent* event)
{
  switch (event->type())
  {
    case QEvent::MouseButtonPress:
    {
      auto* mouse = static_cast<QMouseEvent*>(event);
      if (mouse->button() == Qt::LeftButton || mouse->button() == Qt::MiddleButton || mouse->button() == Qt::RightButton)
      {
        surface->setFocus(Qt::MouseFocusReason);
        const auto active = application.lock();
        if (!active)
        {
          return true;
        }
        const auto& settings = active->user_options();
        const auto action = [&]() -> std::common_type_t<decltype(settings.leftDrag), std::common_type_t<decltype(settings.middleDrag), decltype(settings.rightDrag)>> {
          if (mouse->button() == Qt::LeftButton) {
            return settings.leftDrag;
          } else {
            return ([&]() -> std::common_type_t<decltype(settings.middleDrag), decltype(settings.rightDrag)> {
              if (mouse->button() == Qt::MiddleButton) {
                return settings.middleDrag;
              } else {
                return settings.rightDrag;
              }
            }());
          }
        }();
        const Mode next = [&]() -> Mode {
          if (action == MouseDragAction::Rotate) {
            return Mode::Rotate;
          } else {
            if (action == MouseDragAction::Zoom) {
              return Mode::Zoom;
            } else {
              return Mode::Pan;
            }
          }
        }();
        if (begin(next, mouse->position()))
        {
          button = mouse->button();
        }
      }
      return true;
    }
    case QEvent::MouseMove:
    {
      if (mode != Mode::None && mode != Mode::Wheel)
      {
        if (!static_cast<QMouseEvent*>(event)->buttons().testFlag(button))
        {
          cancel();
          return true;
        }
        point = static_cast<QMouseEvent*>(event)->position();
        if ((point-origin).manhattanLength() >= QApplication::startDragDistance())
        {
          dragStarted = true;
        }
        if (timings != nullptr)
        {
          timings->input();
        }
        pending = true;
        redraw();
      }
      return true;
    }
    case QEvent::MouseButtonRelease:
    {
      auto* mouse = static_cast<QMouseEvent*>(event);
      if (mode != Mode::None && mode != Mode::Wheel && mouse->button() == button)
      {
        point = mouse->position();
        const auto active = application.lock();
        const bool isClick = button == Qt::LeftButton && mouse->modifiers() == Qt::NoModifier && !dragStarted &&
          (point-origin).manhattanLength() < QApplication::startDragDistance() && owns_camera() && active &&
          scene_context_matches();
        if (timings != nullptr)
        {
          timings->input();
        }
        finish();
        if (isClick && clicked)
        {
          clicked(mouse->position());
        }
      }
      return true;
    }
    case QEvent::Wheel:
    {
      auto* wheel = static_cast<QWheelEvent*>(event);
      if (mode != Mode::Wheel && !begin(Mode::Wheel, wheel->position()))
      {
        return true;
      }
      if (wheel->angleDelta().y() != 0) {
        wheelSteps += wheel->angleDelta().y() / 120.0;
      } else {
        wheelSteps += wheel->pixelDelta().y() / NavigationDefaults::wheel_pixels_per_step;
      }
      if (timings != nullptr)
      {
        timings->input();
      }
      pending = true;
      redraw();
      wheelTimer.start();
      return true;
    }
    case QEvent::ShortcutOverride:
    {
      const auto* key = static_cast<QKeyEvent*>(event);
      return (key->modifiers() == Qt::NoModifier &&
        (key->key() == Qt::Key_F || key->key() == Qt::Key_Escape)) ||
        ((key->modifiers() == Qt::NoModifier || key->modifiers() == Qt::ShiftModifier) &&
         key->key() >= Qt::Key_F9 && key->key() <= Qt::Key_F12);
    }
    case QEvent::KeyPress:
    {
      auto* key = static_cast<QKeyEvent*>(event);
      if (key->key() == Qt::Key_Escape)
      {
        cancel();
      }
      else if (!key->isAutoRepeat() && key->key() >= Qt::Key_F9 && key->key() <= Qt::Key_F12 &&
        (key->modifiers() == Qt::NoModifier || key->modifiers() == Qt::ShiftModifier))
      {
        cancel();
        const auto active = application.lock();
        if (!active || active->session().active_view_id != viewId || !active->camera_navigation_ready(viewId))
        {
          return true;
        }
        using Standard = CameraOperations::StandardView;
        const bool shift = key->modifiers() == Qt::ShiftModifier;
        Standard next = Standard::Isometric;
        switch (key->key())
        {
          case Qt::Key_F9:
            if (shift) {
              next = Standard::Back;
            } else {
              next = Standard::Front;
            }
            break;
          case Qt::Key_F10:
            if (shift) {
              next = Standard::Bottom;
            } else {
              next = Standard::Top;
            }
            break;
          case Qt::Key_F11:
            if (shift) {
              next = Standard::Left;
            } else {
              next = Standard::Right;
            }
            break;
          case Qt::Key_F12:
            if (shift) {
              next = Standard::Orthographic;
            } else {
              next = Standard::Isometric;
            }
            break;
        }
        CameraDefinition camera;
        std::string error;
        if (CameraOperations::standard_view(*active->session().views.at(viewId).camera, next, camera, error))
        {
          send(camera, active->user_options().journalCamera);
        }
        else
        {
          report(error);
        }
      }
      else if (key->key() == Qt::Key_F && !key->isAutoRepeat() && key->modifiers() == Qt::NoModifier)
      {
        fit();
      }
      return true;
    }
    case QEvent::FocusOut:
    case QEvent::Hide:
    case QEvent::WindowDeactivate:
    case QEvent::UngrabMouse:
    case QEvent::Resize:
      cancel();
      return false;
    default:
      return false;
  }
}

bool CameraInteraction::preview_pending() const noexcept
{
  return pending;
}

void CameraInteraction::flush_preview()
{
  if (pending)
  {
    pending = false;
    preview();
  }
}

bool CameraInteraction::command_active() const noexcept
{
  return dispatching;
}
