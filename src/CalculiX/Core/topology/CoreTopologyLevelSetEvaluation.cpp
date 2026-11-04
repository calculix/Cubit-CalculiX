#include "CoreTopologyLevelSetEvaluation.hpp"
#include "CoreTopologyLevelSet.hpp"
#include "CoreTopologyLevelSetGeometry.hpp"
#include "CoreTopologyStructs.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <set>
#include <utility>

namespace
{
bool valid_design(const TopologyLevelSetDesign& design)
{
  if (design.iteration_id < 1 || (design.source_iteration != -1 && (design.source_iteration < 1 || design.source_iteration >= design.iteration_id)) ||
      !std::isfinite(design.void_ratio) || design.void_ratio <= 0.0 || design.void_ratio >= 1.0 ||
      !CoreTopologyLevelSet::validate_settings(design.settings) || design.settings.regularization_length <= 0.0 ||
      design.settings.seed_spacing <= 0.0 || design.settings.seed_radius <= 0.0 || design.node_values.empty() ||
      !std::isfinite(design.characteristic_length) || design.characteristic_length <= 0.0)
  {
    PRINT_ERROR("Invalid or unresolved level-set design.\n");
    return false;
  }
  for (double coordinate : design.seed_origin)
  {
    if (!std::isfinite(coordinate))
    {
      PRINT_ERROR("Invalid level-set seed origin.\n");
      return false;
    }
  }
  for (const auto& node : design.node_values)
  {
    if (node.first <= 0 || !std::isfinite(node.second) || node.second < -1.0 || node.second > 1.0 ||
        (design.fixed_nodes.count(node.first) != 0 && node.second != 1.0))
    {
      PRINT_ERROR("Invalid or unpreserved level-set nodal coefficient.\n");
      return false;
    }
  }
  for (int node : design.fixed_nodes)
  {
    if (design.node_values.count(node) == 0)
    {
      PRINT_ERROR("Missing fixed level-set node.\n");
      return false;
    }
  }
  return true;
}

bool valid_domain(const std::map<int, bool>& preserved, const std::map<int, double>& volumes)
{
  if (volumes.empty() || volumes.size() != preserved.size())
  {
    PRINT_ERROR("Incomplete level-set element volume/preservation domain.\n");
    return false;
  }
  for (const auto& entry : volumes)
  {
    if (entry.first <= 0 || preserved.count(entry.first) == 0 || !std::isfinite(entry.second) || entry.second <= 0.0)
    {
      PRINT_ERROR("Invalid level-set domain element volume or preservation.\n");
      return false;
    }
  }
  return true;
}

bool element_geometry(const TopologyLevelSetDesign& design, int element, double volume, bool preserved, std::vector<int>& nodes, TopologyLevelSetElementGeometry& geometry)
{
  std::string type;
  int cubit_type = -1;
  std::vector<std::array<double, 3>> coordinates;
  if (!CoreTopologyLevelSetGeometry::query_element(element, type, cubit_type, nodes, coordinates))
  {
    return false;
  }
  std::vector<double> values;
  values.reserve(nodes.size());
  for (int node : nodes)
  {
    const auto found = design.node_values.find(node);
    if (found == design.node_values.end())
    {
      PRINT_ERROR("Missing level-set coefficient for node %d.\n", node);
      return false;
    }
    values.push_back(found->second);
  }
  if (!CoreTopologyLevelSetGeometry::evaluate_element(type, cubit_type, coordinates, values, volume, preserved, design.settings, geometry))
  {
    PRINT_ERROR("Unable to evaluate level-set geometry for element %d.\n", element);
    return false;
  }
  return true;
}

bool valid_node_domain(const TopologyLevelSetDesign& design, const std::set<int>& nodes, const std::set<int>& fixed)
{
  if (nodes.size() != design.node_values.size() || fixed != design.fixed_nodes)
  {
    PRINT_ERROR("Level-set field/protected nodes do not match the current Cubit domain.\n");
    return false;
  }
  return true;
}
}

bool CoreTopologyLevelSetEvaluation::build_geometry(TopologyLevelSetDesign& design, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes)
{
  if (!valid_design(design) || !valid_domain(preserved, element_volumes))
  {
    return false;
  }
  std::map<int, double> fractions;
  TopologyVolumeEvaluation volume;
  std::set<int> touched;
  std::set<int> fixed;
  bool interface_available = false;
  double fully_fixed_volume = 0.0;
  for (const auto& entry : element_volumes)
  {
    const bool is_preserved = preserved.at(entry.first);
    std::vector<int> nodes;
    TopologyLevelSetElementGeometry geometry;
    if (!element_geometry(design, entry.first, entry.second, is_preserved, nodes, geometry))
    {
      return false;
    }
    bool fully_fixed = true;
    for (std::size_t node = 0; node < nodes.size(); ++node)
    {
      fully_fixed = fully_fixed && design.fixed_nodes.count(nodes[node]) != 0;
      touched.insert(nodes[node]);
      if (is_preserved)
      {
        fixed.insert(nodes[node]);
      }
      if (design.fixed_nodes.count(nodes[node]) == 0 && geometry.fraction_derivatives[node] != 0.0)
      {
        interface_available = true;
      }
    }
    if (fully_fixed)
    {
      fully_fixed_volume += entry.second;
    }
    fractions.emplace(entry.first, geometry.solid_fraction);
    volume.total_volume += entry.second;
    volume.solid_volume += geometry.solid_volume;
    if (is_preserved)
    {
      volume.preserved_volume += entry.second;
    }
  }
  if (!valid_node_domain(design, touched, fixed) || !std::isfinite(volume.total_volume) || volume.total_volume <= 0.0 ||
      !std::isfinite(volume.solid_volume) || !std::isfinite(volume.preserved_volume) || !std::isfinite(fully_fixed_volume))
  {
    PRINT_ERROR("Invalid integrated level-set design volumes.\n");
    return false;
  }
  if (design.source_iteration == -1 && !interface_available)
  {
    PRINT_ERROR("Initial level-set seeds have no resolved free interface; adjust seed size/spacing or refine the mesh/integration.\n");
    return false;
  }
  volume.volume_fraction = volume.solid_volume / volume.total_volume;
  volume.minimum_volume_fraction = volume.preserved_volume / volume.total_volume;
  // A conservative lower bound: pinning can fully fix additional free elements.
  // Partially pinned elements can impose further restrictions not quantified here.
  volume.minimum_design_volume_fraction = fully_fixed_volume / volume.total_volume;
  volume.available = true;
  design.element_solid_fractions.swap(fractions);
  design.volume = std::move(volume);
  design.free_interface_available = interface_available;
  return true;
}

bool CoreTopologyLevelSetEvaluation::build_factors(const TopologyLevelSetDesign& design, const std::map<int, bool>& preserved, std::map<int, double>& factors)
{
  factors.clear();
  if (!valid_design(design) || !design.volume.available || preserved.empty() || preserved.size() != design.element_solid_fractions.size())
  {
    PRINT_ERROR("Complete level-set fractions are required for stiffness factors.\n");
    return false;
  }
  std::map<int, double> candidate;
  for (const auto& fraction : design.element_solid_fractions)
  {
    const auto flag = preserved.find(fraction.first);
    if (fraction.first <= 0 || flag == preserved.end() || !std::isfinite(fraction.second) || fraction.second < 0.0 || fraction.second > 1.0 ||
        (flag->second && fraction.second != 1.0))
    {
      PRINT_ERROR("Invalid level-set fraction/preservation for stiffness interpolation.\n");
      return false;
    }
    const double factor = flag->second ? 1.0 : design.void_ratio + (1.0 - design.void_ratio) * fraction.second;
    if (!std::isfinite(factor) || factor <= 0.0 || factor > 1.0)
    {
      PRINT_ERROR("Invalid level-set stiffness factor.\n");
      return false;
    }
    candidate.emplace(fraction.first, factor);
  }
  factors.swap(candidate);
  return true;
}

bool CoreTopologyLevelSetEvaluation::evaluate_compliance(const TopologyLevelSetDesign& design, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, const std::map<int, double>& element_energies, const std::map<int, double>& applied_factors, TopologyLevelSetEvaluation& evaluation)
{
  evaluation = {};
  if (!valid_design(design) || !valid_domain(preserved, element_volumes) || !design.volume.available ||
      element_volumes.size() != design.element_solid_fractions.size() || element_energies.size() != element_volumes.size() || applied_factors.size() != element_volumes.size())
  {
    PRINT_ERROR("Incomplete solved level-set compliance data.\n");
    return false;
  }
  TopologyLevelSetEvaluation candidate;
  candidate.iteration_id = design.iteration_id;
  for (const auto& node : design.node_values)
  {
    candidate.compliance_derivatives.emplace(node.first, 0.0);
    candidate.volume_derivatives.emplace(node.first, 0.0);
  }
  std::set<int> touched;
  std::set<int> fixed;
  for (const auto& entry : element_volumes)
  {
    const auto energy = element_energies.find(entry.first);
    const auto factor = applied_factors.find(entry.first);
    const auto fraction = design.element_solid_fractions.find(entry.first);
    const bool is_preserved = preserved.at(entry.first);
    if (energy == element_energies.end() || factor == applied_factors.end() || fraction == design.element_solid_fractions.end() ||
        !std::isfinite(energy->second) || energy->second < 0.0 || !std::isfinite(factor->second) || factor->second <= 0.0 || factor->second > 1.0 ||
        !std::isfinite(fraction->second) || fraction->second < 0.0 || fraction->second > 1.0 ||
        (is_preserved && (factor->second != 1.0 || fraction->second != 1.0)))
    {
      PRINT_ERROR("Invalid solved level-set data for element %d.\n", entry.first);
      return false;
    }
    std::vector<int> nodes;
    TopologyLevelSetElementGeometry geometry;
    if (!element_geometry(design, entry.first, entry.second, is_preserved, nodes, geometry))
    {
      return false;
    }
    if (std::abs(geometry.solid_fraction - fraction->second) > design.settings.integration_tolerance)
    {
      PRINT_ERROR("Stored level-set fraction disagrees with the current geometry for element %d.\n", entry.first);
      return false;
    }
    const double requested = design.void_ratio + (1.0 - design.void_ratio) * fraction->second;
    candidate.factor_approximation = candidate.factor_approximation || std::abs(requested - factor->second) > 1.0e-12;
    const double stiffness_derivative = -2.0 * (energy->second / factor->second);
    if (!std::isfinite(stiffness_derivative))
    {
      PRINT_ERROR("Non-finite level-set compliance factor derivative.\n");
      return false;
    }
    candidate.compliance += 2.0 * energy->second;
    candidate.total_volume += entry.second;
    candidate.solid_volume += geometry.solid_volume;
    for (std::size_t node = 0; node < nodes.size(); ++node)
    {
      const int id = nodes[node];
      touched.insert(id);
      if (is_preserved)
      {
        fixed.insert(id);
      }
      if (design.fixed_nodes.count(id) != 0)
      {
        continue;
      }
      auto& objective_derivative = candidate.compliance_derivatives.at(id);
      auto& volume_derivative = candidate.volume_derivatives.at(id);
      objective_derivative += stiffness_derivative * (1.0 - design.void_ratio) * geometry.fraction_derivatives[node];
      volume_derivative += geometry.volume_derivatives[node];
      if (!std::isfinite(objective_derivative) || !std::isfinite(volume_derivative))
      {
        PRINT_ERROR("Non-finite assembled level-set nodal derivative.\n");
        return false;
      }
    }
  }
  if (!valid_node_domain(design, touched, fixed) || !std::isfinite(candidate.compliance) ||
      !std::isfinite(candidate.total_volume) || candidate.total_volume <= 0.0 || !std::isfinite(candidate.solid_volume))
  {
    PRINT_ERROR("Invalid assembled level-set compliance or volume.\n");
    return false;
  }
  candidate.volume_fraction = candidate.solid_volume / candidate.total_volume;
  for (const auto& node : candidate.volume_derivatives)
  {
    const double derivative = node.second / candidate.total_volume;
    if (!std::isfinite(derivative))
    {
      PRINT_ERROR("Non-finite level-set volume fraction derivative.\n");
      return false;
    }
    candidate.volume_fraction_derivatives.emplace(node.first, derivative);
  }
  candidate.available = true;
  evaluation = std::move(candidate);
  return true;
}
