#ifndef POSTPROCESSORANNOTATIONINTERACTION_HPP
#define POSTPROCESSORANNOTATIONINTERACTION_HPP
#include <QPointF>
#include <QString>
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <cstdint>
class QEvent;
class QWidget;
class PostprocessorApplicationInterface;
class PostprocessorAnnotationDisplay;
class PostprocessorAnnotationInteraction
{
public:
  using Dispatch = std::function<bool(const QString&,std::string&)>;
  PostprocessorAnnotationInteraction(QWidget* surface, PostprocessorAnnotationDisplay* display,
    const std::string& view, std::weak_ptr<PostprocessorApplicationInterface> application,
    Dispatch submit, Dispatch silent, std::function<void()> redraw);
  bool handle(QEvent* event);
  void flush_preview();
  bool preview_pending() const;
  void cancel();
private:
  bool owns_label() const;
  bool owns_definition() const;
  bool send(const std::array<double,2>& position, bool journal);
  QWidget* surface;
  PostprocessorAnnotationDisplay* display;
  std::string viewId, label;
  std::weak_ptr<PostprocessorApplicationInterface> application;
  Dispatch submitCommand, silentCommand;
  std::function<void()> redraw;
  QPointF origin;
  std::array<double,2> baseline{}, target{};
  std::uint64_t revision = 0, geometry = 0;
  bool pending = false, moved = false, previewed = false;
};
#endif
