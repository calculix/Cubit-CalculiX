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
    if (!accumulate_compliance(design, nodes, geometry, entry.second, energy->second, factor->second, candidate))
    {
      return false;
    }
    for (std::size_t node = 0; node < nodes.size(); ++node)
    {
      const int id = nodes[node];
      touched.insert(id);
      if (is_preserved)
      {
        fixed.insert(id);
      }
    }
  }
  if (!valid_node_domain(design, touched, fixed) || !finalize_compliance(candidate))
  {
    return false;
  }
  evaluation = std::move(candidate);
  return true;
}

bool CoreTopologyLevelSetEvaluation::accumulate_compliance(const TopologyLevelSetDesign& design, const std::vector<int>& nodes, const TopologyLevelSetElementGeometry& geometry, double element_volume, double element_energy, double applied_factor, TopologyLevelSetEvaluation& evaluation)
{
  if (design.iteration_id < 1 || evaluation.iteration_id != design.iteration_id || evaluation.available ||
      !std::isfinite(design.void_ratio) || design.void_ratio <= 0.0 || design.void_ratio >= 1.0 || nodes.empty() ||
      evaluation.compliance_derivatives.size() != design.node_values.size() || evaluation.volume_derivatives.size() != design.node_values.size() ||
      !evaluation.volume_fraction_derivatives.empty() ||
      geometry.fraction_derivatives.size() != nodes.size() || geometry.volume_derivatives.size() != nodes.size() ||
      !std::isfinite(geometry.solid_fraction) || geometry.solid_fraction < 0.0 || geometry.solid_fraction > 1.0 ||
      !std::isfinite(geometry.integration_volume) || geometry.integration_volume <= 0.0 ||
      !std::isfinite(geometry.solid_volume) || geometry.solid_volume < 0.0 ||
      !std::isfinite(element_volume) || element_volume <= 0.0 || !std::isfinite(element_energy) || element_energy < 0.0 ||
      !std::isfinite(applied_factor) || applied_factor <= 0.0 || applied_factor > 1.0 ||
      !std::isfinite(evaluation.compliance) || evaluation.compliance < 0.0 ||
      !std::isfinite(evaluation.total_volume) || evaluation.total_volume < 0.0 ||
      !std::isfinite(evaluation.solid_volume) || evaluation.solid_volume < 0.0)
  {
    PRINT_ERROR("Invalid element data or unfinished level-set compliance accumulator.\n");
    evaluation = {};
    return false;
  }
  std::set<int> unique;
  for (std::size_t node = 0; node < nodes.size(); ++node)
  {
    const int id = nodes[node];
    const auto value = design.node_values.find(id);
    const auto objective = evaluation.compliance_derivatives.find(id);
    const auto volume = evaluation.volume_derivatives.find(id);
    if (id <= 0 || !unique.insert(id).second || value == design.node_values.end() ||
        objective == evaluation.compliance_derivatives.end() || volume == evaluation.volume_derivatives.end() ||
        !std::isfinite(value->second) || value->second < -1.0 || value->second > 1.0 ||
        !std::isfinite(objective->second) || !std::isfinite(volume->second) ||
        !std::isfinite(geometry.fraction_derivatives[node]) || !std::isfinite(geometry.volume_derivatives[node]) ||
        (design.fixed_nodes.count(id) != 0 && (value->second != 1.0 || objective->second != 0.0 || volume->second != 0.0)))
    {
      PRINT_ERROR("Invalid node domain or derivative in level-set compliance accumulation.\n");
      evaluation = {};
      return false;
    }
  }
  const double stiffness_derivative = -2.0 * (element_energy / applied_factor);
  if (!std::isfinite(stiffness_derivative))
  {
    PRINT_ERROR("Non-finite level-set compliance factor derivative.\n");
    evaluation = {};
    return false;
  }
  evaluation.compliance += 2.0 * element_energy;
  evaluation.total_volume += element_volume;
  evaluation.solid_volume += geometry.solid_volume;
  if (!std::isfinite(evaluation.compliance) || !std::isfinite(evaluation.total_volume) || !std::isfinite(evaluation.solid_volume))
  {
    PRINT_ERROR("Non-finite accumulated level-set compliance or volume.\n");
    evaluation = {};
    return false;
  }
  for (std::size_t node = 0; node < nodes.size(); ++node)
  {
    const int id = nodes[node];
    if (design.fixed_nodes.count(id) != 0)
    {
      continue;
    }
    auto& objective = evaluation.compliance_derivatives.at(id);
    auto& volume = evaluation.volume_derivatives.at(id);
    objective += stiffness_derivative * (1.0 - design.void_ratio) * geometry.fraction_derivatives[node];
    volume += geometry.volume_derivatives[node];
    if (!std::isfinite(objective) || !std::isfinite(volume))
    {
      PRINT_ERROR("Non-finite assembled level-set nodal derivative.\n");
      evaluation = {};
      return false;
    }
  }
  return true;
}

bool CoreTopologyLevelSetEvaluation::finalize_compliance(TopologyLevelSetEvaluation& evaluation)
{
  if (evaluation.available || evaluation.iteration_id < 1 || !std::isfinite(evaluation.compliance) || evaluation.compliance < 0.0 ||
      !std::isfinite(evaluation.total_volume) || evaluation.total_volume <= 0.0 || !std::isfinite(evaluation.solid_volume) || evaluation.solid_volume < 0.0 ||
      evaluation.compliance_derivatives.empty() || evaluation.compliance_derivatives.size() != evaluation.volume_derivatives.size() ||
      !evaluation.volume_fraction_derivatives.empty())
  {
    PRINT_ERROR("Invalid assembled level-set compliance or volume.\n");
    evaluation = {};
    return false;
  }
  evaluation.volume_fraction = evaluation.solid_volume / evaluation.total_volume;
  if (!std::isfinite(evaluation.volume_fraction))
  {
    PRINT_ERROR("Non-finite assembled level-set volume fraction.\n");
    evaluation = {};
    return false;
  }
  for (const auto& node : evaluation.volume_derivatives)
  {
    const auto objective = evaluation.compliance_derivatives.find(node.first);
    const double derivative = node.second / evaluation.total_volume;
    if (node.first <= 0 || objective == evaluation.compliance_derivatives.end() || !std::isfinite(objective->second) ||
        !std::isfinite(node.second) || !std::isfinite(derivative))
    {
      PRINT_ERROR("Invalid nodal domain or non-finite level-set volume fraction derivative.\n");
      evaluation = {};
      return false;
    }
    evaluation.volume_fraction_derivatives.emplace(node.first, derivative);
  }
  evaluation.available = true;
  return true;
}
