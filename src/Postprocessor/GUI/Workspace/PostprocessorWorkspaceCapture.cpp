#include "PostprocessorWorkspace.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "Postprocessor/GUI/Capture/PostprocessorAnimationCaptureDialog.hpp"
#include "Postprocessor/GUI/Capture/PostprocessorCaptureDialog.hpp"
#include "Postprocessor/GUI/Plots/PostprocessorPlotPane.hpp"
#include "Postprocessor/GUI/Resources/PostprocessorIconLoader.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorViewport.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QAction>
#include <QFileDialog>
#include <QMainWindow>
#include <QProgressDialog>
#include <QTimer>
#include <QToolBar>

void PostprocessorWorkspace::create_capture_toolbar(QMainWindow* host)
{
  captureToolbar = new QToolBar(tr("Capture"),host);
  captureToolbar->setObjectName("Postprocessor.CaptureToolbar");
  captureToolbar->setAllowedAreas(Qt::TopToolBarArea);
  captureToolbar->setMovable(true);
  captureToolbar->setFloatable(true);
  captureToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
  captureToolbar->hide();

  const char* captureIcons[] = {"PostprocessorCaptureViewport","PostprocessorRenderImage","PostprocessorRenderAnimation","PostprocessorAddToReport"};
  const QString captureTips[] = {tr("Save viewport screenshot"),tr("Render PNG image"),tr("Render animation"),tr("Add image to Report")};
  for (int index = 0; index < 4; ++index)
  {
    const auto app = this->application.lock();
    QIcon icon;
    if (app != nullptr)
    {
      icon = PostprocessorIconLoader::load(app->active_icons_path(), captureIcons[index]);
    }
    auto* action = captureToolbar->addAction(icon, captureTips[index]);
    action->setToolTip(captureTips[index]);
    connect(action,&QAction::triggered,this,[this,index]()
    {
      if (index == 2)
      {
        animation_capture_dialog();
      }
      else
      {
        capture_dialog(index == 3,index == 0);
      }
    });
  }
}

std::array<int,2> PostprocessorWorkspace::capture_reference_size(Postprocessor::CaptureTarget target,const std::string& id) const
{
  const auto active = application.lock();
  if (active && target == Postprocessor::CaptureTarget::View && id == active->session().active_view_id) {
    return {viewport->width(),viewport->height()};
  }
  if (active && target == Postprocessor::CaptureTarget::Plot) {
    const auto view = active->session().views.find(active->session().active_view_id);
    if (view != active->session().views.end() && view->second.plot_id == id) {
      const auto size = plotPane->chart_size();
      return {size.width(),size.height()};
    }
  }
  return {Postprocessor::CaptureDefaults::width,Postprocessor::CaptureDefaults::height};
}
void PostprocessorWorkspace::capture_dialog(bool report,bool quick) {
  const auto active = application.lock();
  if (!active) {
    return;
  }
  QString command;
  if (quick) {
    auto file = QFileDialog::getSaveFileName(this,tr("Save viewport screenshot"),{},tr("PNG images (*.png)"),nullptr,QFileDialog::DontConfirmOverwrite);
    if (file.isEmpty()) {
      return;
    }
    if (!file.endsWith(".png",Qt::CaseInsensitive)) {
      file += ".png";
    }
    const int width = qRound(viewport->width()*viewport->devicePixelRatioF());
    const int height = qRound(viewport->height()*viewport->devicePixelRatioF());
    command = "post capture view " + Postprocessor::plot_quote(active->session().active_view_id) +
      " file " + Postprocessor::plot_quote(file.toUtf8().toStdString()) + QString(" width %1 height %2").arg(width).arg(height);
  } else {
    command = postprocessor_capture_dialog(this,*active,report);
  }
  if (!command.isEmpty()) {
    submit(command);
  }
}

void PostprocessorWorkspace::animation_capture_dialog()
{
  const auto active = application.lock();
  if (!active)
  {
    return;
  }
  if (active->animation_capture_active())
  {
    show_animation_progress();
    return;
  }
  const auto command = postprocessor_animation_capture_dialog(this,*active);
  if (!command.isEmpty())
  {
    submit(command);
    QTimer::singleShot(0,this,[this]()
    {
      show_animation_progress();
    });
  }
}
void PostprocessorWorkspace::show_animation_progress()
{
  const auto active = application.lock();
  if (!active || !active->animation_capture_active())
  {
    return;
  }
  if (animationProgress)
  {
    animationProgress->show();
    animationProgress->raise();
    return;
  }
  auto* progress = new QProgressDialog(tr("Preparing animation export..."),tr("Cancel export"),0,0,this);
  animationProgress = progress;
  progress->setWindowTitle(tr("Render animation"));
  progress->setWindowModality(Qt::NonModal);
  progress->setMinimumDuration(0);
  progress->setAutoClose(false);
  progress->setAutoReset(false);
  progress->setAttribute(Qt::WA_DeleteOnClose);
  connect(progress,&QProgressDialog::canceled,this,[this]()
  {
    submit("post capture animation cancel");
  });
  auto* timer = new QTimer(progress);
  timer->setInterval(200);
  connect(timer,&QTimer::timeout,progress,[this,progress]()
  {
    const auto active = application.lock();
    if (!active || !active->animation_capture_active())
    {
      progress->blockSignals(true); // Programmatic completion must not issue a cancel command.
      progress->close();
      return;
    }
    progress->setLabelText(QString::fromStdString(active->animation_capture_status()));
  });
  timer->start();
  progress->show();
}
