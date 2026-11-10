#include "CoreMaterialsTopologyGrouping.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

bool CoreMaterialsTopologyGrouping::apply(const std::map<int, int>& element_base_material, const std::map<int, double>& requested, const std::map<int, double>* previous, int material_cap, double minimum_factor, std::map<int, double>& applied)
{
  constexpr double hysteresis_fraction = 0.1;
  constexpr double palette_tolerance = 2.0e-12;
  if (&applied == &requested || previous == &applied)
  {
    PRINT_ERROR("Topology grouping outputs must not alias input factors.\n");
    return false;
  }
  applied.clear();
  const auto fail = [](const char* message)
  {
    PRINT_ERROR("%s\n", message);
    return false;
  };
  if (material_cap < 2 || requested.empty() || requested.size() != element_base_material.size() ||
      requested.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)()) ||
      !std::isfinite(minimum_factor) || minimum_factor <= 0.0 || minimum_factor >= 1.0 ||
      (previous && previous->size() != requested.size()))
  {
    return fail("Invalid stable topology grouping request.");
  }
  std::map<int, int> counts;
  for (const auto& assignment : element_base_material)
  {
    const auto value = requested.find(assignment.first);
    if (assignment.first <= 0 || assignment.second <= 0 || value == requested.end() ||
        !std::isfinite(value->second) || value->second < minimum_factor || value->second > 1.0)
    {
      return fail("Invalid topology grouping domain or requested factor.");
    }
    if (previous)
    {
      const auto old = previous->find(assignment.first);
      if (old == previous->end() || !std::isfinite(old->second) || old->second < minimum_factor || old->second > 1.0)
      {
        return fail("Invalid previous topology factor domain.");
      }
    }
    ++counts[assignment.second];
  }

  std::map<int, int> budgets;
  std::size_t reserved = 0;
  for (const auto& entry : counts)
  {
    const int budget = (std::min)(entry.second, 2);
    budgets.emplace(entry.first, budget);
    reserved += static_cast<std::size_t>(budget);
  }
  if (reserved > static_cast<std::size_t>(material_cap))
  {
    return fail("Topology material cap cannot accommodate the base-material palettes.");
  }
  int remaining = material_cap - static_cast<int>(reserved);
  while (remaining > 0)
  {
    bool allocated = false;
    for (auto& entry : budgets)
    {
      if (entry.second < counts.at(entry.first))
      {
        ++entry.second;
        --remaining;
        allocated = true;
        if (remaining == 0)
        {
          break;
        }
      }
    }
    if (!allocated)
    {
      break;
    }
  }

  std::map<int, double> candidate;
  for (const auto& assignment : element_base_material)
  {
    const double value = requested.at(assignment.first);
    const int budget = budgets.at(assignment.second);
    if (budget >= counts.at(assignment.second))
    {
      candidate.emplace(assignment.first, value);
      continue;
    }
    const double spacing = (1.0 - minimum_factor) / (budget - 1);
    const auto level = [&](int index)
    {
      if (index == 0)
      {
        return minimum_factor;
      }
      if (index == budget - 1)
      {
        return 1.0;
      }
      return minimum_factor + spacing * index;
    };
    const auto nearest = [&](double factor)
    {
      return std::clamp(static_cast<int>(std::llround((factor - minimum_factor) / spacing)), 0, budget - 1);
    };
    int index = nearest(value);
    if (previous)
    {
      const double old = previous->at(assignment.first);
      const int old_index = nearest(old);
      if (std::abs(old - level(old_index)) > palette_tolerance)
      {
        return fail("Previous topology factor does not match the stable palette.");
      }
      const double lower = old_index == 0 ? minimum_factor : 0.5 * (level(old_index - 1) + old);
      const double upper = old_index == budget - 1 ? 1.0 : 0.5 * (old + level(old_index + 1));
      const double margin = hysteresis_fraction * spacing;
      if (value >= lower - margin && value <= upper + margin)
      {
        index = old_index;
      }
    }
    if (value == minimum_factor)
    {
      index = 0;
    }
    else if (value == 1.0)
    {
      index = budget - 1;
    }
    candidate.emplace(assignment.first, level(index));
  }
  applied = std::move(candidate);
  return true;
}
