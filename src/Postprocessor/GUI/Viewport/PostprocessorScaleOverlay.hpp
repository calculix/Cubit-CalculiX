#pragma once
#include "Framework/Core/units/WorkspaceUnitContext.hpp"
#include <vtkSmartPointer.h>
#include <optional>
#include <string>
class vtkRenderer;
class vtkActor2D;
class vtkTextActor;

// A screen-space length sampled in the camera focal plane. Perspective lengths
// are local to that plane, never a distance valid throughout the scene.
class PostprocessorScaleOverlay
{
public:
  explicit PostprocessorScaleOverlay(vtkRenderer* renderer);
  ~PostprocessorScaleOverlay();
  void configure(bool visible,const std::string& unit,const SimUnits::WorkspaceUnitContext* presentation);
  void update();
  void set_capture_scale(double value);
private:
  vtkRenderer* renderer;
  vtkSmartPointer<vtkActor2D> line;
  vtkSmartPointer<vtkTextActor> text;
  bool visible = false;
  std::string unit;
  double captureScale = 1;
  std::optional<SimUnits::WorkspaceUnitContext> presentation;
};
