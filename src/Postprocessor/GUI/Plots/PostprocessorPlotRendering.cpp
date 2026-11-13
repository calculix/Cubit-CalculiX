#include <vtkSmartPointer.h>
#include "PostprocessorPlotRendering.hpp"
#include "Postprocessor/Core/session/PostprocessorSession.hpp"
#include <vtkChartXY.h>
#include <vtkAxis.h>
#include <vtkPlot.h>
#include <vtkPlotPoints.h>
#include <vtkTable.h>
#include <vtkDoubleArray.h>
#include <vtkChartLegend.h>
#include <vtkTextProperty.h>
#include <algorithm>
#include <cmath>
#include <limits>
void populate_plot_chart(vtkChartXY* chart,const Postprocessor::PlotDefinition& definition,
  const Postprocessor::PlotData& data,double scale)
{
  chart->ClearPlots();
  chart->SetTitle(definition.name);
  for (std::size_t index = 0; index < definition.curves.size() && index < data.curves.size(); ++index)
  {
    const auto& curve = definition.curves[index];
    const auto& values = data.curves[index];
    if (!curve.visible || std::none_of(values.samples.begin(),values.samples.end(),[](const auto& sample)
      {
        return sample.valid;
      }))
    {
      continue;
    }
    auto samples = vtkSmartPointer<vtkTable>::New();
    auto x = vtkSmartPointer<vtkDoubleArray>::New();
    auto y = vtkSmartPointer<vtkDoubleArray>::New();
    x->SetName("X");
    y->SetName("Y");
    for (const auto& sample : values.samples)
    {
      const double missing = std::numeric_limits<double>::quiet_NaN();
      x->InsertNextValue(sample.valid ? sample.x : missing);
      y->InsertNextValue(sample.valid ? sample.y : missing);
    }
    samples->AddColumn(x);
    samples->AddColumn(y);
    auto* line = chart->AddPlot(vtkChart::LINE);
    line->SetInputData(samples,"X","Y");
    line->SetLabel(curve.name);
    line->SetWidth(Postprocessor::PlotDefaults::line_width*scale);
    const auto& color = curve.color;
    line->SetColor(color[0],color[1],color[2],255);
    if (auto* points = vtkPlotPoints::SafeDownCast(line))
    {
      points->SetMarkerStyle(vtkPlotPoints::CIRCLE);
      points->SetMarkerSize(Postprocessor::PlotDefaults::marker_size*scale);
    }
  }
  std::string xTitle;
  for (std::size_t index = 0; index < definition.curves.size() && index < data.curves.size(); ++index)
  {
    if (definition.curves[index].visible)
    {
      if (xTitle.empty())
      {
        xTitle = data.curves[index].x_title;
      }
      else if (xTitle != data.curves[index].x_title)
      {
        xTitle = "X (mixed columns)";
      }
    }
  }
  chart->GetAxis(vtkAxis::BOTTOM)->SetTitle(xTitle.empty() ? std::string("X") : xTitle);
  chart->GetAxis(vtkAxis::LEFT)->SetTitle(data.curves.size() == 1 ? data.curves.front().y_title : "Value");
  chart->SetShowLegend(Postprocessor::PlotDefaults::legend_visible);
  chart->SetIgnoreNanInBounds(true);
  for (int axis : {vtkAxis::BOTTOM,vtkAxis::LEFT})
  {
    auto* value = chart->GetAxis(axis);
    value->SetGridVisible(Postprocessor::PlotDefaults::grid_visible);
    if (scale != 1)
    {
      value->GetLabelProperties()->SetFontSize(std::max(1,static_cast<int>(std::lround(value->GetLabelProperties()->GetFontSize()*scale))));
      value->GetTitleProperties()->SetFontSize(std::max(1,static_cast<int>(std::lround(value->GetTitleProperties()->GetFontSize()*scale))));
    }
    const auto& range = axis == vtkAxis::BOTTOM ? definition.axes.x : definition.axes.y;
    value->SetBehavior(range ? vtkAxis::FIXED : vtkAxis::AUTO);
    if (range)
    {
      value->SetRange((*range)[0],(*range)[1]);
    }
  }
  if (scale != 1)
  {
    auto* text = chart->GetTitleProperties();
    text->SetFontSize(std::max(1,static_cast<int>(std::lround(text->GetFontSize()*scale))));
    auto* legend = chart->GetLegend()->GetLabelProperties();
    legend->SetFontSize(std::max(1,static_cast<int>(std::lround(legend->GetFontSize()*scale))));
  }
  chart->RecalculateBounds();
}
std::optional<QLineF> plot_cursor_line(vtkChartXY* chart,std::optional<double> marker,int height,double ratio)
{
  if (!marker || !std::isfinite(*marker))
  {
    return std::nullopt;
  }
  std::array<double,2> range;
  chart->GetAxis(vtkAxis::BOTTOM)->GetRange(range.data());
  if (!std::isfinite(range[0]) || !std::isfinite(range[1]) || range[0] >= range[1] || *marker < range[0] || *marker > range[1])
  {
    return std::nullopt;
  }
  auto* x = chart->GetAxis(vtkAxis::BOTTOM);
  auto* y = chart->GetAxis(vtkAxis::LEFT);
  const double fraction = (*marker-range[0])/(range[1]-range[0]);
  const double point = (x->GetPoint1()[0]+fraction*(x->GetPoint2()[0]-x->GetPoint1()[0]))/ratio;
  return QLineF(point,height-y->GetPoint1()[1]/ratio,point,height-y->GetPoint2()[1]/ratio);
}

std::optional<double> plot_frame_marker(const Postprocessor::PostprocessorSession& state,
  const Postprocessor::PlotDefinition& definition,const Postprocessor::PlotData& values)
{
  const auto view = state.views.find(state.active_view_id);
  std::optional<double> marker;
  std::optional<std::string> markerSource;
  std::optional<std::string> markerAxis;
  bool mixedAxes = false;
  for (std::size_t index = 0; index < definition.curves.size() && index < values.curves.size(); ++index)
  {
    if (!definition.curves[index].visible)
    {
      continue;
    }
    const auto& curve = values.curves[index];
    const auto source = Postprocessor::plot_curve_source(definition.curves[index]);
    mixedAxes = mixedAxes || (!curve.time_axis && !curve.frame_axis) || (markerAxis && *markerAxis != curve.x_title);
    markerAxis = curve.x_title;
    if (!markerSource || source == state.active_source_id)
    {
      markerSource = source;
    }
  }
  for (std::size_t index = 0; index < definition.curves.size() && index < values.curves.size(); ++index)
  {
    const auto& curve = values.curves[index];
    if (mixedAxes || !definition.curves[index].visible || (!curve.time_axis && !curve.frame_axis) ||
        !markerSource || Postprocessor::plot_curve_source(definition.curves[index]) != *markerSource)
    {
      continue;
    }
    const auto source = Postprocessor::plot_curve_source(definition.curves[index]);
    std::optional<std::size_t> frame;
    for (const auto& representation : state.representations)
    {
      if (representation.second.view_id == state.active_view_id && representation.second.input.object_id == source)
      {
        frame = representation.second.increment_index;
      }
    }
    if (view != state.views.end() && view->second.table && !view->second.plot_active && view->second.table->source_id == source)
    {
      frame = view->second.table->frame;
    }
    for (const auto& sample : curve.samples)
    {
      if (frame && sample.frame == frame && std::isfinite(sample.x))
      {
        marker = sample.x;
        break;
      }
    }
    break;
  }
  return marker;
}
