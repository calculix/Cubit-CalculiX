#pragma once
#include "Postprocessor/Core/filters/FilterDefinition.hpp"
#include <vtkSmartPointer.h>
class vtkRenderer;
class vtkActor;
class PostprocessorPlaneOverlay
{
public:
  explicit PostprocessorPlaneOverlay(vtkRenderer* renderer);
  ~PostprocessorPlaneOverlay();
  void update(const Postprocessor::FilterDefinition* filter,double extent);
private:
  vtkRenderer* renderer;
  vtkSmartPointer<vtkActor> actor;
};
