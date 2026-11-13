#ifndef CORETOPOLOGYLEVELSETPIPELINE_HPP
#define CORETOPOLOGYLEVELSETPIPELINE_HPP

#include "CoreTopologyLevelSetStructs.hpp"
#include "CoreTopologyLevelSetQuadrature.hpp"
#include <boost/asio/thread_pool.hpp>
#include <array>
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <vector>

/**
 * @brief Calling-thread coordinator for bounded, independent element integrations.
 * @note Workers never query Cubit, print messages or access a model/source cache.
 * Each element retains its serial quadrature order. Callers assemble results in
 * element order, after a successful batch, to preserve the existing sum order.
 * One instance reuses its pool and shared immutable reference rules per pass.
 * All submitted work finishes before integrate returns, even on interruption.
 * The 128 MiB default bounds shared retained quadrature payload, not allocator
 * overhead, pool stacks, inputs or scratch. Input batches contain at most 256
 * supported volume elements (at most 20 nodes each); no full mesh copy is held.
 */
class CoreTopologyLevelSetPipeline
{
public:
  struct Element
  {
    int id = -1;
    std::string calculix_type;
    int cubit_type = -1;
    std::vector<int> nodes;
    std::vector<std::array<double, 3>> coordinates;
    std::vector<double> values;
    double volume = 0.0;
    bool preserved = false;
    bool cached = false;
    bool success = false;
    TopologyLevelSetElementGeometry geometry;
    double integration_ms = 0.0;
    std::string error;
  };

  struct Statistics
  {
    double wall_ms = 0.0;
    double worker_ms = 0.0;
    double preparation_ms = 0.0;
    std::size_t workers = 0;
    std::size_t rule_hits = 0;
    std::size_t rule_builds = 0;
    std::size_t rule_fallbacks = 0;
    std::size_t preparation_fallbacks = 0;
    std::size_t retained_bytes = 0;
  };

  using Poll = std::function<bool(std::size_t completed)>;
  static constexpr std::size_t maximum_batch_elements = 256;

  explicit CoreTopologyLevelSetPipeline(std::size_t workers, std::size_t quadrature_budget = CoreTopologyLevelSetQuadrature::default_budget_bytes);
  ~CoreTopologyLevelSetPipeline();
  CoreTopologyLevelSetPipeline(const CoreTopologyLevelSetPipeline&) = delete;
  CoreTopologyLevelSetPipeline& operator=(const CoreTopologyLevelSetPipeline&) = delete;

  /// Poll runs exclusively on the calling thread, including while workers run.
  /// Batch outputs may be partial on failure; the caller must not publish them.
  bool integrate(std::vector<Element>& elements, const TopologyLevelSetSettings& settings, const Poll& poll, std::string& error);
  /// Calling-thread access between batches only; no concurrent integrate/access.
  Statistics statistics() const;

private:
  bool prepare_rules(const std::vector<Element>& elements, const TopologyLevelSetSettings& settings, const Poll& poll, std::string& error);
  static void integrate_element(Element& element, const TopologyLevelSetSettings& settings, CoreTopologyLevelSetQuadrature& quadrature, const std::atomic<bool>& cancelled, const std::function<bool()>& caller_poll = {});

  const std::size_t requested_workers;
  // Owner precedes its borrowers, so it is destroyed after them.
  CoreTopologyLevelSetQuadrature reference_rules;
  std::set<CoreTopologyLevelSetQuadrature::Key> prepared_rules;
  std::vector<std::unique_ptr<CoreTopologyLevelSetQuadrature>> quadratures;
  std::unique_ptr<boost::asio::thread_pool> pool;
  double wall_ms = 0.0;
  double worker_ms = 0.0;
  double preparation_ms = 0.0;
};

#endif
