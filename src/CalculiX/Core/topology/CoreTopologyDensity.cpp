#include "CoreTopologyDensity.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyGeometry.hpp"
#include "CoreTopologyIteration.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <vector>

bool CoreTopologyDensity::set_densities(CoreTopology& topology, CoreMaterials& materials, CoreBlocks& blocks, int topology_id, int iteration_id, const std::map<int, double>& densities, int source_iteration)
{
  if (source_iteration < -1 || source_iteration == 0 || source_iteration >= iteration_id)
  {
    PRINT_ERROR("Invalid source iteration for topology density storage.\n");
    return false;
  }

  TopologyDensityDesign design;
  std::map<int, double> factors;

  if (!topology.build_density_design(topology_id, iteration_id, densities, design, factors))
  {
    return false;
  }

  if (source_iteration > 0)
  {
    if (!topology.set_density_update_diagnostics(topology_id, source_iteration, design))
    {
      return false;
    }
  }

  std::map<int, double> element_volumes;

  if (!CoreTopologyGeometry::collect_element_volumes(materials, blocks, topology_id, element_volumes))
  {
    return false;
  }

  std::vector<int> element_ids;
  std::vector<double> stiffness_factors;

  for (const auto& factor : factors)
  {
    element_ids.push_back(factor.first);
    stiffness_factors.push_back(factor.second);
  }

  if (!CoreTopologyIteration::set_factors(topology, materials, topology_id, iteration_id, element_ids, stiffness_factors))
  {
    return false;
  }

  const auto entry = topology.topology_cases.find(topology_id);
  auto& configuration = entry->second;

  configuration.iteration_density_designs.emplace(iteration_id, design);

  // These entries were created by this method, before any job exists.
  const auto discard_iteration = [&]()
  {
    configuration.iteration_density_designs.erase(iteration_id);

    const auto collection = materials.topology_material_data.find(topology_id);

    if (collection != materials.topology_material_data.end())
    {
      collection->second.iteration_factors.erase(iteration_id);
      collection->second.iteration_material_caps.erase(iteration_id);
      collection->second.iteration_material_groups.erase(iteration_id);
    }
  };

  if (!topology.evaluate_density_volume(topology_id, iteration_id, element_volumes))
  {
    discard_iteration();
    return false;
  }

  const auto stored = configuration.iteration_density_designs.find(iteration_id);
  const double minimum = stored->second.volume.minimum_design_volume_fraction;

  if (!std::isfinite(configuration.target_volume_fraction) ||
      configuration.target_volume_fraction <= 0.0 ||
      configuration.target_volume_fraction > 1.0)
  {
    PRINT_ERROR("Invalid topology target volume fraction.\n");
    discard_iteration();
    return false;
  }

  // Allow a small tolerance for accumulated floating-point rounding.
  if (configuration.target_volume_fraction + 1.0e-12 < minimum)
  {
    PRINT_ERROR(
        "Topology %d target volume fraction %.9g is below "
        "the minimum achievable fraction %.9g.\n",
        topology_id, configuration.target_volume_fraction, minimum);

    discard_iteration();
    return false;
  }

  return true;
}
