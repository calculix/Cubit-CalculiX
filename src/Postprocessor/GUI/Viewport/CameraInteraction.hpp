#ifndef POSTPROCESSOR_CAMERAINTERACTION_HPP
#define POSTPROCESSOR_CAMERAINTERACTION_HPP

#include "Postprocessor/Core/views/ViewDefinition.hpp"
#include "Postprocessor/Core/representations/DisplayDefaults.hpp"
#include <QObject>
#include <QPointF>
#include <QSize>
#include <QTimer>
#include <QString>
#include <functional>
#include <memory>

class QWidget;
class CameraTimings;
class PostprocessorApplicationInterface;

// Owner-thread gesture transaction: direct application previews, optional single
// journal entry at commit. Revision tokens protect competing commands.
class CameraInteraction : public QObject
{
public:
  using Dispatch = std::function<bool(const QString&, std::string&)>;
  CameraInteraction(QWidget* surface, std::string view_id,
    std::weak_ptr<PostprocessorApplicationInterface> application,
    Dispatch submit, Dispatch silent, std::function<void()> redraw,
    std::function<void(const std::string&)> report, CameraTimings* timings = nullptr,
    std::function<void()> timing_finished = {}, std::function<void(const QPointF&)> clicked = {});
  ~CameraInteraction() override;
  bool handle(QEvent* event);
  void cancel();
  void synchronize();
  void fit();
  bool command_active() const noexcept;
  bool preview_pending() const noexcept;
  void flush_preview();

private:
  enum class Mode
  {
    None,
    Rotate,
    Pan,
    Zoom,
    Spin,
    Wheel
  };
  bool begin(Mode next, const QPointF& point);
  bool owns_camera() const;
  bool send(const Postprocessor::CameraDefinition& camera, bool journal);
  void preview();
  void finish();
  void restore();
  QWidget* surface;
  std::string viewId;
  std::weak_ptr<PostprocessorApplicationInterface> application;
  Dispatch submitCommand;
  Dispatch silentCommand;
  std::function<void()> redraw;
  std::function<void(const std::string&)> report;
  CameraTimings* timings = nullptr;
  std::function<void()> timingFinished;
  std::function<void(const QPointF&)> clicked;
  QTimer wheelTimer;
  QTimer restoreTimer;
  Mode mode = Mode::None;
  QPointF origin;
  QPointF point;
  QSize size;
  Qt::MouseButton button = Qt::NoButton;
  Postprocessor::CameraDefinition baseline;
  Postprocessor::CameraDefinition target;
  std::uint64_t cameraRevision = 0;
  std::uint64_t sceneRevision = 0;
  double wheelSteps = 0.0;
  bool pending = false;
  bool dragStarted = false;
  double sensitivity = Postprocessor::DisplayDefaults::mouse_sensitivity;
  bool reverseZoom = Postprocessor::DisplayDefaults::reverse_zoom;
  bool dispatching = false;
  bool restorePending = false;
};

#endif
