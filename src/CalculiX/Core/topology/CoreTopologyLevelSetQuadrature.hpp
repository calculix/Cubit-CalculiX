#ifndef CORETOPOLOGYLEVELSETQUADRATURE_HPP
#define CORETOPOLOGYLEVELSETQUADRATURE_HPP

#include "ElementInterpolation.hpp"
#include <cstddef>
#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <vector>

/**
 * @brief Pass-local, bounded reference quadrature data; no mesh or design state.
 * @note Retained point/vector payload is limited to 32 MiB by default. Map/key
 * overhead and temporary interpolation scratch are not part of that budget.
 * Rules that do not fit use ordinary evaluation, without changing accuracy.
 * No eviction: borrowed rule pointers stay valid for this object's lifetime.
 * Calling-thread use only. Partial/interrupted rules are never published.
 */
class CoreTopologyLevelSetQuadrature
{
public:
  struct Point
  {
    double reference_weight = 0.0;
    std::vector<double> values;
    std::vector<ElementInterpolation::Coordinates> gradients;
  };

  struct Rule
  {
    std::vector<Point> points;
  };

  static constexpr std::size_t default_budget_bytes = 32 * 1024 * 1024;
  explicit CoreTopologyLevelSetQuadrature(std::size_t budget_bytes = default_budget_bytes);

  /// True/null rule means budget fallback; false means invalid input/interruption.
  /// Only powers of two through 32 are supported, matching Core refinement limits.
  bool acquire(const std::string& calculix_type, int cubit_type, int subdivisions, const std::function<bool()>& poll, const Rule*& rule);
  std::size_t hit_count() const;
  std::size_t build_count() const;
  std::size_t fallback_count() const;
  std::size_t retained_bytes() const;

private:
  using Key = std::tuple<std::string, int, int>;
  const std::size_t budget_bytes;
  std::size_t bytes = 0;
  std::size_t hits = 0;
  std::size_t builds = 0;
  std::size_t fallbacks = 0;
  bool building = false;
  std::map<Key, Rule> rules;
};

#endif
