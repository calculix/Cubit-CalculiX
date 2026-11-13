#ifndef POSTPROCESSORPLOTRENDERING_HPP
#define POSTPROCESSORPLOTRENDERING_HPP
#include "Postprocessor/Core/plots/PlotData.hpp"
#include <QLineF>
class vtkChartXY;
namespace Postprocessor
{
  struct PostprocessorSession;
}
void populate_plot_chart(vtkChartXY* chart,const Postprocessor::PlotDefinition& definition,
  const Postprocessor::PlotData& data,double scale = 1);
std::optional<QLineF> plot_cursor_line(vtkChartXY* chart,std::optional<double> marker,int height,double ratio);
std::optional<double> plot_frame_marker(const Postprocessor::PostprocessorSession& state,
  const Postprocessor::PlotDefinition& definition,const Postprocessor::PlotData& values);
#endif
