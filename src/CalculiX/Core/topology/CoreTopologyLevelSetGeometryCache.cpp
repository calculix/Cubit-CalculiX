#include "CoreTopologyLevelSetGeometryCache.hpp"
#include "CoreTopologyLevelSet.hpp"
#include "CoreTopologyLevelSetGeometry.hpp"
#include "CubitMessage.hpp"
#include <QScopeGuard>
#include <chrono>
#include <set>
#include <utility>

CoreTopologyLevelSetGeometryCache::CoreTopologyLevelSetGeometryCache(int source_iteration)
  : source_iteration(source_iteration)
{
}

bool CoreTopologyLevelSetGeometryCache::evaluate(int iteration_id, int element_id, const std::string& calculix_type, int cubit_type, const std::vector<int>& nodes, const std::vector<std::array<double, 3>>& coordinates, const std::vector<double>& values, double cubit_volume, bool preserved, const TopologyLevelSetSettings& settings, TopologyLevelSetElementGeometry& geometry, const std::function<bool()>& poll, CoreTopologyLevelSetQuadrature* quadrature)
{
  geometry = {};
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
  if (poll && !poll())
  {
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
      return true;
    }
  }

  ++misses;
  TopologyLevelSetElementGeometry candidate;
  bool integrated = false;
  {
    const auto begin = std::chrono::steady_clock::now();
    const auto record_time = qScopeGuard([&]()
    {
      integration_ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
    });
    integrated = CoreTopologyLevelSetGeometry::evaluate_element(calculix_type, cubit_type, coordinates, values, cubit_volume, preserved, settings, candidate, poll, quadrature);
  }
  if (!integrated)
  {
    return false;
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
    entry.geometry = candidate;
    entries.insert_or_assign(element_id, std::move(entry));
  }
  geometry = std::move(candidate);
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
