#include "CoreBCRemoval.hpp"
#include "CoreBCsDisplacements.hpp"
#include "CoreBCsTemperatures.hpp"
#include "CoreHBCs.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/steps/CoreInitialConditions.hpp"

namespace
{
  template<typename BCType>
  bool delete_bc_and_references(
    BCType& bcs, CoreSteps& steps, CoreHBCs& hbcs,
    CoreInitialConditions& conditions, int bc_id, int bc_type)
  {
    if (!bcs.delete_bc(bc_id))
    {
      return false;
    }

    steps.remove_bc_references(bc_type, bc_id);
    hbcs.remove_bc_references(bc_type, bc_id);
    conditions.clear_bc_references(bc_type, bc_id);
    return true;
  }
}

bool CoreBCRemoval::delete_bc(
  CoreBCsDisplacements& bcs, CoreSteps& steps, CoreHBCs& hbcs,
  CoreInitialConditions& conditions, int bc_id)
{
  return delete_bc_and_references(bcs, steps, hbcs, conditions, bc_id, 1);
}

bool CoreBCRemoval::delete_bc(
  CoreBCsTemperatures& bcs, CoreSteps& steps, CoreHBCs& hbcs,
  CoreInitialConditions& conditions, int bc_id)
{
  return delete_bc_and_references(bcs, steps, hbcs, conditions, bc_id, 2);
}
