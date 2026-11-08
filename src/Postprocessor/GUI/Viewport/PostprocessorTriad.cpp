#include "PostprocessorTriad.hpp"
#include "Postprocessor/Core/representations/DisplayDefaults.hpp"
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkOpenGLActor.h>
#include <vtkOpenGLCamera.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLProperty.h>
#include <vtkOpenGLRenderer.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkUnsignedCharArray.h>
#include <algorithm>
#include <cmath>

PostprocessorTriad::PostprocessorTriad(vtkRenderWindow* host) : window(host)
{
  overlay = vtkSmartPointer<vtkOpenGLRenderer>::New();
  overlay->SetLayer(1);
  overlay->InteractiveOff();
  auto camera = vtkSmartPointer<vtkOpenGLCamera>::New();
  camera->ParallelProjectionOn();
  camera->SetPosition(0, 0, 4);
  camera->SetFocalPoint(0, 0, 0);
  camera->SetParallelScale(1.4);
  overlay->SetActiveCamera(camera);
  geometry = vtkSmartPointer<vtkPolyData>::New();
  points = vtkSmartPointer<vtkPoints>::New();
  points->Allocate(64);
  lines = vtkSmartPointer<vtkCellArray>::New();
  lines->AllocateEstimate(32, 2);
  colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  colors->SetNumberOfComponents(3);
  colors->Allocate(96);
  auto mapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
  mapper->SetInputData(geometry);
  mapper->SetScalarModeToUseCellData();
  mapper->SetColorModeToDirectScalars();
  auto actor = vtkSmartPointer<vtkOpenGLActor>::New();
  actor->SetMapper(mapper);
  auto property = vtkSmartPointer<vtkOpenGLProperty>::New();
  property->SetLineWidth(2);
  property->LightingOff();
  actor->SetProperty(property);
  overlay->AddActor(actor);
  window->SetNumberOfLayers(std::max(2, window->GetNumberOfLayers()));
  window->AddRenderer(overlay);
}

PostprocessorTriad::~PostprocessorTriad()
{
  window->RemoveRenderer(overlay);
}

void PostprocessorTriad::update(vtkCamera* camera, bool visible,
  const std::array<double, 3>& background, int width, int height, double pixel_ratio)
{
  overlay->SetDraw(visible);
  if (!visible || width <= 0 || height <= 0)
  {
    return;
  }
  const double extent = std::min({double(width), double(height),
    Postprocessor::DisplayDefaults::triad_size_pixels * pixel_ratio});
  overlay->SetViewport(0, 0, extent / width, extent / height);
  double direction[3];
  camera->GetDirectionOfProjection(direction);
  const auto* viewUp = camera->GetViewUp();
  const std::array<double, 3> currentDirection = {direction[0], direction[1], direction[2]};
  const std::array<double, 3> currentUp = {viewUp[0], viewUp[1], viewUp[2]};
  if (initialized && currentDirection == lastDirection && currentUp == lastUp && background == lastBackground &&
      width == lastWidth && height == lastHeight && pixel_ratio == lastPixelRatio)
  {
    return;
  }
  std::array<double, 3> right =
  {
    direction[1] * viewUp[2] - direction[2] * viewUp[1],
    direction[2] * viewUp[0] - direction[0] * viewUp[2],
    direction[0] * viewUp[1] - direction[1] * viewUp[0]
  };
  const double norm = std::hypot(right[0], right[1], right[2]);
  if (norm <= 0)
  {
    return;
  }
  for (auto& component : right)
  {
    component /= norm;
  }
  const std::array<double, 3> up =
  {
    right[1] * direction[2] - right[2] * direction[1],
    right[2] * direction[0] - right[0] * direction[2],
    right[0] * direction[1] - right[1] * direction[0]
  };
  points->Reset();
  lines->Reset();
  colors->Reset();
  const auto line = [&](double x1, double y1, double x2, double y2, const std::array<unsigned char, 3>& color)
  {
    const vtkIdType ids[] = {points->InsertNextPoint(x1, y1, 0), points->InsertNextPoint(x2, y2, 0)};
    lines->InsertNextCell(2, ids);
    colors->InsertNextTypedTuple(color.data());
  };
  const std::array<unsigned char, 3> axisColors[] = {{220, 35, 35}, {30, 180, 45}, {40, 90, 235}};
  const unsigned char ink = background[0] * 0.2126 + background[1] * 0.7152 + background[2] * 0.0722 > 0.5 ? 20 : 245;
  const std::array<unsigned char, 3> labelColor = {ink, ink, ink};
  for (std::size_t axis = 0; axis < 3; ++axis)
  {
    const double x = right[axis] * 0.85;
    const double y = up[axis] * 0.85;
    line(0, 0, x, y, axisColors[axis]);
    line(x, y, x * 0.82 - y * 0.09, y * 0.82 + x * 0.09, axisColors[axis]);
    line(x, y, x * 0.82 + y * 0.09, y * 0.82 - x * 0.09, axisColors[axis]);
    const double tx = right[axis] * 1.08;
    const double ty = up[axis] * 1.08;
    // Screen-facing stroke glyphs stay legible without fonts or host resources.
    if (axis == 0)
    {
      line(tx - 0.07, ty - 0.09, tx + 0.07, ty + 0.09, labelColor);
      line(tx - 0.07, ty + 0.09, tx + 0.07, ty - 0.09, labelColor);
    }
    else if (axis == 1)
    {
      line(tx - 0.07, ty + 0.09, tx, ty, labelColor);
      line(tx + 0.07, ty + 0.09, tx, ty, labelColor);
      line(tx, ty, tx, ty - 0.09, labelColor);
    }
    else
    {
      line(tx - 0.07, ty + 0.09, tx + 0.07, ty + 0.09, labelColor);
      line(tx + 0.07, ty + 0.09, tx - 0.07, ty - 0.09, labelColor);
      line(tx - 0.07, ty - 0.09, tx + 0.07, ty - 0.09, labelColor);
    }
  }
  geometry->SetPoints(points);
  geometry->SetLines(lines);
  geometry->GetCellData()->SetScalars(colors);
  // SetPoints/SetLines may receive the same objects after Reset. Explicitly
  // invalidate their content so VTK uploads the modified tuples/connectivity.
  points->Modified();
  lines->Modified();
  colors->Modified();
  geometry->Modified();
  lastDirection = currentDirection;
  lastUp = currentUp;
  lastBackground = background;
  lastWidth = width;
  lastHeight = height;
  lastPixelRatio = pixel_ratio;
  initialized = true;
}
