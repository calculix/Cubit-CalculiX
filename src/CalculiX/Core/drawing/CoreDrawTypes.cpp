#include "CoreDrawTypes.hpp"

DrawGroup drawing_group(DrawFeature feature)
{
  switch (feature)
  {
    case DrawFeature::Displacement:
    case DrawFeature::Temperature: return DrawGroup::BoundaryConditions;
    case DrawFeature::Orientation: return DrawGroup::Orientations;
    case DrawFeature::Equation:
    case DrawFeature::EquationGroup: return DrawGroup::Constraints;
    case DrawFeature::Topology: return DrawGroup::Topology;
    default: return DrawGroup::Loads;
  }
}

const char* drawing_group_name(DrawGroup group)
{
  switch (group)
  {
    case DrawGroup::BoundaryConditions: return "ccx_bcs";
    case DrawGroup::Loads: return "ccx_loads";
    case DrawGroup::Constraints: return "ccx_constraints";
    case DrawGroup::Orientations: return "ccx_orientations";
    case DrawGroup::Topology: return "ccx_topology";
  }
  return "ccx_drawing";
}
