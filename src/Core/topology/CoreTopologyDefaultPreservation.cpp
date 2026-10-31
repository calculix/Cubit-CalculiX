#include "CoreTopologyDefaultPreservation.hpp"
#include "CoreTopologyPreservation.hpp"
#include "CoreTopologyInteractionPreservation.hpp"
#include "CoreTopologyLoadPreservation.hpp"
#include "CoreTopologyBCPreservation.hpp"
#include "CoreTopologyTrajectoryPreservation.hpp"
#include "CoreTopologyGeometry.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "CubitMessage.hpp"

bool CoreTopologyDefaultPreservation::collect(const CoreTopologyPreservationData& data, std::map<int, std::set<int>>& default_sets, std::set<int>& default_elements)
{
  default_sets.clear();
  default_elements.clear();

  std::map<int, std::set<int>> collected;
  std::set<int> collected_elements;
  std::set<int> collected_nodes;
  std::map<int, std::set<int>> used_loads;
  std::map<int, std::set<int>> used_bcs;

  if (!CoreTopologyPreservation::collect_step_references(data.steps, used_loads, used_bcs))
  {
    return false;
  }

  if (!CoreTopologyInteractionPreservation::append_constraints(data.constraints, collected, collected_nodes))
  {
    return false;
  }

  if (!CoreTopologyInteractionPreservation::append_contact_pairs(data.contacts, collected))
  {
    return false;
  }

  if (!CoreTopologyLoadPreservation::append_set_loads(data.gravity.loads_data, used_loads[4], 1, "gravity", collected) ||
      !CoreTopologyLoadPreservation::append_set_loads(data.centrifugal.loads_data, used_loads[5], 1, "centrifugal", collected))
  {
    return false;
  }

  if (!CoreTopologyLoadPreservation::append_set_loads(data.film.loads_data, used_loads[7], 3, "film", collected) ||
      !CoreTopologyLoadPreservation::append_set_loads(data.radiation.loads_data, used_loads[8], 3, "radiation", collected) ||
      !CoreTopologyLoadPreservation::append_set_loads(data.traction.loads_data, used_loads[9], 3, "surface traction", collected))
  {
    return false;
  }

  if (!CoreTopologyLoadPreservation::append_body_heatflux(data.bodyheatflux, used_loads[10], collected, collected_elements))
  {
    return false;
  }

  if (!CoreTopologyTrajectoryPreservation::append(data.trajectories, data.blocks, data.mesh, used_loads[6], collected_elements))
  {
    return false;
  }

  if (!CoreTopologyLoadPreservation::append_loads(data.forces, data.mesh, used_loads[1], collected) ||
      !CoreTopologyLoadPreservation::append_loads(data.pressures, data.mesh, used_loads[2], collected) ||
      !CoreTopologyLoadPreservation::append_loads(data.heatfluxes, data.mesh, used_loads[3], collected))
  {
    return false;
  }

  if (!CoreTopologyBCPreservation::append_bcs(data.displacements, data.mesh, used_bcs[1], collected) ||
      !CoreTopologyBCPreservation::append_bcs(data.temperatures, data.mesh, used_bcs[2], collected))
  {
    return false;
  }

  if (!collected_nodes.empty())
  {
    if (!data.blocks)
    {
      PRINT_ERROR("Block core is not initialized.\n");
      return false;
    }

    if (!CoreTopologyGeometry::append_node_elements(*data.blocks, data.mesh, collected_nodes, collected_elements))
    {
      return false;
    }
  }

  default_sets.swap(collected);
  default_elements.swap(collected_elements);
  return true;
}
