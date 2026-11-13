#include "CoreTopologyLevelSetEvaluation.hpp"
#include "CoreTopologyLevelSet.hpp"
#include "CoreTopologyLevelSetGeometry.hpp"
#include "CoreTopologyLevelSetPipeline.hpp"
#include "loadUserOptions.hpp"
#include "CoreTopologyLevelSetGeometryCache.hpp"
#include "CoreTopologyProgress.hpp"
#include "CoreTopologyStructs.hpp"
#include "CubitMessage.hpp"
#include <QScopeGuard>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <limits>
#include <exception>
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

// Only this coordinator accesses Cubit and the transient source cache. A batch
// owns its queried inputs until all numerical workers have completed.
bool evaluate_elements(const TopologyLevelSetDesign& design, const std::map<int, bool>& preserved,
  const std::map<int, double>& volumes, CoreTopologyLevelSetGeometryCache* cache,
  CoreTopologyProgress& progress, CoreTopologyLevelSetPipeline& pipeline, double& query_ms, double& cache_ms,
  const std::function<bool(const CoreTopologyLevelSetPipeline::Element&)>& append)
{
  auto next = volumes.begin();
  std::size_t progressed = 0;
  try
  {
    while (next != volumes.end())
    {
      std::vector<CoreTopologyLevelSetPipeline::Element> batch;
      batch.reserve(CoreTopologyLevelSetPipeline::maximum_batch_elements);
      while (next != volumes.end() && batch.size() < CoreTopologyLevelSetPipeline::maximum_batch_elements)
      {
        CoreTopologyLevelSetPipeline::Element input;
        input.id = next->first;
        input.volume = next->second;
        input.preserved = preserved.at(input.id);
        const auto query_begin = std::chrono::steady_clock::now();
        const bool queried = CoreTopologyLevelSetGeometry::query_element(input.id, input.calculix_type,
          input.cubit_type, input.nodes, input.coordinates);
        query_ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - query_begin).count();
        if (!queried)
        {
          return false;
        }
        input.values.reserve(input.nodes.size());
        for (int node : input.nodes)
        {
          const auto found = design.node_values.find(node);
          if (found == design.node_values.end())
          {
            PRINT_ERROR("Missing level-set coefficient for node %d.\n", node);
            return false;
          }
          input.values.push_back(found->second);
        }
        if (cache)
        {
          const auto cache_begin = std::chrono::steady_clock::now();
          const bool valid = cache->lookup(design.iteration_id, input.id, input.calculix_type, input.cubit_type,
            input.nodes, input.coordinates, input.values, input.volume, input.preserved, design.settings, input.geometry, input.cached);
          cache_ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - cache_begin).count();
          if (!valid)
          {
            return false;
          }
        }
        batch.push_back(std::move(input));
        ++next;
        if (!progress.poll())
        {
          return false;
        }
      }
      const std::size_t before = progressed;
      const auto poll = [&](std::size_t completed)
      {
        while (progressed < before + completed)
        {
          ++progressed;
          if (!progress.advance())
          {
            return false;
          }
        }
        return progress.poll();
      };
      std::string error;
      if (!pipeline.integrate(batch, design.settings, poll, error))
      {
        PRINT_ERROR("Level-set integration failed: %s\n", error.c_str());
        return false;
      }
      for (const auto& input : batch)
      {
        if (cache && !input.cached)
        {
          const auto cache_begin = std::chrono::steady_clock::now();
          const bool stored = cache->store(design.iteration_id, input.id, input.calculix_type, input.cubit_type,
            input.nodes, input.coordinates, input.values, input.volume, input.preserved, design.settings, input.geometry, input.integration_ms);
          cache_ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - cache_begin).count();
          if (!stored)
          {
            return false;
          }
        }
        if (!append(input) || !progress.poll())
        {
          return false;
        }
      }
    }
  }
  catch (const std::exception& failure)
  {
    PRINT_ERROR("Level-set evaluation failed: %s\n", failure.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Unexpected level-set evaluation failure.\n");
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

bool CoreTopologyLevelSetEvaluation::build_geometry(TopologyLevelSetDesign& design, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, CoreTopologyLevelSetGeometryCache* cache)
{
  const int configured_workers = ccx_uo.mConverterThreads;
  const int configured_budget_mib = ccx_uo.mTopologyQuadratureCacheMiB;
  const bool valid_budget = configured_budget_mib > 0 && static_cast<std::size_t>(configured_budget_mib) <= (std::numeric_limits<std::size_t>::max)() / (1024 * 1024);
  const std::size_t budget_bytes = valid_budget ? static_cast<std::size_t>(configured_budget_mib) * 1024 * 1024 : 0;
  CoreTopologyLevelSetPipeline pipeline(configured_workers > 0 ? static_cast<std::size_t>(configured_workers) : 0, budget_bytes);
  const auto begin = std::chrono::steady_clock::now();
  double query_ms = 0.0;
  double cache_ms = 0.0;
  const std::size_t hits_before = cache ? cache->hit_count() : 0;
  const std::size_t misses_before = cache ? cache->miss_count() : 0;
  double slowest_element_ms = 0.0;
  std::size_t completed = 0;
  int slowest_element = -1;
  int current_element = -1;
  int maximum_subdivisions = 0;
  bool succeeded = false;
  const auto report_timing = qScopeGuard([&]()
  {
    const double total_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
    const auto statistics = pipeline.statistics();
    const double other_ms = (std::max)(0.0, total_ms - query_ms - statistics.wall_ms - cache_ms);
    PRINT_INFO("Level-set iteration %d geometry [ms]: total %.3f; mesh queries %.3f; integration wall %.3f; other %.3f; status %s.\n",
      design.iteration_id, total_ms, query_ms, statistics.wall_ms, other_ms, succeeded ? "complete" : "incomplete");
    PRINT_INFO("Level-set iteration %d geometry cache: hits %zu; misses %zu; lookup/copy/validation %.3f ms.\n",
      design.iteration_id, cache ? cache->hit_count() - hits_before : 0, cache ? cache->miss_count() - misses_before : 0, cache_ms);
    PRINT_INFO("Level-set iteration %d geometry quadrature: rule hits %zu; builds %zu; budget fallbacks %zu; retained bytes %zu.\n",
      design.iteration_id, statistics.rule_hits, statistics.rule_builds, statistics.rule_fallbacks, statistics.retained_bytes);
    PRINT_INFO("Level-set iteration %d integration workers: configured %d; active %zu; aggregate integration %.3f ms.\n",
      design.iteration_id, configured_workers, statistics.workers, statistics.worker_ms);
    PRINT_INFO("Level-set iteration %d shared quadrature: budget %d MiB; preparation %.3f ms; preparation budget fallbacks %zu.\n",
      design.iteration_id, configured_budget_mib, statistics.preparation_ms, statistics.preparation_fallbacks);
    PRINT_INFO("Level-set iteration %d geometry: elements %zu/%zu; slowest element %d: %.3f ms; maximum subdivisions %d; incomplete element %d.\n",
      design.iteration_id, completed, element_volumes.size(), slowest_element, slowest_element_ms, maximum_subdivisions, current_element);
  });
  if (!valid_budget)
  {
    PRINT_ERROR("Topology quadrature cache budget must be positive MiB and fit in the address space.\n");
    return false;
  }
  if (configured_workers < 1)
  {
    PRINT_ERROR("Topology integration requires at least one converter thread.\n");
    return false;
  }
  if (!valid_design(design) || !valid_domain(preserved, element_volumes))
  {
    return false;
  }
  CoreTopologyProgress progress("Integrating Level-Set Geometry", element_volumes.size());
  std::map<int, double> fractions;
  TopologyVolumeEvaluation volume;
  std::set<int> touched;
  std::set<int> fixed;
  bool interface_available = false;
  double fully_fixed_volume = 0.0;
  const auto append = [&](const CoreTopologyLevelSetPipeline::Element& input)
  {
    const bool is_preserved = preserved.at(input.id);
    const auto& nodes = input.nodes;
    const auto& geometry = input.geometry;
    current_element = input.id;
    if (slowest_element < 0 || input.integration_ms > slowest_element_ms)
    {
      slowest_element_ms = input.integration_ms;
      slowest_element = input.id;
    }
    maximum_subdivisions = (std::max)(maximum_subdivisions, geometry.subdivisions);
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
      fully_fixed_volume += input.volume;
    }
    fractions.emplace(input.id, geometry.solid_fraction);
    volume.total_volume += input.volume;
    volume.solid_volume += geometry.solid_volume;
    if (is_preserved)
    {
      volume.preserved_volume += input.volume;
    }
    ++completed;
    current_element = -1;
    return true;
  };
  if (!evaluate_elements(design, preserved, element_volumes, cache, progress, pipeline, query_ms, cache_ms, append))
  {
    return false;
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
  if (!progress.finish())
  {
    return false;
  }
  design.element_solid_fractions.swap(fractions);
  design.volume = std::move(volume);
  design.free_interface_available = interface_available;
  succeeded = true;
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

bool CoreTopologyLevelSetEvaluation::evaluate_compliance(const TopologyLevelSetDesign& design, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, const std::map<int, double>& element_energies, const std::map<int, double>& applied_factors, TopologyLevelSetEvaluation& evaluation, CoreTopologyLevelSetGeometryCache* cache)
{
  const int configured_workers = ccx_uo.mConverterThreads;
  const int configured_budget_mib = ccx_uo.mTopologyQuadratureCacheMiB;
  const bool valid_budget = configured_budget_mib > 0 && static_cast<std::size_t>(configured_budget_mib) <= (std::numeric_limits<std::size_t>::max)() / (1024 * 1024);
  const std::size_t budget_bytes = valid_budget ? static_cast<std::size_t>(configured_budget_mib) * 1024 * 1024 : 0;
  CoreTopologyLevelSetPipeline pipeline(configured_workers > 0 ? static_cast<std::size_t>(configured_workers) : 0, budget_bytes);
  const auto begin = std::chrono::steady_clock::now();
  double query_ms = 0.0;
  double cache_ms = 0.0;
  const std::size_t hits_before = cache ? cache->hit_count() : 0;
  const std::size_t misses_before = cache ? cache->miss_count() : 0;
  double slowest_element_ms = 0.0;
  std::size_t completed = 0;
  int slowest_element = -1;
  int current_element = -1;
  int maximum_subdivisions = 0;
  bool succeeded = false;
  const auto report_timing = qScopeGuard([&]()
  {
    const double total_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
    const auto statistics = pipeline.statistics();
    const double other_ms = (std::max)(0.0, total_ms - query_ms - statistics.wall_ms - cache_ms);
    PRINT_INFO("Level-set iteration %d sensitivities [ms]: total %.3f; mesh queries %.3f; integration wall %.3f; other %.3f; status %s.\n",
      design.iteration_id, total_ms, query_ms, statistics.wall_ms, other_ms, succeeded ? "complete" : "incomplete");
    PRINT_INFO("Level-set iteration %d sensitivities cache: hits %zu; misses %zu; lookup/copy/validation %.3f ms.\n",
      design.iteration_id, cache ? cache->hit_count() - hits_before : 0, cache ? cache->miss_count() - misses_before : 0, cache_ms);
    PRINT_INFO("Level-set iteration %d sensitivities quadrature: rule hits %zu; builds %zu; budget fallbacks %zu; retained bytes %zu.\n",
      design.iteration_id, statistics.rule_hits, statistics.rule_builds, statistics.rule_fallbacks, statistics.retained_bytes);
    PRINT_INFO("Level-set iteration %d integration workers: configured %d; active %zu; aggregate integration %.3f ms.\n",
      design.iteration_id, configured_workers, statistics.workers, statistics.worker_ms);
    PRINT_INFO("Level-set iteration %d shared quadrature: budget %d MiB; preparation %.3f ms; preparation budget fallbacks %zu.\n",
      design.iteration_id, configured_budget_mib, statistics.preparation_ms, statistics.preparation_fallbacks);
    PRINT_INFO("Level-set iteration %d sensitivities: elements %zu/%zu; slowest element %d: %.3f ms; maximum subdivisions %d; incomplete element %d.\n",
      design.iteration_id, completed, element_volumes.size(), slowest_element, slowest_element_ms, maximum_subdivisions, current_element);
  });
  evaluation = {};
  if (!valid_budget)
  {
    PRINT_ERROR("Topology quadrature cache budget must be positive MiB and fit in the address space.\n");
    return false;
  }
  if (configured_workers < 1)
  {
    PRINT_ERROR("Topology integration requires at least one converter thread.\n");
    return false;
  }
  if (!valid_design(design) || !valid_domain(preserved, element_volumes) || !design.volume.available ||
      element_volumes.size() != design.element_solid_fractions.size() || element_energies.size() != element_volumes.size() || applied_factors.size() != element_volumes.size())
  {
    PRINT_ERROR("Incomplete solved level-set compliance data.\n");
    return false;
  }
  CoreTopologyProgress progress("Evaluating Level-Set Compliance Sensitivities", element_volumes.size());
  TopologyLevelSetEvaluation candidate;
  candidate.iteration_id = design.iteration_id;
  for (const auto& node : design.node_values)
  {
    candidate.compliance_derivatives.emplace(node.first, 0.0);
    candidate.volume_derivatives.emplace(node.first, 0.0);
  }
  std::set<int> touched;
  std::set<int> fixed;
  const auto append = [&](const CoreTopologyLevelSetPipeline::Element& input)
  {
    const auto energy = element_energies.find(input.id);
    const auto factor = applied_factors.find(input.id);
    const auto fraction = design.element_solid_fractions.find(input.id);
    const bool is_preserved = preserved.at(input.id);
    if (energy == element_energies.end() || factor == applied_factors.end() || fraction == design.element_solid_fractions.end() ||
        !std::isfinite(energy->second) || energy->second < 0.0 || !std::isfinite(factor->second) || factor->second <= 0.0 || factor->second > 1.0 ||
        !std::isfinite(fraction->second) || fraction->second < 0.0 || fraction->second > 1.0 ||
        (is_preserved && (factor->second != 1.0 || fraction->second != 1.0)))
    {
      PRINT_ERROR("Invalid solved level-set data for element %d.\n", input.id);
      return false;
    }
    const auto& nodes = input.nodes;
    const auto& geometry = input.geometry;
    current_element = input.id;
    if (slowest_element < 0 || input.integration_ms > slowest_element_ms)
    {
      slowest_element_ms = input.integration_ms;
      slowest_element = input.id;
    }
    maximum_subdivisions = (std::max)(maximum_subdivisions, geometry.subdivisions);
    if (std::abs(geometry.solid_fraction - fraction->second) > design.settings.integration_tolerance)
    {
      PRINT_ERROR("Stored level-set fraction disagrees with the current geometry for element %d.\n", input.id);
      return false;
    }
    const double requested = design.void_ratio + (1.0 - design.void_ratio) * fraction->second;
    candidate.factor_approximation = candidate.factor_approximation || std::abs(requested - factor->second) > 1.0e-12;
    if (!accumulate_compliance(design, nodes, geometry, input.volume, energy->second, factor->second, candidate))
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
    ++completed;
    current_element = -1;
    return true;
  };
  if (!evaluate_elements(design, preserved, element_volumes, cache, progress, pipeline, query_ms, cache_ms, append))
  {
    return false;
  }
  if (!valid_node_domain(design, touched, fixed) || !finalize_compliance(candidate))
  {
    return false;
  }
  if (!progress.finish())
  {
    return false;
  }
  evaluation = std::move(candidate);
  succeeded = true;
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
