#include "CoreDraw.hpp"
#include "CoreDrawMeshGeometry.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <limits>

bool CoreDraw::draw_topology(int topologyId, int iterationId, double threshold)
{
  if (!is_initialized || iterationId < -1 || !std::isfinite(threshold) || threshold < 0 || threshold > 1)
  {
    PRINT_ERROR("Invalid topology drawing request.\n");
    return false;
  }
  CalculiXTopologyInterface topology;
  const auto rows = topology.get_topology_iteration_element_data(topologyId, iterationId);
  if (rows.empty())
  {
    return false;
  }
  DrawGeometry geometry;
  for (const auto& row : rows)
  {
    if (row.size() != 4 || !std::isfinite(row[0]) || std::floor(row[0]) != row[0] || row[0] < 1 || row[0] > static_cast<double>((std::numeric_limits<int>::max)()) || !std::isfinite(row[1]) || row[1] < 0 || row[1] > 1 || (row[3] != 0 && row[3] != 1))
    {
      PRINT_ERROR("Invalid topology preview element data.\n");
      return false;
    }
    const bool removed = row[3] == 0 && row[1] < threshold;
    if (!CoreDrawMeshGeometry::append("element", static_cast<int>(row[0]), removed, removed ? "red" : "green", geometry))
    {
      PRINT_ERROR("Unable to build topology preview element %.0f.\n", row[0]);
      return false;
    }
  }
  if (!rebuilding)
  {
    scene.state.overlayVisible = true;
  }
  scene.clear(DrawGroup::Topology);
  scene.replace({DrawFeature::Topology, topologyId}, std::move(geometry));
  auto& group = scene.state.groups[DrawGroup::Topology];
  group.enabled = true;
  group.initialized = true;
  group.dirty = false;
  group.entities = {{DrawFeature::Topology, topologyId}};
  activeTopologyId = topologyId;
  activeTopologyIteration = iterationId;
  activeTopologyThreshold = threshold;
  return rebuilding || graphics_flush();
}

void CoreDraw::invalidate_topology(int topology_id)
{
  if (activeTopologyId != topology_id)
  {
    return;
  }
  scene.clear(DrawGroup::Topology);
  scene.state.groups[DrawGroup::Topology].enabled = false;
  activeTopologyId = -1;
  activeTopologyIteration = -1;
  graphics_flush();
}

void CoreDraw::topologyPreview(int& caseId, int& iterationId, double& threshold) const
{
  caseId = activeTopologyId;
  iterationId = activeTopologyIteration;
  threshold = activeTopologyThreshold;
}
