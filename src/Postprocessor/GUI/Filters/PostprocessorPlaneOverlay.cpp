#include "PostprocessorPlaneOverlay.hpp"
#include <vtkRenderer.h>
#include <vtkOpenGLActor.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkNew.h>
#include <algorithm>
#include <cmath>
PostprocessorPlaneOverlay::PostprocessorPlaneOverlay(vtkRenderer* value) : renderer(value)
{
  actor = vtkSmartPointer<vtkOpenGLActor>::New();
  actor->PickableOff();
  actor->UseBoundsOff();
  const auto& color = Postprocessor::FilterDefaults::plane_color;
  actor->GetProperty()->SetColor(color[0],color[1],color[2]);
  actor->GetProperty()->SetLineWidth(Postprocessor::FilterDefaults::plane_line_width);
  actor->GetProperty()->LightingOff();
}
PostprocessorPlaneOverlay::~PostprocessorPlaneOverlay()
{
  renderer->RemoveActor(actor);
}
void PostprocessorPlaneOverlay::update(const Postprocessor::FilterDefinition* filter,double extent)
{
  actor->SetVisibility(filter != nullptr && filter->enabled && filter->kind != Postprocessor::FilterKind::Threshold);
  if (!filter || !filter->enabled || filter->kind == Postprocessor::FilterKind::Threshold)
  {
    return;
  }
  if (!renderer->HasViewProp(actor))
  {
    renderer->AddActor(actor);
  }
  const auto& normal = filter->plane.normal;
  const int axis = [&]() -> int {
    if (std::abs(normal[0]) < 0.8) {
      return 0;
    } else {
      return 1;
    }
  }();
  std::array<double,3> u = {0,normal[2],-normal[1]};
  if (axis == 1)
  {
    u = {-normal[2],0,normal[0]};
  }
  const double length = std::hypot(u[0],u[1],u[2]);
  for (auto& value : u)
  {
    value /= length;
  }
  const std::array<double,3> v = {normal[1]*u[2]-normal[2]*u[1],normal[2]*u[0]-normal[0]*u[2],normal[0]*u[1]-normal[1]*u[0]};
  const double size = std::max(1.0,extent)*0.4;
  vtkNew<vtkPoints> points;
  points->SetDataTypeToDouble();
  const std::array<std::array<int,2>,4> corners = {{{-1,-1},{1,-1},{1,1},{-1,1}}};
  for (const auto& signs : corners)
  {
    std::array<double,3> point;
    for (int i = 0; i < 3; ++i)
    {
      point[i] = filter->plane.origin[i]+size*(signs[0]*u[i]+signs[1]*v[i]);
    }
    points->InsertNextPoint(point.data());
  }
  points->InsertNextPoint(filter->plane.origin.data());
  points->InsertNextPoint(filter->plane.origin[0]+size*normal[0],filter->plane.origin[1]+size*normal[1],filter->plane.origin[2]+size*normal[2]);
  vtkNew<vtkCellArray> lines;
  const vtkIdType border[] = {0,1,2,3,0};
  const vtkIdType arrow[] = {4,5};
  lines->InsertNextCell(5,border);
  lines->InsertNextCell(2,arrow);
  vtkNew<vtkPolyData> geometry;
  geometry->SetPoints(points);
  geometry->SetLines(lines);
  vtkNew<vtkOpenGLPolyDataMapper> mapper;
  mapper->SetInputData(geometry);
  actor->SetMapper(mapper);
}
