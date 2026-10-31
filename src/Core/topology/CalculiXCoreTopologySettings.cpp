#include "Core/CalculiXCore.hpp"
#include "CoreTopology.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::create_topology(int topology_id, const TopologyOptions& options)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->create_topology(topology_id, options);
}

bool CalculiXCore::modify_topology(int topology_id, const TopologyOptions& options)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->modify_topology(topology_id, options);
}

std::vector<double> CalculiXCore::get_topology_parameters(int topology_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return {};
  }

  return topology->get_topology_parameters(topology_id);
}

bool CalculiXCore::get_topology_settings(int topology_id, TopologyOptions& options)
{
  if (!topology)
  {
    options = {};
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->get_topology_settings(topology_id, options);
}
