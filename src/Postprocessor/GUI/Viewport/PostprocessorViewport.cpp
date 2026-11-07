#include "PostprocessorViewport.hpp"
#include "PostprocessorScene.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"

#include <QEvent>
#include <QShowEvent>
#include <QTimer>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkOpenGLRenderer.h>
#include <exception>

PostprocessorViewport::PostprocessorViewport(const std::string& view_id, QWidget* parent)
  : QVTKOpenGLNativeWidget(static_cast<vtkGenericOpenGLRenderWindow*>(nullptr), parent), viewId(view_id)
{
  setObjectName("Postprocessor.Viewport");
  setFormat(QVTKOpenGLNativeWidget::defaultFormat());
  setFocusPolicy(Qt::NoFocus);
  sceneWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  // Use the concrete backend without changing global VTK object factories.
  renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  sceneWindow->AddRenderer(renderer);
  setRenderWindow(sceneWindow.GetPointer());
  scene = std::make_unique<PostprocessorScene>(renderer, viewId);
}

PostprocessorViewport::~PostprocessorViewport() = default;

void PostprocessorViewport::initializeGL()
{
  QVTKOpenGLNativeWidget::initializeGL();
  queue_render();
}

void PostprocessorViewport::resizeGL(int width, int height)
{
  QVTKOpenGLNativeWidget::resizeGL(width, height);
  queue_render();
}

bool PostprocessorViewport::event(QEvent* event)
{
  switch (event->type())
  {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::Wheel:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
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
  QVTKOpenGLNativeWidget::showEvent(event);
  queue_render();
}

void PostprocessorViewport::synchronize(const std::shared_ptr<PostprocessorApplicationInterface>& active)
{
  application = active;
  if (!active)
  {
    scene->clear();
    return;
  }
  // CPU-side synchronization releases closed actors even while this view is hidden.
  // OpenGL rendering and camera fitting still wait for the initialized Qt widget.
  std::string error;
  if (!scene->synchronize(*active, error))
  {
    report_error(error);
  }
  queue_render();
}

void PostprocessorViewport::queue_render()
{
  if (renderQueued)
  {
    return;
  }
  renderQueued = true;
  QTimer::singleShot(0, this, [this]()
  {
    renderQueued = false;
    if (isVisible() && isValid() && width() > 0 && height() > 0)
    {
      render_scene();
    }
  });
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
    const bool synchronized = scene->synchronize(*active, error);
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
    sceneWindow->Render();
    if (completed)
    {
      lastError.clear();
    }
  }
  catch (const std::exception& exception)
  {
    report_error(exception.what());
  }
}
