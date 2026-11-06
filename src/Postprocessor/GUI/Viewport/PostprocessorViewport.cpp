#include "PostprocessorViewport.hpp"

#include <QEvent>
#include <QShowEvent>
#include <QTimer>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkOpenGLRenderer.h>

PostprocessorViewport::PostprocessorViewport(QWidget* parent)
  : QVTKOpenGLNativeWidget(static_cast<vtkGenericOpenGLRenderWindow*>(nullptr), parent)
{
  setObjectName("Postprocessor.Viewport");
  setFormat(QVTKOpenGLNativeWidget::defaultFormat());
  setFocusPolicy(Qt::NoFocus);
  sceneWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  // Use the concrete backend without changing global VTK object factories.
  renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  sceneWindow->AddRenderer(renderer);
  setRenderWindow(sceneWindow.GetPointer());
}

PostprocessorViewport::~PostprocessorViewport() = default;

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
  // Rendering is queued until Qt has initialized the widget's OpenGL context.
  QTimer::singleShot(0, this, [this]()
  {
    if (isVisible() && isValid())
    {
      sceneWindow->Render();
    }
  });
}
