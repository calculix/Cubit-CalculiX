#include "PostprocessorViewport.hpp"
#include "PostprocessorScene.hpp"
#include "CameraInteraction.hpp"
#include "PostprocessorTriad.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"

#include <QEvent>
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
    application(active), viewId(view_id)
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
    return;
  }
  // CPU-side synchronization releases closed actors even while this view is hidden.
  // OpenGL rendering and camera fitting still wait for the initialized Qt widget.
  std::string error;
  if (!scene->synchronize(*active, error))
  {
    report_error(error);
  }
  const auto view = active->session().views.find(viewId);
  const auto& options = active->user_options();
  if (!renderedGeometry || *renderedGeometry != active->display_revision() ||
      view == active->session().views.end() || !renderedCamera || *renderedCamera != view->second.camera_revision ||
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
