#include "CoreLoadRemoval.hpp"
#include "CoreLoadsForces.hpp"
#include "CoreLoadsPressures.hpp"
#include "CoreLoadsHeatfluxes.hpp"
#include "CoreLoadsGravity.hpp"
#include "CoreLoadsCentrifugal.hpp"
#include "CoreLoadsTrajectory.hpp"
#include "CoreLoadsFilm.hpp"
#include "CoreLoadsRadiation.hpp"
#include "CoreLoadsSurfaceTraction.hpp"
#include "CoreLoadsBodyHeatflux.hpp"
#include "Core/steps/CoreSteps.hpp"

namespace
{
  template<typename LoadType>
  bool delete_load_and_references(LoadType& loads, CoreSteps& steps, int load_id, int load_type)
  {
    if (!loads.delete_load(load_id))
    {
      return false;
    }

    steps.remove_load_references(load_type, load_id);
    return true;
  }
}

bool CoreLoadRemoval::delete_load(CoreLoadsForces& loads, CoreSteps& steps, int load_id)
{
  return delete_load_and_references(loads, steps, load_id, 1);
}

bool CoreLoadRemoval::delete_load(CoreLoadsPressures& loads, CoreSteps& steps, int load_id)
{
  return delete_load_and_references(loads, steps, load_id, 2);
}

bool CoreLoadRemoval::delete_load(CoreLoadsHeatfluxes& loads, CoreSteps& steps, int load_id)
{
  return delete_load_and_references(loads, steps, load_id, 3);
}

bool CoreLoadRemoval::delete_load(CoreLoadsGravity& loads, CoreSteps& steps, int load_id)
{
  return delete_load_and_references(loads, steps, load_id, 4);
}

bool CoreLoadRemoval::delete_load(CoreLoadsCentrifugal& loads, CoreSteps& steps, int load_id)
{
  return delete_load_and_references(loads, steps, load_id, 5);
}

bool CoreLoadRemoval::delete_load(CoreLoadsTrajectory& loads, CoreSteps& steps, int load_id)
{
  return delete_load_and_references(loads, steps, load_id, 6);
}

bool CoreLoadRemoval::delete_load(CoreLoadsFilm& loads, CoreSteps& steps, int load_id)
{
  return delete_load_and_references(loads, steps, load_id, 7);
}

bool CoreLoadRemoval::delete_load(CoreLoadsRadiation& loads, CoreSteps& steps, int load_id)
{
  return delete_load_and_references(loads, steps, load_id, 8);
}

bool CoreLoadRemoval::delete_load(CoreLoadsSurfaceTraction& loads, CoreSteps& steps, int load_id)
{
  return delete_load_and_references(loads, steps, load_id, 9);
}

bool CoreLoadRemoval::delete_load(CoreLoadsBodyHeatflux& loads, CoreSteps& steps, int load_id)
{
  return delete_load_and_references(loads, steps, load_id, 10);
}
