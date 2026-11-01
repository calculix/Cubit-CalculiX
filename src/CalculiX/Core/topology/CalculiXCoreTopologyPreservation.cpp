#include "Core/CalculiXCore.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyPreservation.hpp"
#include "CoreTopologyDefaultPreservation.hpp"
#include "CoreTopologyModelPreparation.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::reset_preserved_sets(int topology_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->reset_preserved_sets(topology_id);
}

bool CalculiXCore::add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->add_preserved_sets(topology_id, set_type, ids);
}

bool CalculiXCore::remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->remove_preserved_sets(topology_id, set_type, ids);
}

std::vector<std::vector<int>> CalculiXCore::get_topology_element_preservation(int topology_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return {};
  }

  return topology->get_topology_element_preservation(topology_id);
}

bool CalculiXCore::get_topology_preserved_sets(int topology_id, std::vector<TopologyPreservedSetInfo>& sets, std::set<int>& direct_elements)
{
  sets.clear();
  direct_elements.clear();

  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  std::map<int, std::set<int>> defaults;
  std::set<int> collected_direct_elements;

  if (!collect_topology_default_sets(defaults, collected_direct_elements))
  {
    return false;
  }

  return CoreTopologyPreservation::build_set_rows(entry->second, defaults, collected_direct_elements, sets, direct_elements);
}

bool CalculiXCore::get_topology_set_element_ids(int topology_id, int set_type, int set_id, std::vector<int>& element_ids)
{
  element_ids.clear();

  if (!mat || !me_iface)
  {
    PRINT_ERROR("Topology mesh/material interfaces are not initialized.\n");
    return false;
  }

  if (!cb)
  {
    PRINT_ERROR("Block core is not initialized.\n");
    return false;
  }

  return CoreTopologyPreservation::get_set_element_ids(*mat, *cb, *me_iface, topology_id, set_type, set_id, element_ids);
}

bool CalculiXCore::collect_topology_default_sets(std::map<int, std::set<int>>& default_sets, std::set<int>& default_elements)
{
  default_sets.clear();
  default_elements.clear();

  if (!constraints || !contactpairs || !me_iface || !steps ||
    !loadsforces || !loadspressures || !loadsheatfluxes ||
    !bcsdisplacements || !bcstemperatures ||
    !loadsgravity || !loadscentrifugal ||
    !loadsfilm || !loadsradiation || !loadssurfacetraction || !loadsbodyheatflux)
  {
    PRINT_ERROR("Topology preservation cores are not initialized.\n");
    return false;
  }

  const CoreTopologyPreservationData data = {
    *steps,
    *constraints,
    *contactpairs,
    *loadsforces,
    *loadspressures,
    *loadsheatfluxes,
    *loadsgravity,
    *loadscentrifugal,
    *loadsfilm,
    *loadsradiation,
    *loadssurfacetraction,
    *loadsbodyheatflux,
    *bcsdisplacements,
    *bcstemperatures,
    *me_iface,
    cb,
    loadstrajectory
  };

  return CoreTopologyDefaultPreservation::collect(data, default_sets, default_elements);
}

bool CalculiXCore::resolve_topology_preservation(int topology_id, const std::map<int, std::set<int>>& default_sets, const std::set<int>& default_elements)
{
  if (!topology || !mat)
  {
    PRINT_ERROR("Topology/material cores are not initialized.\n");
    return false;
  }

  return CoreTopologyModelPreparation::resolve_preservation(*topology, *mat, topology_id, default_sets, default_elements);
}
