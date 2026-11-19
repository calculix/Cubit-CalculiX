#include "PostprocessorVectorActors.hpp"
#include "Postprocessor/Core/explosion/ExplosionPulseGeometry.hpp"
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkOpenGLActor.h>
#include <vtkProperty.h>
#include <vtkGlyph3DMapper.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkTextRenderer.h>
#include <vtkRenderWindow.h>
#include <algorithm>
#include <vtkIdTypeArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkTextProperty.h>
#include <vtkNew.h>
#include <cmath>
namespace
{
  vtkSmartPointer<vtkPolyData> arrow_geometry()
  {
    auto result = vtkSmartPointer<vtkPolyData>::New();
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> triangles;
    constexpr vtkIdType sides = 12;
    for (vtkIdType ring = 0; ring < 3; ++ring)
    {
      double x = 0.7;
      double radius = 0.025;
      if (ring == 0)
      {
        x = 0;
      }
      if (ring == 2)
      {
        radius = 0.08;
      }
      for (vtkIdType side = 0; side < sides; ++side)
      {
        const double angle = 6.283185307179586*static_cast<double>(side)/sides;
        points->InsertNextPoint(x,radius*std::cos(angle),radius*std::sin(angle));
      }
    }
    const vtkIdType tip = points->InsertNextPoint(1,0,0);
    const vtkIdType base = points->InsertNextPoint(0,0,0);
    const auto triangle = [&](vtkIdType a,vtkIdType b,vtkIdType c)
    {
      const vtkIdType ids[] = {a,b,c};
      triangles->InsertNextCell(3,ids);
    };
    for (vtkIdType side = 0; side < sides; ++side)
    {
      const auto next = (side+1)%sides;
      triangle(base,next,side);
      triangle(side,next,sides+side);
      triangle(next,sides+next,sides+side);
      triangle(sides+side,sides+next,2*sides+side);
      triangle(sides+next,2*sides+next,2*sides+side);
      triangle(2*sides+side,2*sides+next,tip);
    }
    result->SetPoints(points);
    result->SetPolys(triangles);
    return result;
  }
}
PostprocessorVectorActors::PostprocessorVectorActors(vtkRenderer* target,const Postprocessor::VectorPlotData& data,
  const Postprocessor::VectorPlotDefinition& settings,double sceneScale,unsigned int legendSlot,bool showLegend) : renderer(target)
{
  if (data.samples.empty())
  {
    return;
  }
  vtkNew<vtkPolyData> samples;
  pulseSamples = samples.GetPointer();
  vtkNew<vtkDoubleArray> pulseBase;
  vtkNew<vtkDoubleArray> pulseComponents;
  vtkNew<vtkDoubleArray> pulseMesh;
  pulseBase->SetName("PostprocessorPulseBase");
  pulseComponents->SetName("PostprocessorPulseComponents");
  pulseMesh->SetName("PostprocessorPulseMesh");
  for (auto* array : {pulseBase.GetPointer(),pulseComponents.GetPointer(),pulseMesh.GetPointer()})
  {
    array->SetNumberOfComponents(3);
    samples->GetPointData()->AddArray(array);
  }
  vtkNew<vtkPoints> points;
  points->SetDataTypeToDouble();
  vtkNew<vtkDoubleArray> directions;
  directions->SetName("VectorDirection");
  directions->SetNumberOfComponents(3);
  vtkNew<vtkDoubleArray> lengths;
  lengths->SetName("VectorLength");
  vtkNew<vtkDoubleArray> magnitudes;
  magnitudes->SetName("VectorMagnitude");
  const auto& coloring = settings.legend;
  std::array<double,2> range = {0,data.reference_magnitude};
  if (!std::isfinite(range[1]) || range[1] <= 0)
  {
    range[1] = 1;
  }
  if (coloring.range_mode == Postprocessor::ColorRangeMode::Automatic || coloring.range_mode == Postprocessor::ColorRangeMode::Symmetric)
  {
    range = {data.samples.front().magnitude,data.samples.front().magnitude};
    for (const auto& sample : data.samples)
    {
      range[0] = std::min(range[0],sample.magnitude);
      range[1] = std::max(range[1],sample.magnitude);
    }
    if (coloring.range_mode == Postprocessor::ColorRangeMode::Symmetric)
    {
      const double extent = std::max(std::abs(range[0]),std::abs(range[1]));
      range = {-extent,extent};
    }
    if (range[0] == range[1])
    {
      const double extent = std::max(1.0,std::abs(range[0]))*.01;
      range = {range[0]-extent,range[1]+extent};
    }
  }
  if (coloring.range_mode == Postprocessor::ColorRangeMode::Fixed)
  {
    range = coloring.fixed_range;
  }
  vtkNew<vtkUnsignedCharArray> colors;
  colors->SetName("VectorColor");
  colors->SetNumberOfComponents(3);
  vtkNew<vtkIdTypeArray> datasets;
  datasets->SetName("VectorDatasetId");
  vtkNew<vtkTypeInt64Array> entities;
  entities->SetName("VectorEntityId");
  vtkNew<vtkTypeInt64Array> integrationPoints;
  integrationPoints->SetName("VectorIntegrationPointId");
  for (const auto& sample : data.samples)
  {
    points->InsertNextPoint(sample.position.data());
    pulseBase->InsertNextTuple(sample.pulse_base.data());
    pulseComponents->InsertNextTuple(sample.pulse_components.data());
    pulseMesh->InsertNextTuple(sample.pulse_mesh.data());
    directions->InsertNextTuple(sample.direction.data());
    lengths->InsertNextValue(sample.length);
    magnitudes->InsertNextValue(sample.magnitude);
    const auto rgb = Postprocessor::result_color(sample.magnitude,coloring,range);
    unsigned char bytes[3];
    for (int channel = 0; channel < 3; ++channel)
    {
      bytes[channel] = static_cast<unsigned char>(std::round(std::clamp(rgb[channel],0.0,1.0)*255));
    }
    colors->InsertNextTypedTuple(bytes);
    datasets->InsertNextValue(static_cast<vtkIdType>(sample.dataset));
    entities->InsertNextValue(sample.entity);
    integrationPoints->InsertNextValue(sample.integration_point);
  }
  samples->SetPoints(points);
  samples->GetPointData()->AddArray(colors);
  for (vtkDataArray* array : {static_cast<vtkDataArray*>(directions.GetPointer()),static_cast<vtkDataArray*>(lengths.GetPointer()),
       static_cast<vtkDataArray*>(magnitudes.GetPointer()),static_cast<vtkDataArray*>(datasets.GetPointer()),
       static_cast<vtkDataArray*>(entities.GetPointer()),static_cast<vtkDataArray*>(integrationPoints.GetPointer())})
  {
    samples->GetPointData()->AddArray(array);
  }
  vtkNew<vtkGlyph3DMapper> mapper;
  mapper->SetInputData(samples);
  mapper->SetSourceData(arrow_geometry());
  mapper->SetOrientationArray("VectorDirection");
  mapper->SetOrientationModeToDirection();
  mapper->OrientOn();
  mapper->SetScaleArray("VectorLength");
  mapper->SetScaleModeToScaleByMagnitude();
  mapper->SetScaleFactor(1);
  mapper->ScalingOn();
  mapper->ScalarVisibilityOff();
  vtkNew<vtkLookupTable> lookup;
  lookup->SetRange(range.data());
  const int count = static_cast<int>(coloring.bands.value_or(256));
  lookup->SetNumberOfTableValues(count);
  lookup->Build();
  for (int index = 0; index < count; ++index)
  {
    const double t = static_cast<double>(index)/(count-1);
    const auto color = Postprocessor::result_color(range[0]*(1-t)+range[1]*t,coloring,range);
    lookup->SetTableValue(index,color[0],color[1],color[2],1);
  }
  if (settings.magnitude_color)
  {
    mapper->SetScalarModeToUsePointFieldData();
    mapper->SetColorModeToDirectScalars();
    mapper->SelectColorArray("VectorColor");
    mapper->ScalarVisibilityOn();
  }
  actor = vtkSmartPointer<vtkOpenGLActor>::New();
  actor->SetMapper(mapper);
  actor->SetScale(sceneScale,sceneScale,sceneScale);
  actor->GetProperty()->SetColor(settings.color[0],settings.color[1],settings.color[2]);
  actor->GetProperty()->LightingOff();
  // Existing entity picking continues to address the underlying mesh. Sample
  // provenance is retained above, including -1 for generated filter samples.
  actor->PickableOff();
  renderer->AddActor(actor);
  position = coloring.legend_position;
  slot = legendSlot;
  if (settings.magnitude_color && showLegend && coloring.legend_visible)
  {
    legend = vtkSmartPointer<vtkScalarBarActor>::New();
    legend->SetLookupTable(lookup);
    std::string title = "Vector magnitude";
    if (!data.unit.empty())
    {
      title += " ("+data.unit+")";
    }
    legend->SetTitle(title.c_str());
    legend->SetNumberOfLabels(5);
    legend->SetMaximumNumberOfColors(count);
    legend->SetLabelFormat("%.6g");
    legend->SetBarRatio(Postprocessor::ColoringDefaults::legend_bar_pixels/Postprocessor::ColoringDefaults::legend_width_pixels);
    legend->UnconstrainedFontSizeOn();
    for (auto* text : {legend->GetLabelTextProperty(),legend->GetTitleTextProperty()})
    {
      text->SetFontSize(Postprocessor::ColoringDefaults::legend_font_pixels);
      text->BoldOff();
      text->ItalicOff();
      text->ShadowOff();
    }
    update_appearance();
    renderer->AddActor2D(legend);
  }
}
PostprocessorVectorActors::~PostprocessorVectorActors()
{
  if (actor)
  {
    renderer->RemoveActor(actor);
  }
  if (legend)
  {
    renderer->RemoveActor2D(legend);
  }
}
void PostprocessorVectorActors::apply_pulse(const Postprocessor::ExplosionState& settings)
{
  Postprocessor::ExplosionPointBuffers updated;
  Postprocessor::apply_explosion_pulse_mesh(pulseSamples,settings.pulse,updated);
}
void PostprocessorVectorActors::scale_capture(double scale)
{
  captureScale = scale;
  update_appearance();
}
std::array<double,4> PostprocessorVectorActors::legend_frame() const
{
  if (!legend)
  {
    return {0,0,0,0};
  }
  const auto* p = legend->GetPosition();
  return {p[0],p[1],legend->GetWidth(),legend->GetHeight()};
}
bool PostprocessorVectorActors::legend_contains(double x,double y) const
{
  if (!legend || !legend->GetVisibility())
  {
    return false;
  }
  const auto frame = legend_frame();
  double left = frame[0],right = frame[0]+frame[2];
  int bounds[4];
  auto* text = vtkTextRenderer::GetInstance();
  auto* window = renderer->GetRenderWindow();
  if (text && window && legend->GetTitle() && text->GetBoundingBox(legend->GetTitleTextProperty(),legend->GetTitle(),bounds,window->GetDPI()))
  {
    const double half = .5*(bounds[1]-bounds[0]+1)/std::max(1,renderer->GetSize()[0]);
    left = std::min(left,frame[0]+frame[2]*.5-half);
    right = std::max(right,frame[0]+frame[2]*.5+half);
  }
  return x >= left && x <= right && y >= frame[1] && y <= frame[1]+frame[3];
}
void PostprocessorVectorActors::update_appearance()
{
  if (!legend)
  {
    return;
  }
  const double width = std::min(.8,Postprocessor::ColoringDefaults::legend_width_pixels*captureScale/std::max(1,renderer->GetSize()[0]));
  const double height = std::min(.7,Postprocessor::ColoringDefaults::legend_height_pixels*captureScale/std::max(1,renderer->GetSize()[1]));
  double x = std::max(.02,.98-width-(width+.02)*slot),y = .06;
  if (position)
  {
    x = (*position)[0];
    y = (*position)[1];
  }
  legend->SetWidth(width);
  legend->SetHeight(height);
  legend->SetPosition(std::clamp(x,0.0,1.0-width),std::clamp(y,0.0,1.0-height));
  const auto* background = renderer->GetBackground();
  const auto* top = background;
  if (renderer->GetGradientBackground())
  {
    top = renderer->GetBackground2();
  }
  double ink = 1;
  if ((background[0]+background[1]+background[2]+top[0]+top[1]+top[2])/6 > .5)
  {
    ink = 0;
  }
  for (auto* text : {legend->GetLabelTextProperty(),legend->GetTitleTextProperty()})
  {
    text->SetColor(ink,ink,ink);
    text->SetOpacity(1);
    text->SetFontSize(static_cast<int>(std::max(1.0,std::round(Postprocessor::ColoringDefaults::legend_font_pixels*captureScale))));
    text->SetBackgroundColor(1-ink,1-ink,1-ink);
    double opacity = 0;
    if (renderer->GetGradientBackground())
    {
      opacity = .75;
    }
    text->SetBackgroundOpacity(opacity);
  }
}
