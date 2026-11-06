#ifndef POSTPROCESSORVIEWPORT_HPP
#define POSTPROCESSORVIEWPORT_HPP

#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>

class vtkGenericOpenGLRenderWindow;
class vtkRenderer;

// Independent scene. Shell-stage input is blocked until view/camera commands
// provide a journalable interaction contract. No Cubit scene objects are reused.
class PostprocessorViewport : public QVTKOpenGLNativeWidget
{
public:
  explicit PostprocessorViewport(QWidget* parent = nullptr);
  ~PostprocessorViewport() override;

protected:
  bool event(QEvent* event) override;
  void showEvent(QShowEvent* event) override;

private:
  vtkSmartPointer<vtkGenericOpenGLRenderWindow> sceneWindow;
  vtkSmartPointer<vtkRenderer> renderer;
};

#endif // POSTPROCESSORVIEWPORT_HPP
