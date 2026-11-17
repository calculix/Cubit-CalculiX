#include "PostprocessorVectorActors.hpp"
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
  vtkNew<vtkPoints> points;
  points->SetDataTypeToDouble();
  vtkNew<vtkDoubleArray> directions;
  directions->SetName("VectorDirection");
  directions->SetNumberOfComponents(3);
  vtkNew<vtkDoubleArray> lengths;
  lengths->SetName("VectorLength");
  vtkNew<vtkDoubleArray> magnitudes;
  magnitudes->SetName("VectorMagnitude");
  vtkNew<vtkIdTypeArray> datasets;
  datasets->SetName("VectorDatasetId");
  vtkNew<vtkTypeInt64Array> entities;
  entities->SetName("VectorEntityId");
  vtkNew<vtkTypeInt64Array> integrationPoints;
  integrationPoints->SetName("VectorIntegrationPointId");
  for (const auto& sample : data.samples)
  {
    points->InsertNextPoint(sample.position.data());
    directions->InsertNextTuple(sample.direction.data());
    lengths->InsertNextValue(sample.length);
    magnitudes->InsertNextValue(sample.magnitude);
    datasets->InsertNextValue(static_cast<vtkIdType>(sample.dataset));
    entities->InsertNextValue(sample.entity);
    integrationPoints->InsertNextValue(sample.integration_point);
  }
  samples->SetPoints(points);
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
  double maximum = data.reference_magnitude;
  if (!std::isfinite(maximum) || maximum <= 0)
  {
    maximum = 1;
  }
  lookup->SetRange(0,maximum);
  lookup->SetNumberOfTableValues(256);
  lookup->Build();
  for (int index = 0; index < 256; ++index)
  {
    const auto color = Postprocessor::palette_color(Postprocessor::ColoringDefaults::palette,index/255.0);
    lookup->SetTableValue(index,color[0],color[1],color[2],1);
  }
  if (settings.magnitude_color)
  {
    mapper->SetScalarModeToUsePointFieldData();
    mapper->SetColorModeToMapScalars();
    mapper->SelectColorArray("VectorMagnitude");
    mapper->SetLookupTable(lookup);
    mapper->SetScalarRange(0,maximum);
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
  if (settings.magnitude_color && showLegend)
  {
    legend = vtkSmartPointer<vtkScalarBarActor>::New();
    legend->SetLookupTable(lookup);
    std::string title = "Vector magnitude";
    if (!data.unit.empty())
    {
      title += " ["+data.unit+"]";
    }
    legend->SetTitle(title.c_str());
    legend->SetNumberOfLabels(5);
    legend->SetPosition(0.02+0.16*legendSlot,0.08);
    legend->SetWidth(0.13);
    legend->SetHeight(0.35);
    legend->GetLabelTextProperty()->SetColor(0.2,0.2,0.2);
    legend->GetTitleTextProperty()->SetColor(0.2,0.2,0.2);
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
void PostprocessorVectorActors::scale_capture(double scale)
{
  if (legend)
  {
    legend->GetLabelTextProperty()->SetFontSize(static_cast<int>(12*scale));
    legend->GetTitleTextProperty()->SetFontSize(static_cast<int>(12*scale));
  }
}
