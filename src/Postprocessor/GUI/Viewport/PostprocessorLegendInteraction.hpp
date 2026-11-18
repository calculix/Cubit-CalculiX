#ifndef POSTPROCESSORLEGENDINTERACTION_HPP
#define POSTPROCESSORLEGENDINTERACTION_HPP
#include "PostprocessorScene.hpp"
#include <QPointF>
#include <QString>
#include <functional>
class QEvent;
class QWidget;

// Drag previews use silent commands; release journals one normalized position.
class PostprocessorLegendInteraction
{
public:
  using Dispatch = std::function<bool(const QString&,std::string&)>;
  PostprocessorLegendInteraction(QWidget* surface,PostprocessorScene* scene,const std::string& view,
    std::weak_ptr<PostprocessorApplicationInterface> application,Dispatch submit,Dispatch silent,
    std::function<void()> redraw,std::function<void(const std::string&)> report);
  bool handle(QEvent* event);
  void flush_preview();
  bool preview_pending() const;
  void cancel();
  void discard();
private:
  bool owns_legend() const;
  bool send(const std::optional<std::array<double,2>>& position,bool journal);
  QWidget* surface;
  PostprocessorScene* scene;
  std::string viewId,sourceName;
  std::weak_ptr<PostprocessorApplicationInterface> application;
  Dispatch submitCommand,silentCommand;
  std::function<void()> redraw;
  std::function<void(const std::string&)> report;
  std::optional<PostprocessorScene::LegendHit> hit;
  Postprocessor::ResultColoring baseline,expected;
  Postprocessor::VectorPlotDefinition expectedVectors;
  QPointF origin;
  std::array<double,2> target{};
  bool pending = false,moved = false,previewed = false;
};
#endif
