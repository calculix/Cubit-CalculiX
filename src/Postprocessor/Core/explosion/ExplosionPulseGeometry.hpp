#pragma once
#include "ExplosionDefinition.hpp"
#include <set>

class vtkDataArray;
class vtkDataSet;

namespace Postprocessor
{
  struct EvaluatedOutput;

  // Updates only privately owned displayed points using cached displacement
  // arrays. Topology, solver arrays and immutable source buffers are unchanged.
  void apply_explosion_pulse(const EvaluatedOutput& output,const ExplosionState::Pulse& pulse);
  // Key by coordinate array because distinct vtkPoints can share storage.
  using ExplosionPointBuffers = std::set<const vtkDataArray*>;
  void remember_explosion_pulse_buffers(const EvaluatedOutput& output,ExplosionPointBuffers& updated);
  void apply_explosion_pulse_mesh(vtkDataSet* mesh,const ExplosionState::Pulse& pulse,ExplosionPointBuffers& updated);
}
