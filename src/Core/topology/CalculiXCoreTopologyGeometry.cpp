#include "Core/CalculiXCore.hpp"
#include "CoreTopologyGeometry.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::collect_topology_element_volumes(int topology_id, std::map<int, double>& element_volumes, std::map<int, std::array<double, 3>>* element_centers)
{
  element_volumes.clear();

  if (element_centers)
  {
    element_centers->clear();
  }

  if (!mat)
  {
    PRINT_ERROR("Material core is not initialized.\n");
    return false;
  }

  if (!cb)
  {
    PRINT_ERROR("Block core is not initialized.\n");
    return false;
  }

  return CoreTopologyGeometry::collect_element_volumes(*mat, *cb, topology_id, element_volumes, element_centers);
}

bool CalculiXCore::collect_topology_integration_weights(int topology_id, std::map<int, std::map<int, double>>& weights)
{
  weights.clear();

  if (!topology || !mat || !cb)
  {
    PRINT_ERROR("Topology integration weight interfaces are not initialized.\n");
    return false;
  }

  return CoreTopologyGeometry::collect_integration_weights(*topology, *mat, *cb, topology_id, weights);
}
