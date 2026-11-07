#ifndef POSTPROCESSORVIEWPORT_HPP
#define POSTPROCESSORVIEWPORT_HPP

#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <memory>
#include <string>

class vtkGenericOpenGLRenderWindow;
class vtkRenderer;
class PostprocessorScene;
class PostprocessorApplicationInterface;

// Independent scene. Shell-stage input is blocked until view/camera commands
// provide a journalable interaction contract. No Cubit scene objects are reused.
class PostprocessorViewport : public QVTKOpenGLNativeWidget
{
public:
  PostprocessorViewport(const std::string& view_id, QWidget* parent = nullptr);
  ~PostprocessorViewport() override;
  void synchronize(const std::shared_ptr<PostprocessorApplicationInterface>& application);

protected:
  void initializeGL() override;
  void resizeGL(int width, int height) override;
  bool event(QEvent* event) override;
  void showEvent(QShowEvent* event) override;

private:
  void queue_render();
  void render_scene();
  void report_error(const std::string& error);
  vtkSmartPointer<vtkGenericOpenGLRenderWindow> sceneWindow;
  vtkSmartPointer<vtkRenderer> renderer;
  std::unique_ptr<PostprocessorScene> scene;
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::string viewId;
  std::string lastError;
  bool renderQueued = false;
};

#endif // POSTPROCESSORVIEWPORT_HPP
