#ifndef POSTPROCESSORPLOTCHART_HPP
#define POSTPROCESSORPLOTCHART_HPP
#include "Postprocessor/Core/plots/PlotData.hpp"
#include <QVTKOpenGLNativeWidget.h>
#include <vtkSmartPointer.h>
#include <functional>
#include <memory>
class vtkGenericOpenGLRenderWindow;
class vtkContextView;
class vtkChartXY;
class vtkTable;
class PostprocessorPlotInteraction;
class PostprocessorPlotChart : public QVTKOpenGLNativeWidget
{
public:
  PostprocessorPlotChart(std::function<bool(const Postprocessor::PlotAxes&,bool)> send,
    std::function<std::uint64_t()> revision,QWidget* parent);
  ~PostprocessorPlotChart() override;
  void set_data(const Postprocessor::PlotDefinition& definition,const Postprocessor::PlotData& data);
  void apply_axes(const Postprocessor::PlotAxes& axes);
  Postprocessor::PlotAxes current_axes() const;
  void set_marker(std::optional<double> value);
  void cancel();
  void request_frame();
protected:
  void initializeGL() override;
  void resizeGL(int width,int height) override;
  bool event(QEvent* event) override;
  void showEvent(QShowEvent* event) override;
private:
  void render_frame();
  vtkSmartPointer<vtkGenericOpenGLRenderWindow> window;
  vtkSmartPointer<vtkContextView> context;
  vtkSmartPointer<vtkChartXY> chart;
  QWidget* cursor = nullptr;
  std::unique_ptr<PostprocessorPlotInteraction> interaction;
  Postprocessor::PlotAxes axes;
  std::optional<double> markerX;
  bool queued = false, dirty = false, presenting = false, rendering = false;
};
#endif
