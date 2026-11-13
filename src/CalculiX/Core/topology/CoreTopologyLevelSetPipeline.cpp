#include "CoreTopologyLevelSetPipeline.hpp"
#include "CoreTopologyLevelSetGeometry.hpp"
#include "CoreTopologyLevelSet.hpp"
#include <boost/asio/post.hpp>
#include <QScopeGuard>
#include <algorithm>
#include <chrono>
#include <exception>
#include <thread>

CoreTopologyLevelSetPipeline::CoreTopologyLevelSetPipeline(std::size_t workers, std::size_t quadrature_budget)
  : requested_workers(workers), reference_rules(quadrature_budget)
{
}

CoreTopologyLevelSetPipeline::~CoreTopologyLevelSetPipeline()
{
  if (pool)
  {
    pool->join();
  }
}

void CoreTopologyLevelSetPipeline::integrate_element(Element& element, const TopologyLevelSetSettings& settings, CoreTopologyLevelSetQuadrature& quadrature, const std::atomic<bool>& cancelled, const std::function<bool()>& caller_poll)
{
  if (cancelled.load(std::memory_order_relaxed))
  {
    return;
  }
  if (element.cached)
  {
    element.success = true;
    return;
  }
  const auto begin = std::chrono::steady_clock::now();
  const auto poll = [&cancelled, &caller_poll]()
  {
    return !cancelled.load(std::memory_order_relaxed) && (!caller_poll || caller_poll());
  };
  try
  {
    element.success = CoreTopologyLevelSetGeometry::evaluate_element(element.calculix_type, element.cubit_type,
      element.coordinates, element.values, element.volume, element.preserved, settings, element.geometry, poll, &quadrature, &element.error);
  }
  catch (const std::exception& failure)
  {
    element.success = false;
    element.error = failure.what();
  }
  catch (...)
  {
    element.success = false;
    element.error = "Unexpected level-set integration worker failure.";
  }
  element.integration_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
}

bool CoreTopologyLevelSetPipeline::prepare_rules(const std::vector<Element>& elements, const TopologyLevelSetSettings& settings, const Poll& poll, std::string& error)
{
  const auto begin = std::chrono::steady_clock::now();
  const auto record_time = qScopeGuard([&]()
  {
    preparation_ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
  });
  if (!CoreTopologyLevelSet::validate_settings(settings, &error))
  {
    return false;
  }
  std::set<CoreTopologyLevelSetQuadrature::Key> needed;
  for (const auto& element : elements)
  {
    if (!element.cached)
    {
      // Both levels are always required by the existing refinement comparison.
      needed.emplace(element.calculix_type, element.cubit_type, 1);
      needed.emplace(element.calculix_type, element.cubit_type, 2);
    }
  }
  for (const auto& quadrature : quadratures)
  {
    const auto& requests = quadrature->requested_rules();
    needed.insert(requests.begin(), requests.end());
  }
  const auto preparation_poll = [&]()
  {
    return !poll || poll(0);
  };
  // Prepare finer levels only after an earlier batch actually requested them.
  // Coarse levels across all encountered types have priority under a tight budget.
  int subdivisions = 1;
  for (int level = 0; level <= settings.integration_max_refinements; ++level)
  {
    for (const auto& key : needed)
    {
      if (std::get<2>(key) != subdivisions || prepared_rules.count(key) != 0)
      {
        continue;
      }
      const CoreTopologyLevelSetQuadrature::Rule* rule = nullptr;
      if (!reference_rules.acquire(std::get<0>(key), std::get<1>(key), subdivisions, preparation_poll, rule, &error))
      {
        if (error.empty())
        {
          error = "Topology reference-rule preparation interrupted.";
        }
        return false;
      }
      // A rejected rule cannot fit later: this pass never evicts or grows its budget.
      prepared_rules.insert(key);
    }
    subdivisions *= 2;
  }
  return true;
}

bool CoreTopologyLevelSetPipeline::integrate(std::vector<Element>& elements, const TopologyLevelSetSettings& settings, const Poll& poll, std::string& error)
{
  error.clear();
  if (requested_workers == 0 || elements.size() > maximum_batch_elements)
  {
    error = "Invalid topology integration worker count or batch size.";
    return false;
  }
  for (auto& element : elements)
  {
    element.success = false;
    element.integration_ms = 0.0;
    element.error.clear();
    if (element.id < 1 || element.nodes.empty() || element.nodes.size() > 20 ||
        element.coordinates.size() != element.nodes.size() || element.values.size() != element.nodes.size())
    {
      error = "Invalid topology integration batch element.";
      return false;
    }
  }
  const auto begin = std::chrono::steady_clock::now();
  std::atomic<bool> cancelled{false};
  std::atomic<std::size_t> next{0};
  std::atomic<std::size_t> completed{0};
  std::atomic<std::size_t> finished{0};
  std::size_t submitted = 0;
  std::vector<unsigned char> worker_failed;
  // Drain even if allocating an error description itself throws while handling
  // a failure. This guard is destroyed before the worker-captured stack data.
  const auto drain = qScopeGuard([&]()
  {
    cancelled.store(true, std::memory_order_relaxed);
    while (finished.load(std::memory_order_acquire) < submitted)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });
  try
  {
    if (poll && !poll(0))
    {
      cancelled.store(true, std::memory_order_relaxed);
    }
    const bool all_cached = std::all_of(elements.begin(), elements.end(), [](const Element& element)
    {
      return element.cached;
    });
    if (!cancelled.load(std::memory_order_relaxed) && !all_cached && !prepare_rules(elements, settings, poll, error))
    {
      cancelled.store(true, std::memory_order_relaxed);
    }
    if (!cancelled.load(std::memory_order_relaxed) && !all_cached && quadratures.empty())
    {
      const std::size_t count = (std::min)(requested_workers, elements.size());
      // Build into temporaries: allocation/pool failure leaves this instance reusable.
      std::vector<std::unique_ptr<CoreTopologyLevelSetQuadrature>> candidate;
      for (std::size_t worker = 0; worker < count; ++worker)
      {
        candidate.push_back(std::make_unique<CoreTopologyLevelSetQuadrature>(0, &reference_rules));
      }
      auto candidate_pool = count > 1 ? std::make_unique<boost::asio::thread_pool>(count) : nullptr;
      quadratures.swap(candidate);
      pool = std::move(candidate_pool);
    }
    if (!cancelled.load(std::memory_order_relaxed) && all_cached)
    {
      for (auto& element : elements)
      {
        element.success = true;
      }
      completed.store(elements.size(), std::memory_order_relaxed);
    }
    else if (!cancelled.load(std::memory_order_relaxed) && quadratures.size() == 1)
    {
      for (auto& element : elements)
      {
        if (poll && !poll(completed.load(std::memory_order_relaxed)))
        {
          cancelled.store(true, std::memory_order_relaxed);
          break;
        }
        const auto serial_poll = [&]()
        {
          return !poll || poll(completed.load(std::memory_order_relaxed));
        };
        integrate_element(element, settings, *quadratures.front(), cancelled, serial_poll);
        if (!element.success)
        {
          cancelled.store(true, std::memory_order_relaxed);
          break;
        }
        completed.fetch_add(1, std::memory_order_relaxed);
      }
    }
    else if (!cancelled.load(std::memory_order_relaxed))
    {
      worker_failed.resize(quadratures.size(), 0);
      for (std::size_t worker = 0; worker < quadratures.size(); ++worker)
      {
        boost::asio::post(*pool, [&, worker]()
        {
          try
          {
            while (!cancelled.load(std::memory_order_relaxed))
            {
              const std::size_t index = next.fetch_add(1, std::memory_order_relaxed);
              if (index >= elements.size())
              {
                break;
              }
              integrate_element(elements[index], settings, *quadratures[worker], cancelled);
              if (!elements[index].success)
              {
                cancelled.store(true, std::memory_order_relaxed);
                break;
              }
              completed.fetch_add(1, std::memory_order_release);
            }
          }
          catch (...)
          {
            worker_failed[worker] = 1;
            cancelled.store(true, std::memory_order_relaxed);
          }
          finished.fetch_add(1, std::memory_order_acq_rel);
        });
        ++submitted;
      }
      while (finished.load(std::memory_order_acquire) < submitted)
      {
        if (poll && !poll(completed.load(std::memory_order_acquire)))
        {
          cancelled.store(true, std::memory_order_relaxed);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }
  }
  catch (const std::exception& failure)
  {
    cancelled.store(true, std::memory_order_relaxed);
    error = failure.what();
  }
  catch (...)
  {
    cancelled.store(true, std::memory_order_relaxed);
    error = "Unexpected topology integration scheduling/progress failure.";
  }
  // Includes partial submission and throwing progress callbacks. No task may
  // retain references to the stack or batch after this function returns.
  while (finished.load(std::memory_order_acquire) < submitted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  wall_ms += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
  for (const auto& element : elements)
  {
    worker_ms += element.integration_ms;
    if (!element.error.empty() && error.empty())
    {
      error = "Element " + std::to_string(element.id) + ": " + element.error;
    }
  }
  if (std::any_of(worker_failed.begin(), worker_failed.end(), [](unsigned char failed)
    {
      return failed != 0;
    }) && error.empty())
  {
    error = "Unexpected topology integration worker failure.";
  }
  if (cancelled.load(std::memory_order_relaxed) || completed.load(std::memory_order_acquire) != elements.size() || !error.empty())
  {
    if (error.empty())
    {
      error = "Topology integration interrupted or incomplete.";
    }
    return false;
  }
  try
  {
    if (poll && !poll(elements.size()))
    {
      error = "Topology integration interrupted.";
      return false;
    }
  }
  catch (...)
  {
    error = "Topology integration progress callback failed.";
    return false;
  }
  return true;
}

CoreTopologyLevelSetPipeline::Statistics CoreTopologyLevelSetPipeline::statistics() const
{
  Statistics result;
  result.wall_ms = wall_ms;
  result.worker_ms = worker_ms;
  result.preparation_ms = preparation_ms;
  result.workers = quadratures.size();
  result.rule_builds = reference_rules.build_count();
  result.retained_bytes = reference_rules.retained_bytes();
  result.preparation_fallbacks = reference_rules.fallback_count();
  for (const auto& quadrature : quadratures)
  {
    result.rule_hits += quadrature->hit_count();
    result.rule_fallbacks += quadrature->fallback_count();
  }
  return result;
}
