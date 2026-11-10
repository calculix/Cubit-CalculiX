#include "PostprocessorViewport.hpp"
#include "PostprocessorScene.hpp"
#include "CameraInteraction.hpp"
#include "PostprocessorTriad.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include "Postprocessor/Commands/selection/PickCommandText.hpp"

#include <QEvent>
#include <QMouseEvent>
#include <QCursor>
#include <QToolTip>
#include <sstream>
#include <locale>
#include <QApplication>
#include <QShowEvent>
#include <QTimer>
#include <QScopedValueRollback>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkOpenGLRenderer.h>
#include <exception>
#include <utility>
#include <iomanip>
#include <locale>
#include <sstream>

PostprocessorViewport::PostprocessorViewport(const std::string& view_id,
  std::weak_ptr<PostprocessorApplicationInterface> active, Dispatch submit, Dispatch silent, QWidget* parent)
  : QVTKOpenGLNativeWidget(static_cast<vtkGenericOpenGLRenderWindow*>(nullptr), parent),
    submitCommand(submit), application(active), viewId(view_id)
{
  setObjectName("Postprocessor.Viewport");
  auto surfaceFormat = QVTKOpenGLNativeWidget::defaultFormat();
  surfaceFormat.setSwapInterval(1);
  setFormat(surfaceFormat);
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
  sceneWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  // Use the concrete backend without changing global VTK object factories.
  renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  sceneWindow->AddRenderer(renderer);
  setRenderWindow(sceneWindow.GetPointer());
  scene = std::make_unique<PostprocessorScene>(renderer, viewId);
  triad = std::make_unique<PostprocessorTriad>(sceneWindow);
  hoverTimer = new QTimer(this);
  hoverTimer->setSingleShot(true);
  hoverTimer->setInterval(Postprocessor::SelectionDefaults::hover_interval_ms);
  connect(hoverTimer,&QTimer::timeout,this,&PostprocessorViewport::update_hover);
  connect(this, &QOpenGLWidget::frameSwapped, this, &PostprocessorViewport::on_frame_presented);
  interaction = std::make_unique<CameraInteraction>(this, viewId, active, std::move(submit),
    std::move(silent), [this]()
    {
      queue_render();
    },
    [this](const std::string& error)
    {
      report_error(error);
    }, &timings, [this]()
    {
      queue_timing_report();
    },[this](const QPointF& position)
    {
      pick(position);
    });
}

PostprocessorViewport::~PostprocessorViewport()
{
  interaction.reset();
  timings.abandon_frames();
  report_timings();
  triad.reset();
}

void PostprocessorViewport::cancel_interaction()
{
  interaction->cancel();
}

void PostprocessorViewport::fit_view()
{
  interaction->fit();
}

void PostprocessorViewport::clear_hover()
{
  QToolTip::hideText();
  tooltipEntity.reset();
  tooltipText.clear();
  if (hoverTimer)
  {
    hoverTimer->stop();
  }
  if (scene && scene->clear_hover())
  {
    queue_render();
  }
}

void PostprocessorViewport::update_hover()
{
  const auto active = application.lock();
  if (!active || active->session().active_view_id != viewId || !isVisible() ||
      QApplication::mouseButtons() != Qt::NoButton || !rect().contains(mapFromGlobal(QCursor::pos())))
  {
    clear_hover();
    return;
  }
  const auto view = active->session().views.find(viewId);
  if (view == active->session().views.end())
  {
    clear_hover();
    return;
  }
  std::string error;
  if (!scene->synchronize(*active,error))
  {
    clear_hover();
    return;
  }
  std::optional<Postprocessor::EntityReference> entity;
  const double ratio = devicePixelRatioF();
  if (!scene->pick(hoverPosition.x()*ratio,(height()-1-hoverPosition.y())*ratio,view->second.selection.mode,entity,error))
  {
    clear_hover();
    return;
  }
  // Preselection is renderer-only feedback. It does not change Core selection,
  // the probe readout, the active source, or the command journal.
  if (scene->set_hover(entity,*active))
  {
    queue_render();
  }
  if (!entity)
  {
    tooltipEntity.reset();
    tooltipText.clear();
    QToolTip::hideText();
    return;
  }
  if (tooltipEntity && Postprocessor::same_entity(*tooltipEntity,*entity) && !tooltipText.isEmpty())
  {
    QToolTip::showText(mapToGlobal(hoverPosition.toPoint()),tooltipText,this);
    return;
  }
  const auto result = active->probe_entity(*entity,error);
  if (!result)
  {
    QToolTip::hideText();
    return;
  }
  std::ostringstream text;
  text.imbue(std::locale::classic());
  text << result->source_name << " / " << result->dataset_name << '\n';
  text << (entity->kind == Postprocessor::EntityKind::Node ? "Node: " : "Element: ") << entity->entity_id;
  if (entity->kind == Postprocessor::EntityKind::Node && !result->element_ids.empty())
  {
    text << "\nElement(s): ";
    for (std::size_t index = 0; index < result->element_ids.size(); ++index)
    {
      text << (index ? ", " : "") << result->element_ids[index];
    }
  }
  if (entity->integration_point)
  {
    text << "\nIntegration point: " << *entity->integration_point;
  }
  text << "\n" << result->field << " / " << result->component << ": ";
  if (result->values)
  {
    text << (*result->values)[0];
    if (result->range_value)
    {
      text << " .. " << (*result->values)[1];
    }
  }
  else
  {
    text << "Unavailable";
  }
  text << "\nStep " << result->step << " / Increment " << result->increment << " / Time " << result->time;
  tooltipEntity = entity;
  tooltipText = QString::fromStdString(text.str()).toHtmlEscaped().replace("\n","<br>");
  QToolTip::showText(mapToGlobal(hoverPosition.toPoint()),tooltipText,this);
}

void PostprocessorViewport::pick(const QPointF& position)
{
  clear_hover();
  const auto active = application.lock();
  if (!active || active->session().active_view_id != viewId)
  {
    return;
  }
  std::string error, command;
  if (!scene->synchronize(*active,error))
  {
    report_error(error);
    return;
  }
  const auto view = active->session().views.find(viewId);
  if (view == active->session().views.end())
  {
    return;
  }
  std::optional<Postprocessor::EntityReference> entity;
  const double ratio = devicePixelRatioF();
  if (!scene->pick(position.x()*ratio,(height()-1-position.y())*ratio,view->second.selection.mode,entity,error))
  {
    report_error(error);
    return;
  }
  if (entity)
  {
    if (!Postprocessor::pick_command_text(*active,*entity,command,error))
    {
      report_error(error);
      return;
    }
  }
  else
  {
    command = "post pick clear";
  }
  if (!submitCommand || !submitCommand(QString::fromUtf8(command.c_str()),error))
  {
    report_error(error.empty() ? "Selection command rejected." : error);
  }
  synchronize(active);
}

bool PostprocessorViewport::camera_command_active() const noexcept
{
  return interaction && interaction->command_active();
}

void PostprocessorViewport::initializeGL()
{
  QVTKOpenGLNativeWidget::initializeGL();
  queue_render();
}

void PostprocessorViewport::resizeGL(int width, int height)
{
  timings.abandon_frames();
  queue_timing_report();
  awaitingPresentation = false;
  QVTKOpenGLNativeWidget::resizeGL(width, height);
  queue_render();
}

bool PostprocessorViewport::event(QEvent* event)
{
  if (event->type() == QEvent::MouseMove && hoverTimer)
  {
    auto* mouse = static_cast<QMouseEvent*>(event);
    if (mouse->buttons() == Qt::NoButton && mouse->modifiers() == Qt::NoModifier)
    {
      hoverPosition = mouse->position();
      if (!hoverTimer->isActive())
      {
        hoverTimer->start();
      }
    }
    else
    {
      clear_hover();
    }
  }
  else if (event->type() == QEvent::Leave || event->type() == QEvent::MouseButtonPress ||
           event->type() == QEvent::Wheel || event->type() == QEvent::KeyPress ||
           event->type() == QEvent::Hide || event->type() == QEvent::WindowDeactivate)
  {
    clear_hover();
  }
  if (interaction && interaction->handle(event))
  {
    event->accept();
    return true;
  }
  if (event->type() == QEvent::Hide || event->type() == QEvent::WindowDeactivate)
  {
    timings.abandon_frames();
    queue_timing_report();
    awaitingPresentation = false;
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
    case QEvent::ContextMenu:
    case QEvent::Gesture:
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
      event->accept();
      return true;
    default:
      return QVTKOpenGLNativeWidget::event(event);
  }
}

void PostprocessorViewport::showEvent(QShowEvent* event)
{
  timings.abandon_frames();
  queue_timing_report();
  awaitingPresentation = false;
  QVTKOpenGLNativeWidget::showEvent(event);
  queue_render();
}

void PostprocessorViewport::synchronize(const std::shared_ptr<PostprocessorApplicationInterface>& active)
{
  interaction->synchronize();
  application = active;
  if (!active)
  {
    scene->clear();
    renderedGeometry.reset();
    renderedCamera.reset();
    renderedSelection.reset();
    return;
  }
  const auto currentView = active->session().views.find(viewId);
  if (!renderedGeometry || *renderedGeometry != active->display_revision() || currentView == active->session().views.end() ||
      !renderedCamera || *renderedCamera != currentView->second.camera_revision ||
      !renderedSelection || *renderedSelection != currentView->second.selection.revision)
  {
    clear_hover();
  }
  // CPU-side synchronization releases closed actors even while this view is hidden.
  // OpenGL rendering and camera fitting still wait for the initialized Qt widget.
  std::string error;
  if (!scene->synchronize(*active, error))
  {
    report_error(error);
  }
  report_warning();
  const auto view = active->session().views.find(viewId);
  const auto& options = active->user_options();
  if (!renderedGeometry || *renderedGeometry != active->display_revision() ||
      view == active->session().views.end() || !renderedCamera || *renderedCamera != view->second.camera_revision ||
      !renderedSelection || *renderedSelection != view->second.selection.revision ||
      options.backgroundTop != renderedOptions.backgroundTop || options.backgroundBottom != renderedOptions.backgroundBottom ||
      options.triadVisible != renderedOptions.triadVisible)
  {
    queue_render();
  }
}

void PostprocessorViewport::queue_render()
{
  request_frame();
}

void PostprocessorViewport::request_frame()
{
  frameDirty = true;
  // A redraw caused by flush_preview is consumed by this frame, not a second
  // queued frame. Keep diagnostic reference counts consistent with that rule.
  if (!renderingFrame)
  {
    timings.queue_frame();
  }
  if (renderQueued || awaitingPresentation || renderingFrame)
  {
    return;
  }
  renderQueued = true;
  QTimer::singleShot(0, this, [this]()
  {
    renderQueued = false;
    render_frame();
  });
}

void PostprocessorViewport::render_frame()
{
  if (renderingFrame || awaitingPresentation || !frameDirty || !isVisible() || !isValid() || width() <= 0 || height() <= 0)
  {
    return;
  }
  QScopedValueRollback<bool> guard(renderingFrame, true);
  timings.begin_frame();
  interaction->flush_preview();
  frameDirty = false;
  awaitingPresentation = true;
  render_scene();
  timings.await_presentation();
  // QVTK renders into its FBO; Qt then composes/presents that result. Never call
  // Render from paintGL or queue another VTK frame until this one is presented.
  update();
}

void PostprocessorViewport::on_frame_presented()
{
  timings.presented();
  queue_timing_report();
  awaitingPresentation = false;
  if (frameDirty || interaction->preview_pending())
  {
    request_frame();
  }
}

void PostprocessorViewport::report_warning()
{
  const auto& warning = scene->warning();
  if (warning != lastWarning)
  {
    if (!warning.empty())
    {
      PRINT_WARNING("Postprocessor deformation: %s\n",warning.c_str());
    }
    lastWarning = warning;
  }
}

void PostprocessorViewport::report_error(const std::string& error)
{
  if (error != lastError)
  {
    PRINT_ERROR("Postprocessor display: %s\n", error.c_str());
    lastError = error;
  }
}

void PostprocessorViewport::render_scene()
{
  const auto active = application.lock();
  if (!active)
  {
    return;
  }
  try
  {
    std::string error;
    const auto sceneStarted = timings.frame_stamp();
    const bool synchronized = scene->synchronize(*active, error);
    report_warning();
    timings.record(CameraTimingStage::SceneSynchronization, sceneStarted);
    bool completed = synchronized;
    if (!synchronized)
    {
      report_error(error);
    }
    const auto found = active->session().views.find(viewId);
    if (synchronized && found != active->session().views.end() && found->second.fit_pending)
    {
      const auto request = found->second.fit_request;
      const auto revision = active->display_revision();
      std::array<double, 6> bounds;
      // An empty new view keeps its initial fit pending until a mesh is visible.
      if (active->visible_bounds(viewId, bounds, error))
      {
        Postprocessor::CameraDefinition fitted;
        if (scene->prepare_camera_fit(bounds, fitted, error) &&
            active->complete_camera_fit(viewId, request, revision, fitted, error))
        {
          scene->apply_camera(fitted);
        }
        else
        {
          completed = false;
          report_error(error);
        }
      }
    }
    const auto& options = active->user_options();
    scene->apply_appearance(options.backgroundTop, options.backgroundBottom);
    const auto* dimensions = sceneWindow->GetSize();
    const auto triadStarted = timings.frame_stamp();
    triad->update(renderer->GetActiveCamera(), options.triadVisible, options.backgroundBottom,
      dimensions[0], dimensions[1], devicePixelRatioF());
    timings.record(CameraTimingStage::TriadUpdate, triadStarted);
    const auto renderStarted = timings.frame_stamp();
    sceneWindow->Render();
    timings.record(CameraTimingStage::RenderCall, renderStarted);
    if (completed)
    {
      renderedGeometry = active->display_revision();
      const auto current = active->session().views.find(viewId);
      if (current != active->session().views.end())
      {
        renderedCamera = current->second.camera_revision;
        renderedSelection = current->second.selection.revision;
      }
      // Copy only display scalars, never paths or complete session registries.
      renderedOptions.backgroundTop = options.backgroundTop;
      renderedOptions.backgroundBottom = options.backgroundBottom;
      renderedOptions.triadVisible = options.triadVisible;
      lastError.clear();
    }
  }
  catch (const std::exception& exception)
  {
    report_error(exception.what());
  }
}

void PostprocessorViewport::queue_timing_report()
{
  if (timingReportQueued || !timings.reports_ready())
  {
    return;
  }
  timingReportQueued = true;
  QTimer::singleShot(0, this, [this]()
  {
    timingReportQueued = false;
    report_timings();
  });
}

void PostprocessorViewport::report_timings()
{
  if (renderingFrame || camera_command_active())
  {
    return;
  }
  while (const auto report = timings.take_report())
  {
    std::ostringstream text;
    text.imbue(std::locale::classic());
    text << std::fixed << std::setprecision(3)
      << "Postprocessor camera timing: view " << viewId << ", gesture " << report->id
      << (report->cancelled ? " cancelled" : " completed")
      << "; frames " << report->frames << ", presented " << report->presented
      << ", unpresented " << report->unpresented << ", superseded " << report->superseded
      << ", unrendered " << report->unrendered
      << "; inputs " << report->inputs << ", coalesced " << report->coalescedInputs
      << ", dropped diagnostic gestures " << report->droppedGestures
      << ", unpaired dispatches " << report->unpairedDispatches << '\n';
    for (std::size_t index = 0; index < report->metrics.size(); ++index)
    {
      const auto& metric = report->metrics[index];
      text << "  " << CameraTimings::name(static_cast<CameraTimingStage>(index))
        << ": n=" << metric.count << ", avg=" << metric.average()
        << " ms, max=" << metric.maximumMilliseconds << " ms\n";
    }
    // One console emission per completed/cancelled gesture, never per frame.
    PRINT_INFO("%s", text.str().c_str());
  }
}
