#ifndef CORETOPOLOGYLEVELSETGEOMETRYCACHE_HPP
#define CORETOPOLOGYLEVELSETGEOMETRYCACHE_HPP

#include "CoreTopologyLevelSetStructs.hpp"
#include <array>
#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <vector>

class CoreTopologyLevelSetQuadrature;

/**
 * @brief Reuse integrated source geometry within one transient proposal.
 * @note Calling-thread use only. The caller still queries current mesh inputs.
 * Hits require exact ordered nodes, coordinates, coefficients, volume, types,
 * preservation and integration settings. Valid changes fall back to integration.
 * A different positive iteration is evaluated without storing/reusing entries.
 * Only successful integrations are cached. No model data or file is modified.
 */
class CoreTopologyLevelSetGeometryCache
{
public:
  explicit CoreTopologyLevelSetGeometryCache(int source_iteration);

  /// Output cleared on entry/failure. Poll false aborts without publishing an entry.
  bool evaluate(int iteration_id, int element_id, const std::string& calculix_type, int cubit_type, const std::vector<int>& nodes, const std::vector<std::array<double, 3>>& coordinates, const std::vector<double>& values, double cubit_volume, bool preserved, const TopologyLevelSetSettings& settings, TopologyLevelSetElementGeometry& geometry, const std::function<bool()>& poll, CoreTopologyLevelSetQuadrature* quadrature = nullptr);

  /// Calling-thread lookup; true/hit=false requests integration.
  bool lookup(int iteration_id, int element_id, const std::string& calculix_type, int cubit_type, const std::vector<int>& nodes, const std::vector<std::array<double, 3>>& coordinates, const std::vector<double>& values, double cubit_volume, bool preserved, const TopologyLevelSetSettings& settings, TopologyLevelSetElementGeometry& geometry, bool& hit);

  /// Publish an exclusively computed successful result on the calling thread.
  bool store(int iteration_id, int element_id, const std::string& calculix_type, int cubit_type, const std::vector<int>& nodes, const std::vector<std::array<double, 3>>& coordinates, const std::vector<double>& values, double cubit_volume, bool preserved, const TopologyLevelSetSettings& settings, const TopologyLevelSetElementGeometry& geometry, double elapsed_ms);

  /// Cumulative calling-thread counters; snapshot differences give per-pass totals.
  std::size_t hit_count() const;
  std::size_t miss_count() const;
  /// Sum of actual evaluate_element durations, excluding comparisons/copies.
  /// Parallel insertions contribute aggregate worker time, not pass wall time.
  double integration_time_ms() const;

private:
  struct Entry
  {
    std::string calculix_type;
    int cubit_type = -1;
    std::vector<int> nodes;
    std::vector<std::array<double, 3>> coordinates;
    std::vector<double> values;
    double cubit_volume = 0.0;
    bool preserved = false;
    TopologyLevelSetSettings settings{};
    TopologyLevelSetElementGeometry geometry;
  };

  const int source_iteration;
  std::map<int, Entry> entries;
  std::size_t hits = 0;
  std::size_t misses = 0;
  double integration_ms = 0.0;
};

#endif
