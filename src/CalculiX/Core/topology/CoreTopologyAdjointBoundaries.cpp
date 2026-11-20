#include "CoreTopologyAdjointBoundaries.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreHBCs.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "CubitInterface.hpp"
#include "Core/topology/CoreTopologyConsoleMessages.hpp"
#include <cmath>
#include <cstddef>

bool CoreTopologyAdjointBoundaries::collect(CoreBCsDisplacements& displacements, const CoreHBCs& hbcs, CoreSteps& steps, MeshExportInterface& mesh, std::set<std::pair<int, int>>& boundaries)
{
  boundaries.clear();
  std::set<std::pair<int, int>> collected;
  CoreMeshQuery mesh_query(mesh);

  const auto collect_bc = [&](int bc_id) -> bool
  {
    const int index = displacements.get_bcs_data_id_from_bc_id(bc_id);

    if (index < 0 || displacements.bcs_data[index].size() < 7)
    {
      TOPOLOGY_ERROR("Missing displacement BC %d.\n", bc_id);
      return false;
    }

    if (displacements.bcs_data[index][1] != 0)
    {
      TOPOLOGY_ERROR("Adjoint export does not yet support OP=NEW in displacement BC %d.\n", bc_id);
      return false;
    }

    const int nodeset_id = displacements.bcs_data[index][4];
    const int displacement_id = displacements.bcs_data[index][5];

    if (!mesh_query.check_nodeset_exists(nodeset_id) || CubitInterface::get_nodeset_nodes_inclusive(nodeset_id).empty())
    {
      TOPOLOGY_ERROR("Displacement BC %d has a missing or empty nodeset.\n", bc_id);
      return false;
    }

    const int displacement_data_id = displacements.get_displacement_data_id_from_displacement_id(displacement_id);
    const int mode_data_id = displacements.get_mode_data_id_from_displacement_id(displacement_id);

    if (displacement_data_id < 0 || mode_data_id < 0)
    {
      TOPOLOGY_ERROR("Missing displacement data for BC %d.\n", bc_id);
      return false;
    }

    if (displacements.displacement_data[displacement_data_id].size() < 7 ||
        displacements.mode_data[mode_data_id].size() < 7)
    {
      TOPOLOGY_ERROR("Invalid displacement data for BC %d.\n", bc_id);
      return false;
    }

    CalculiXDofUnits::Boundaries mapped;
    std::string error;
    if (!displacements.get_solver_boundaries(bc_id, mapped, error))
    {
      TOPOLOGY_ERROR("Cannot resolve adjoint boundary %d: %s\n", bc_id, error.c_str());
      return false;
    }
    for (int dof = 1; dof <= 6; ++dof)
    {
      const int mode = displacements.mode_data[mode_data_id][dof];

      if (mode < 0 || mode > 2)
      {
        TOPOLOGY_ERROR("Invalid displacement mode for BC %d, DOF %d.\n", bc_id, dof);
        return false;
      }

      if (mode == 1)
      {
        const double magnitude = displacements.displacement_data[displacement_data_id][dof];

        if (!std::isfinite(magnitude))
        {
          TOPOLOGY_ERROR("Invalid displacement magnitude for BC %d, DOF %d.\n", bc_id, dof);
          return false;
        }
      }

    }
    for (const auto& boundary : mapped)
    {
      collected.insert(boundary.first);
    }

    return true;
  };

  for (const auto& bc : hbcs.bcs_data)
  {
    if (bc.size() < 3 || bc[1] != 1 || !collect_bc(bc[2]))
    {
      TOPOLOGY_ERROR("Unable to collect model-level adjoint boundary conditions.\n");
      return false;
    }
  }

  for (const auto& step : steps.steps_data)
  {
    if (step.size() < 7)
    {
      TOPOLOGY_ERROR("Invalid step boundary assignment.\n");
      return false;
    }

    for (int index : steps.get_bc_data_ids_from_bcs_id(step[6]))
    {
      if (index < 0 || static_cast<std::size_t>(index) >= steps.bcs_data.size())
      {
        TOPOLOGY_ERROR("Invalid step boundary assignment.\n");
        return false;
      }

      const auto& bc = steps.bcs_data[index];

      if (bc.size() < 3 || bc[1] != 1 || !collect_bc(bc[2]))
      {
        TOPOLOGY_ERROR("Unable to collect adjoint boundary conditions for step %d.\n", step[0]);
        return false;
      }
    }
  }

  boundaries.swap(collected);
  return true;
}
