#pragma once
#include "Postprocessor/Core/representations/VectorPlotRuntime.hpp"
#include <vtkSmartPointer.h>
class vtkRenderer;
class vtkActor;
class vtkScalarBarActor;
class vtkPolyData;
namespace Postprocessor
{
  struct ExplosionState;
}
class PostprocessorVectorActors
{
public:
  PostprocessorVectorActors(vtkRenderer* renderer,const Postprocessor::VectorPlotData& data,
    const Postprocessor::VectorPlotDefinition& settings,double sceneScale,unsigned int legendSlot,bool showLegend);
  ~PostprocessorVectorActors();
  void scale_capture(double scale);
  void update_appearance();
  std::array<double,4> legend_frame() const;
  bool legend_contains(double x,double y) const;
  void apply_pulse(const Postprocessor::ExplosionState& settings);
private:
  vtkSmartPointer<vtkPolyData> pulseSamples;
  std::optional<std::array<double,2>> position;
  unsigned int slot = 0;
  double captureScale = 1;
  vtkSmartPointer<vtkRenderer> renderer;
  vtkSmartPointer<vtkActor> actor;
  vtkSmartPointer<vtkScalarBarActor> legend;
};
