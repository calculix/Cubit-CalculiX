#include "PostprocessorScaleOverlay.hpp"
#include "Postprocessor/Core/views/ScaleDefinition.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkNew.h>
#include <vtkOpenGLPolyDataMapper2D.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <algorithm>
#include <array>
#include <locale>
#include <cmath>
#include <iomanip>
#include <sstream>

PostprocessorScaleOverlay::PostprocessorScaleOverlay(vtkRenderer* value) : renderer(value)
{
  line = vtkSmartPointer<vtkActor2D>::New();
  text = vtkSmartPointer<vtkTextActor>::New();
  line->PickableOff();
  line->UseBoundsOff();
  text->PickableOff();
  text->UseBoundsOff();
  line->VisibilityOff();
  text->VisibilityOff();
  text->GetTextProperty()->SetJustificationToCentered();
}
PostprocessorScaleOverlay::~PostprocessorScaleOverlay()
{
  renderer->RemoveViewProp(line);
  renderer->RemoveViewProp(text);
}
void PostprocessorScaleOverlay::configure(bool enabled,const std::string& lengthUnit,const SimUnits::WorkspaceUnitContext* context)
{
  visible = enabled;
  unit = lengthUnit;
  presentation.reset();
  if (context)
  {
    presentation = *context;
  }
  update();
}
void PostprocessorScaleOverlay::set_capture_scale(double value)
{
  if (std::isfinite(value) && value > 0)
  {
    captureScale = value;
  }
  update();
}
void PostprocessorScaleOverlay::update()
{
  line->VisibilityOff();
  text->VisibilityOff();
  const auto* size = renderer->GetSize();
  if (!visible || unit.empty() || size[0] < 80 || size[1] < 70)
  {
    return;
  }
  auto* camera = renderer->GetActiveCamera();
  const auto* focus = camera->GetFocalPoint();
  renderer->SetWorldPoint(focus[0],focus[1],focus[2],1);
  renderer->WorldToDisplay();
  const double depth = renderer->GetDisplayPoint()[2];
  const double width = std::min(Postprocessor::ScaleDefaults::width_pixels*captureScale,size[0]*0.4);
  std::array<std::array<double,3>,2> ends;
  for (int i = 0; i < 2; ++i)
  {
    renderer->SetDisplayPoint(size[0]*0.5+i*width,size[1]*0.5,depth);
    renderer->DisplayToWorld();
    const auto* point = renderer->GetWorldPoint();
    if (!std::isfinite(point[3]) || point[3] == 0)
    {
      return;
    }
    for (int axis = 0; axis < 3; ++axis)
    {
      ends[i][axis] = point[axis]/point[3];
    }
  }
  const double length = std::hypot(ends[1][0]-ends[0][0],ends[1][1]-ends[0][1],ends[1][2]-ends[0][2]);
  const SimUnits::QuantityUnit quantity{"length",unit,SimUnits::Length};
  double displayed = 0;
  std::string label, error;
  const SimUnits::WorkspaceUnitContext* context = nullptr;
  if (presentation)
  {
    context = &*presentation;
  }
  if (!SimResults::ResultsUnits::display_value(length,quantity,"postprocessor",displayed,label,error,context) || label.empty())
  {
    return;
  }
  const double interval = Postprocessor::scale_interval(displayed);
  if (interval <= 0 || !std::isfinite(interval))
  {
    return;
  }
  const double pixels = width*interval/displayed;
  const double left = (size[0]-pixels)*0.5;
  const double right = left+pixels;
  const double baseline = 18*captureScale;
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(left,baseline,0);
  points->InsertNextPoint(right,baseline,0);
  points->InsertNextPoint(left,baseline-4*captureScale,0);
  points->InsertNextPoint(left,baseline+4*captureScale,0);
  points->InsertNextPoint(right,baseline-4*captureScale,0);
  points->InsertNextPoint(right,baseline+4*captureScale,0);
  vtkNew<vtkCellArray> lines;
  for (vtkIdType i = 0; i < 6; i += 2)
  {
    const vtkIdType ids[] = {i,i+1};
    lines->InsertNextCell(2,ids);
  }
  vtkNew<vtkPolyData> geometry;
  geometry->SetPoints(points);
  geometry->SetLines(lines);
  vtkNew<vtkOpenGLPolyDataMapper2D> mapper;
  mapper->SetInputData(geometry);
  line->SetMapper(mapper);
  const auto* background = renderer->GetBackground();
  double ink = 1;
  if (0.2126*background[0]+0.7152*background[1]+0.0722*background[2] > 0.5)
  {
    ink = 0;
  }
  line->GetProperty()->SetColor(ink,ink,ink);
  line->GetProperty()->SetLineWidth(static_cast<float>(2*captureScale));
  auto* style = text->GetTextProperty();
  style->SetColor(ink,ink,ink);
  style->SetBackgroundColor(background);
  style->SetBackgroundOpacity(0.85);
  style->SetFontSize(static_cast<int>(12*captureScale));
  std::ostringstream caption;
  caption.imbue(std::locale::classic());
  caption << std::setprecision(4) << interval << " " << label;
  if (!camera->GetParallelProjection())
  {
    caption << " (at focus)";
  }
  text->SetInput(caption.str().c_str());
  text->SetPosition(size[0]*0.5,baseline+7*captureScale);
  if (!renderer->HasViewProp(line))
  {
    renderer->AddViewProp(line);
  }
  if (!renderer->HasViewProp(text))
  {
    renderer->AddViewProp(text);
  }
  line->VisibilityOn();
  text->VisibilityOn();
}
