#include "Core/CalculiXCore.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyModelPreparation.hpp"
#include "CoreTopologyDefaultPreservation.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::collect_topology_run_sections(int topology_id, std::vector<int>& section_ids, std::set<int>& block_ids)
{
  section_ids.clear();
  block_ids.clear();

  if (!topology || !mat || !sections || !jobs || !steps || !historyoutputs || !me_iface)
  {
    PRINT_ERROR("Topology run cores are not initialized.\n");
    return false;
  }

  if (topology->topology_cases.find(topology_id) == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (!validate_topology_compliance_model())
  {
    return false;
  }

  return CoreTopologyModelPreparation::collect_run_sections(*sections, *cb, section_ids, block_ids);
}

bool CalculiXCore::initialize_topology_run_design(int topology_id, const std::vector<int>& section_ids)
{
  if (!topology || !mat || !sections)
  {
    PRINT_ERROR("Topology initialization cores are not initialized.\n");
    return false;
  }

  if (!cb || !me_iface)
  {
    PRINT_ERROR("Topology initialization interfaces are not initialized.\n");
    return false;
  }

  if (!constraints || !contactpairs || !steps ||
    !loadsforces || !loadspressures || !loadsheatfluxes ||
    !bcsdisplacements || !bcstemperatures ||
    !loadsgravity || !loadscentrifugal ||
    !loadsfilm || !loadsradiation || !loadssurfacetraction || !loadsbodyheatflux)
  {
    PRINT_ERROR("Topology preservation cores are not initialized.\n");
    return false;
  }

  const CoreTopologyPreservationData preservation = {
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

  return CoreTopologyModelPreparation::initialize_run_design(*topology, *mat, *sections, preservation, topology_id, section_ids);
}

bool CalculiXCore::initialize_topology(int topology_id, const std::vector<int>& section_ids)
{
  if (!mat || !sections || !cb || !me_iface)
  {
    PRINT_ERROR("Topology initialization interfaces are not initialized.\n");
    return false;
  }

  return CoreTopologyModelPreparation::initialize(*mat, *sections, *cb, *me_iface, topology_id, section_ids);
}
