#include "CoreTopologyLevelSetGeometryCache.hpp"
#include "CoreTopologyLevelSet.hpp"
#include "CoreTopologyLevelSetGeometry.hpp"
#include "ElementInterpolation.hpp"
#include "CubitMessage.hpp"
#include <chrono>
#include <cmath>
#include <set>
#include <utility>

CoreTopologyLevelSetGeometryCache::CoreTopologyLevelSetGeometryCache(int source_iteration)
  : source_iteration(source_iteration)
{
}

bool CoreTopologyLevelSetGeometryCache::evaluate(int iteration_id, int element_id, const std::string& calculix_type, int cubit_type, const std::vector<int>& nodes, const std::vector<std::array<double, 3>>& coordinates, const std::vector<double>& values, double cubit_volume, bool preserved, const TopologyLevelSetSettings& settings, TopologyLevelSetElementGeometry& geometry, const std::function<bool()>& poll, CoreTopologyLevelSetQuadrature* quadrature)
{
  geometry = {};
  if (poll && !poll())
  {
    return false;
  }
  bool hit = false;
  if (!lookup(iteration_id, element_id, calculix_type, cubit_type, nodes, coordinates, values, cubit_volume, preserved, settings, geometry, hit))
  {
    return false;
  }
  if (hit)
  {
    return true;
  }
  const auto begin = std::chrono::steady_clock::now();
  TopologyLevelSetElementGeometry candidate;
  const bool success = CoreTopologyLevelSetGeometry::evaluate_element(calculix_type, cubit_type, coordinates, values, cubit_volume, preserved, settings, candidate, poll, quadrature);
  const double elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
  if (!success)
  {
    integration_ms += elapsed;
    return false;
  }
  if (!store(iteration_id, element_id, calculix_type, cubit_type, nodes, coordinates, values, cubit_volume, preserved, settings, candidate, elapsed))
  {
    return false;
  }
  geometry = std::move(candidate);
  return true;
}

bool CoreTopologyLevelSetGeometryCache::lookup(int iteration_id, int element_id, const std::string& calculix_type, int cubit_type, const std::vector<int>& nodes, const std::vector<std::array<double, 3>>& coordinates, const std::vector<double>& values, double cubit_volume, bool preserved, const TopologyLevelSetSettings& settings, TopologyLevelSetElementGeometry& geometry, bool& hit)
{
  geometry = {};
  hit = false;
  if (!CoreTopologyLevelSet::validate_settings(settings))
  {
    return false;
  }
  const std::set<int> unique_nodes(nodes.begin(), nodes.end());
  if (source_iteration < 1 || iteration_id < 1 || element_id < 1 || nodes.empty() ||
      nodes.size() != coordinates.size() || nodes.size() != values.size() ||
      unique_nodes.size() != nodes.size() || *unique_nodes.begin() <= 0)
  {
    PRINT_ERROR("Invalid level-set geometry cache request.\n");
    return false;
  }
  const auto found = entries.find(element_id);
  if (iteration_id == source_iteration && found != entries.end())
  {
    const auto& entry = found->second;
    // Only these settings affect regularized geometry; all settings are validated above.
    if (entry.calculix_type == calculix_type && entry.cubit_type == cubit_type &&
        entry.nodes == nodes && entry.coordinates == coordinates && entry.values == values &&
        entry.cubit_volume == cubit_volume && entry.preserved == preserved &&
        entry.settings.interface_width == settings.interface_width &&
        entry.settings.integration_tolerance == settings.integration_tolerance &&
        entry.settings.integration_max_refinements == settings.integration_max_refinements)
    {
      geometry = entry.geometry;
      ++hits;
      hit = true;
      return true;
    }
  }

  ++misses;
  return true;
}

bool CoreTopologyLevelSetGeometryCache::store(int iteration_id, int element_id, const std::string& calculix_type, int cubit_type, const std::vector<int>& nodes, const std::vector<std::array<double, 3>>& coordinates, const std::vector<double>& values, double cubit_volume, bool preserved, const TopologyLevelSetSettings& settings, const TopologyLevelSetElementGeometry& geometry, double elapsed_ms)
{
  const std::set<int> unique_nodes(nodes.begin(), nodes.end());
  if (!CoreTopologyLevelSet::validate_settings(settings) || source_iteration < 1 || iteration_id < 1 || element_id < 1 ||
      nodes.empty() || nodes.size() != coordinates.size() || nodes.size() != values.size() ||
      unique_nodes.size() != nodes.size() || *unique_nodes.begin() <= 0 ||
      !ElementInterpolation::matches_cubit_type(calculix_type, cubit_type) || nodes.size() != ElementInterpolation::node_count(calculix_type) ||
      !std::isfinite(cubit_volume) || cubit_volume <= 0.0 || !std::isfinite(geometry.integration_volume) || geometry.integration_volume <= 0.0 ||
      !std::isfinite(geometry.solid_volume) || geometry.solid_volume != cubit_volume * geometry.solid_fraction ||
      (preserved && geometry.solid_fraction != 1.0) || geometry.subdivisions < 2 || geometry.subdivisions > 32 ||
      (geometry.subdivisions & (geometry.subdivisions - 1)) != 0 ||
      !std::isfinite(elapsed_ms) || elapsed_ms < 0.0 || !std::isfinite(geometry.solid_fraction) ||
      geometry.solid_fraction < 0.0 || geometry.solid_fraction > 1.0 ||
      geometry.fraction_derivatives.size() != nodes.size() || geometry.volume_derivatives.size() != nodes.size())
  {
    PRINT_ERROR("Invalid completed level-set cache entry.\n");
    return false;
  }
  for (std::size_t node = 0; node < nodes.size(); ++node)
  {
    if (!std::isfinite(values[node]) || values[node] < -1.0 || values[node] > 1.0 ||
        !std::isfinite(geometry.fraction_derivatives[node]) || !std::isfinite(geometry.volume_derivatives[node]) ||
        (preserved && (geometry.fraction_derivatives[node] != 0.0 || geometry.volume_derivatives[node] != 0.0)))
    {
      PRINT_ERROR("Invalid completed level-set cache coefficients/derivatives.\n");
      return false;
    }
    for (double coordinate : coordinates[node])
    {
      if (!std::isfinite(coordinate))
      {
        PRINT_ERROR("Invalid completed level-set cache coordinates.\n");
        return false;
      }
    }
  }
  if (iteration_id == source_iteration)
  {
    Entry entry;
    entry.calculix_type = calculix_type;
    entry.cubit_type = cubit_type;
    entry.nodes = nodes;
    entry.coordinates = coordinates;
    entry.values = values;
    entry.cubit_volume = cubit_volume;
    entry.preserved = preserved;
    entry.settings = settings;
    entry.geometry = geometry;
    entries.insert_or_assign(element_id, std::move(entry));
  }
  integration_ms += elapsed_ms;
  return true;
}

std::size_t CoreTopologyLevelSetGeometryCache::hit_count() const
{
  return hits;
}

std::size_t CoreTopologyLevelSetGeometryCache::miss_count() const
{
  return misses;
}

double CoreTopologyLevelSetGeometryCache::integration_time_ms() const
{
  return integration_ms;
}
