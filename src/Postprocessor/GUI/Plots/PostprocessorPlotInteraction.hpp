#ifndef POSTPROCESSORPLOTINTERACTION_HPP
#define POSTPROCESSORPLOTINTERACTION_HPP
#include "Postprocessor/Core/plots/PlotDefinition.hpp"
#include <QObject>
#include <QPointF>
#include <QSizeF>
#include <QTimer>
#include <functional>
class QEvent;
class PostprocessorPlotInteraction : public QObject
{
public:
  PostprocessorPlotInteraction(std::function<Postprocessor::PlotAxes()> current,
    std::function<Postprocessor::PlotAxes()> definition,std::function<QSizeF()> area,
    std::function<std::uint64_t()> revision,std::function<bool(const Postprocessor::PlotAxes&,bool)> send,
    std::function<void()> redraw,QObject* parent);
  bool handle(QEvent* event);
  void flush_preview();
  void cancel();
  bool pending() const;
private:
  void begin();
  void finish();
  bool dispatch(const Postprocessor::PlotAxes& axes,bool journal);
  std::function<Postprocessor::PlotAxes()> currentAxes, definitionAxes;
  std::function<QSizeF()> plotArea;
  std::function<std::uint64_t()> currentRevision;
  std::function<bool(const Postprocessor::PlotAxes&,bool)> send;
  std::function<void()> redraw;
  Postprocessor::PlotAxes baseline, baselineState, target;
  QTimer wheelTimer;
  QPointF origin;
  std::uint64_t revision = 0;
  bool active = false, dirty = false, changed = false, previewed = false, wheel = false;
};
#endif
