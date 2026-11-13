#ifndef CORETOPOLOGYLEVELSETQUADRATURE_HPP
#define CORETOPOLOGYLEVELSETQUADRATURE_HPP

#include "ElementInterpolation.hpp"
#include <cstddef>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

/**
 * @brief Pass-local, bounded reference quadrature data; no mesh or design state.
 * @note Retained point/vector payload is limited to 128 MiB by default. Map/key
 * overhead and temporary interpolation scratch are not part of that budget.
 * Rules that do not fit use ordinary evaluation, without changing accuracy.
 * No eviction: borrowed rule pointers stay valid for this object's lifetime.
 * An owning instance is mutated only between worker batches. Borrowing instances
 * share immutable rules but own their request sets/counters; the owner must
 * outlive every borrower. Partial/interrupted rules are never published. A non-null error
 * destination suppresses Cubit messages, including in reference interpolation.
 */
class CoreTopologyLevelSetQuadrature
{
public:
  using Key = std::tuple<std::string, int, int>;
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

  static constexpr int default_budget_mib = 128;
  static constexpr std::size_t default_budget_bytes = static_cast<std::size_t>(default_budget_mib) * 1024 * 1024;
  explicit CoreTopologyLevelSetQuadrature(std::size_t budget_bytes = default_budget_bytes, const CoreTopologyLevelSetQuadrature* reference_rules = nullptr);

  /// True/null rule means budget fallback; false means invalid input/interruption.
  /// Only powers of two through 32 are supported, matching Core refinement limits.
  bool acquire(const std::string& calculix_type, int cubit_type, int subdivisions, const std::function<bool()>& poll, const Rule*& rule, std::string* error = nullptr);
  /// Read-only owner lookup. No owner mutation is permitted while workers read.
  const Rule* find(const std::string& calculix_type, int cubit_type, int subdivisions) const;
  /// Borrower's missing-rule requests; inspect only after its worker finishes.
  const std::set<Key>& requested_rules() const;
  std::size_t hit_count() const;
  std::size_t build_count() const;
  std::size_t fallback_count() const;
  std::size_t retained_bytes() const;

private:
  const std::size_t budget_bytes;
  const CoreTopologyLevelSetQuadrature* reference_rules;
  std::set<Key> requests;
  std::size_t bytes = 0;
  std::size_t hits = 0;
  std::size_t builds = 0;
  std::size_t fallbacks = 0;
  bool building = false;
  std::map<Key, Rule> rules;
};

#endif
