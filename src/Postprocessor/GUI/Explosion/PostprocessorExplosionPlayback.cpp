#include "PostprocessorExplosionPlayback.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "CubitMessage.hpp"
#include <QString>
#include <QWidget>
#include <utility>
PostprocessorExplosionPlayback::PostprocessorExplosionPlayback(
  std::weak_ptr<PostprocessorApplicationInterface> application,Dispatch silent,
  std::function<bool()> ready,std::function<bool()> paused,
  std::function<void()> redraw,QObject* parent)
  : QObject(parent),application(std::move(application)),silent(std::move(silent)),paused(std::move(paused))
{
  timer.setTimerType(Qt::PreciseTimer);
  timer.setInterval(Postprocessor::ExplosionDefaults::pulse_interval_ms);
  connect(&timer,&QTimer::timeout,this,[this,ready,redraw]()
  {
    if (active() && ready())
    {
      redraw();
    }
  });
  timer.start();
}

bool PostprocessorExplosionPlayback::active()
{
  const auto app = application.lock();
  auto* widget = qobject_cast<QWidget*>(parent());
  if (!app || (widget && !widget->isVisible()) || paused())
  {
    pause_clock();
    return false;
  }
  const auto& session = app->session();
  const auto view = session.views.find(session.active_view_id);
  if (view == session.views.end() || !view->second.explosion.pulse.enabled || app->animation_capture_active())
  {
    pause_clock();
    return false;
  }
  if (owner != view->first || !clock.isValid())
  {
    owner = view->first;
    clock.start();
  }
  return true;
}

void PostprocessorExplosionPlayback::advance_for_render()
{
  if (!active())
  {
    return;
  }
  const double seconds = static_cast<double>(clock.nsecsElapsed())/1e9;
  clock.restart();
  const auto command = QStringLiteral("post explode pulse advance ")+QString::number(seconds,'g',17)+
    QStringLiteral(" view ")+Postprocessor::plot_quote(owner);
  std::string error;
  if (silent && !silent(command.toUtf8().toStdString(),error))
  {
    if (lastError != error)
    {
      PRINT_ERROR("Cannot advance explosion pulse: %s\n",error.c_str());
      lastError = error;
    }
    return;
  }
  lastError.clear();
}

void PostprocessorExplosionPlayback::pause_clock()
{
  clock.invalidate();
}
