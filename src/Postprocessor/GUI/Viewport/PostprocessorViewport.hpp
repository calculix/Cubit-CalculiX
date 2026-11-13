#ifndef POSTPROCESSORVIEWPORT_HPP
#define POSTPROCESSORVIEWPORT_HPP

#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <memory>
#include <string>
#include <functional>
#include "CameraTimings.hpp"
#include "Postprocessor/Core/selection/EntityReference.hpp"
#include "Postprocessor/Core/options/PostprocessorUserOptions.hpp"

class vtkGenericOpenGLRenderWindow;
class vtkRenderer;
class PostprocessorScene;
class PostprocessorApplicationInterface;
class CameraInteraction;
class PostprocessorTriad;
class QTimer;
class PostprocessorAnnotationDisplay;
class PostprocessorAnnotationInteraction;

// Independent scene and command-driven input. No Cubit scene objects are reused.
class PostprocessorViewport : public QVTKOpenGLNativeWidget
{
public:
  using Dispatch = std::function<bool(const QString&, std::string&)>;
  PostprocessorViewport(const std::string& view_id,
    std::weak_ptr<PostprocessorApplicationInterface> application,
    Dispatch submit, Dispatch silent, QWidget* parent = nullptr);
  ~PostprocessorViewport() override;
  void synchronize(const std::shared_ptr<PostprocessorApplicationInterface>& application);
  void cancel_interaction();
  void request_redraw();
  void fit_view();
  bool camera_command_active() const noexcept;

protected:
  void initializeGL() override;
  void resizeGL(int width, int height) override;
  bool event(QEvent* event) override;
  void showEvent(QShowEvent* event) override;

private:
  void queue_render();
  void request_frame();
  void render_frame();
  void on_frame_presented();
  void render_scene();
  void report_error(const std::string& error);
  void report_warning();
  void pick(const QPointF& position);
  void update_hover();
  void clear_hover();
  QTimer* hoverTimer = nullptr;
  QPointF hoverPosition;
  std::optional<Postprocessor::EntityReference> tooltipEntity;
  QString tooltipText;
  Dispatch submitCommand;
  void queue_timing_report();
  void report_timings();
  vtkSmartPointer<vtkGenericOpenGLRenderWindow> sceneWindow;
  vtkSmartPointer<vtkRenderer> renderer;
  std::unique_ptr<PostprocessorScene> scene;
  CameraTimings timings;
  std::unique_ptr<CameraInteraction> interaction;
  std::unique_ptr<PostprocessorTriad> triad;
  std::unique_ptr<PostprocessorAnnotationDisplay> annotationDisplay;
  std::unique_ptr<PostprocessorAnnotationInteraction> annotationInteraction;
  std::optional<std::uint64_t> renderedAnnotations;
  std::string renderedAnnotationSource, renderedAnnotationRepresentation;
  QPointF contextPress;
  bool contextClick = true;
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::string viewId;
  std::string lastError;
  std::string lastWarning;
  std::optional<std::uint64_t> renderedGeometry;
  std::optional<std::uint64_t> renderedCamera;
  std::optional<std::uint64_t> renderedSelection;
  Postprocessor::UserOptions renderedOptions;
  bool timingReportQueued = false;
  bool renderQueued = false;
  bool frameDirty = false;
  bool awaitingPresentation = false;
  bool renderingFrame = false;
};

#endif // POSTPROCESSORVIEWPORT_HPP
