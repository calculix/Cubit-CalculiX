#pragma once
#include "Postprocessor/Core/representations/VectorPlotRuntime.hpp"
#include <vtkSmartPointer.h>
class vtkRenderer;
class vtkActor;
class vtkScalarBarActor;
class PostprocessorVectorActors
{
public:
  PostprocessorVectorActors(vtkRenderer* renderer,const Postprocessor::VectorPlotData& data,
    const Postprocessor::VectorPlotDefinition& settings,double sceneScale,unsigned int legendSlot,bool showLegend);
  ~PostprocessorVectorActors();
  void scale_capture(double scale);
private:
  vtkSmartPointer<vtkRenderer> renderer;
  vtkSmartPointer<vtkActor> actor;
  vtkSmartPointer<vtkScalarBarActor> legend;
};
