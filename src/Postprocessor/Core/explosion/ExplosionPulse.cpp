#include "ExplosionPulseGeometry.hpp"
#include "../filters/FilterRuntime.hpp"
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <cmath>
namespace Postprocessor
{
  double explosion_pulse_factor(const ExplosionState::Pulse& pulse)
  {
    if (!pulse.enabled)
    {
      return 1.0;
    }
    constexpr double pi = 3.14159265358979323846;
    const double phase = std::fmod(pulse.elapsed_seconds,pulse.period_seconds)/pulse.period_seconds;
    const double weight = 0.5+0.5*std::cos(2.0*pi*phase);
    return pulse.minimum+(pulse.maximum-pulse.minimum)*weight;
  }
  void apply_explosion_pulse_mesh(vtkDataSet* mesh,const ExplosionState::Pulse& pulse,ExplosionPointBuffers& updated)
  {
    const double factor = explosion_pulse_factor(pulse);
    double components = 1.0, elements = 1.0;
    if (pulse.target != ExplosionState::Pulse::Target::Mesh)
    {
      components = factor;
    }
    if (pulse.target != ExplosionState::Pulse::Target::Components)
    {
      elements = factor;
    }
    auto* points = vtkPointSet::SafeDownCast(mesh);
    if (!points || !points->GetPoints())
    {
      return;
    }
    auto* base = points->GetPointData()->GetArray("PostprocessorPulseBase");
    auto* component = points->GetPointData()->GetArray("PostprocessorPulseComponents");
    auto* element = points->GetPointData()->GetArray("PostprocessorPulseMesh");
    const auto count = points->GetNumberOfPoints();
    if (!base || !component || !element || base->GetNumberOfTuples() != count ||
        component->GetNumberOfTuples() != count || element->GetNumberOfTuples() != count ||
        base->GetNumberOfComponents() != 3 || component->GetNumberOfComponents() != 3 || element->GetNumberOfComponents() != 3)
    {
      return;
    }
    if (!updated.insert(points->GetPoints()->GetData()).second)
    {
      return;
    }
    for (vtkIdType i = 0; i < count; ++i)
    {
      double position[3];
      for (int axis = 0; axis < 3; ++axis)
      {
        position[axis] = base->GetComponent(i,axis)+components*component->GetComponent(i,axis)+elements*element->GetComponent(i,axis);
      }
      points->GetPoints()->SetPoint(i,position);
    }
    points->GetPoints()->Modified();
    points->Modified();
  }
  namespace
  {
    template<class Visit>
    void visit_pulse_meshes(const EvaluatedOutput& output,Visit visit)
    {
      for (const auto& part : output.parts)
      {
        visit(part.mesh.get());
        visit(part.removed.get());
        visit(part.nodes.get());
        visit(part.edges.get());
        visit(part.reference_edges.get());
      }
    }
  }

  void remember_explosion_pulse_buffers(const EvaluatedOutput& output,ExplosionPointBuffers& updated)
  {
    // Core already advanced these meshes. Their actor aliases see the same
    // coordinates through the shared array, including its modified timestamp.
    visit_pulse_meshes(output,[&updated](vtkDataSet* mesh)
    {
      auto* pointSet = vtkPointSet::SafeDownCast(mesh);
      if (pointSet && pointSet->GetPoints())
      {
        updated.insert(pointSet->GetPoints()->GetData());
      }
    });
  }

  void apply_explosion_pulse(const EvaluatedOutput& output,const ExplosionState::Pulse& pulse)
  {
    if (!output.exploded)
    {
      return;
    }
    ExplosionPointBuffers updated;
    visit_pulse_meshes(output,[&](vtkDataSet* mesh)
    {
      apply_explosion_pulse_mesh(mesh,pulse,updated);
    });
  }
}
