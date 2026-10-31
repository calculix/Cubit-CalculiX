#include "CoreTopologyTrajectoryPreservation.hpp"
#include "CoreTopologyGeometry.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsTrajectoryHeatflux.hpp"
#include "Core/loads/CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <map>
#include <vector>

namespace
{
bool append_surface_heatflux(CoreLoadsTrajectoryHeatflux& loads, CoreBlocks* blocks, MeshExportInterface& mesh, int subload_id, std::set<int>& elements)
{
  if (loads.get_loads_data_id_from_load_id(subload_id) < 0)
  {
    PRINT_ERROR("Missing surface heat flux trajectory data.\n");
    return false;
  }

  if (!blocks)
  {
    PRINT_ERROR("Block core is not initialized.\n");
    return false;
  }

  std::map<std::vector<int>, std::set<int>> face_owners;

  if (!CoreTopologyGeometry::collect_face_owners(*blocks, mesh, face_owners))
  {
    return false;
  }

  const auto targets = loads.get_face_ids(subload_id);
  std::set<int> selected_faces;

  for (const auto& position : targets)
  {
    for (const auto& selection : position)
    {
      selected_faces.insert(selection.begin(), selection.end());
    }
  }

  for (int face_id : selected_faces)
  {
    if (face_id <= 0)
    {
      PRINT_ERROR("Invalid trajectory face ID %d.\n", face_id);
      return false;
    }

    // Cubit get_connectivity() returns corner nodes.
    auto key = CubitInterface::get_connectivity("face", face_id);

    if (key.size() != 3 && key.size() != 4)
    {
      PRINT_ERROR("Invalid connectivity for trajectory face %d.\n", face_id);
      return false;
    }

    std::sort(key.begin(), key.end());
    const auto owners = face_owners.find(key);

    if (owners == face_owners.end())
    {
      PRINT_ERROR(
          "Trajectory face %d has no hex, tet or wedge owner "
          "in the analysis blocks.\n",
          face_id);
      return false;
    }

    elements.insert(owners->second.begin(), owners->second.end());
  }

  return true;
}

bool append_body_heatflux_sphere(CoreLoadsTrajectoryBodyHeatfluxSphere& loads, int subload_id, std::set<int>& elements)
{
  if (loads.get_loads_data_id_from_load_id(subload_id) < 0)
  {
    PRINT_ERROR("Missing body heat flux sphere trajectory data.\n");
    return false;
  }

  const auto targets = loads.get_element_ids(subload_id);

  // Targets are grouped by trajectory position and radius/depth.
  for (const auto& position : targets)
  {
    for (const auto& selection : position)
    {
      for (int element_id : selection)
      {
        if (element_id <= 0)
        {
          PRINT_ERROR("Invalid trajectory target element %d.\n", element_id);
          return false;
        }

        elements.insert(element_id);
      }
    }
  }

  return true;
}
}

bool CoreTopologyTrajectoryPreservation::append(CoreLoadsTrajectory* trajectories, CoreBlocks* blocks, MeshExportInterface& mesh, const std::set<int>& trajectory_ids, std::set<int>& elements)
{
  for (int trajectory_id : trajectory_ids)
  {
    if (!trajectories)
    {
      PRINT_ERROR("Trajectory core is not initialized.\n");
      return false;
    }

    const int index = trajectories->get_loads_data_id_from_load_id(trajectory_id);

    if (index < 0 || trajectories->loads_data[index].size() < 3)
    {
      PRINT_ERROR("Missing trajectory load %d.\n", trajectory_id);
      return false;
    }

    const auto& trajectory = trajectories->loads_data[index];
    const int type = trajectory[1];
    const int subload_id = trajectory[2];

    if (type == 1)
    {
      if (!trajectories->heatflux)
      {
        PRINT_ERROR("Missing surface heat flux trajectory data.\n");
        return false;
      }

      if (!append_surface_heatflux(*trajectories->heatflux, blocks, mesh, subload_id, elements))
      {
        return false;
      }
    }
    else
    {
      if (type != 2 || !trajectories->bodyheatfluxsphere)
      {
        PRINT_ERROR("Invalid trajectory load type for %d.\n", trajectory_id);
        return false;
      }

      if (!append_body_heatflux_sphere(*trajectories->bodyheatfluxsphere, subload_id, elements))
      {
        return false;
      }
    }
  }

  return true;
}
