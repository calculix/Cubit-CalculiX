#ifndef CORERESULTSVTKROWMAPS_HPP
#define CORERESULTSVTKROWMAPS_HPP

#include "CoreResultsVtkStructs.hpp"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <list>
#include <vector>

/** Caller-owned cache for one conversion. Domains are immutable and have unique
 * IDs during that conversion. Input source rows must be validated, uniquely
 * keyed and sorted. Only call layout()/mapping() between completed
 * pipeline.run() operations. Returned views must not escape the current field.
 * Vector payloads and estimated list-node overhead are charged to the budget;
 * existing reader/mesh indexes and allocator bookkeeping are not charged here.
 */
class CoreResultsVtkRowMaps
{
public:
  using Key = CoreResultsVtkPointKey;
  using Association = SimResults::Association;
  static constexpr std::size_t missing = std::numeric_limits<std::size_t>::max();

  struct Row
  {
    Key key;
    std::size_t data = 0;
  };

  struct Mapping
  {
    std::size_t domain = 0;
    std::vector<std::size_t> rows;
  };

  struct Layout
  {
    Association association = Association::Node;
    std::size_t hash = 0;
    std::vector<Row> rows;
    std::list<Mapping> mappings;
  };

  explicit CoreResultsVtkRowMaps(std::size_t budget) : budget(budget)
  {
  }

  CoreResultsVtkRowMaps(const CoreResultsVtkRowMaps&) = delete;
  CoreResultsVtkRowMaps& operator=(const CoreResultsVtkRowMaps&) = delete;

  std::size_t bytes() const
  {
    return used;
  }

  Layout* layout(const std::vector<Row>& rows, Association association)
  {
    if (budget < layout_overhead || rows.size() > (budget - layout_overhead) / sizeof(Row))
    {
      return nullptr;
    }
    std::size_t hash = static_cast<std::size_t>(association);
    const auto combine = [&](std::size_t value)
    {
      hash ^= value + static_cast<std::size_t>(0x9e3779b9) + (hash << 6) + (hash >> 2);
    };
    for (const auto& row : rows)
    {
      combine(std::hash<int>{}(row.key.entity_id));
      combine(std::hash<int>{}(row.key.integration_point));
      combine(std::hash<std::size_t>{}(row.data));
    }
    for (auto entry = layouts.begin(); entry != layouts.end(); ++entry)
    {
      if (entry->association == association && entry->hash == hash && entry->rows.size() == rows.size() &&
        std::equal(rows.begin(), rows.end(), entry->rows.begin(), [](const Row& a, const Row& b)
        {
          return a.key == b.key && a.data == b.data;
        }))
      {
        layouts.splice(layouts.begin(), layouts, entry);
        return &layouts.front();
      }
    }
    const auto requested = layout_overhead + rows.size() * sizeof(Row);
    if (!room(requested, nullptr))
    {
      return nullptr;
    }
    layouts.emplace_front();
    auto& entry = layouts.front();
    entry.association = association;
    entry.hash = hash;
    entry.rows.assign(rows.begin(), rows.end());
    const auto actual = layout_overhead + entry.rows.capacity() * sizeof(Row);
    if (actual > budget - used)
    {
      layouts.pop_front();
      return nullptr;
    }
    used += actual;
    return &entry;
  }

  // Dense ordered destinations use a merge; sparse/unsorted destinations use
  // binary search only during map construction, never while copying values.
  template<class KeyAt>
  const std::vector<std::size_t>* mapping(Layout* layout, std::size_t domain,
    std::size_t count, KeyAt key_at)
  {
    if (!layout)
    {
      return nullptr;
    }
    for (auto entry = layout->mappings.begin(); entry != layout->mappings.end(); ++entry)
    {
      if (entry->domain == domain && entry->rows.size() == count)
      {
        layout->mappings.splice(layout->mappings.begin(), layout->mappings, entry);
        return &layout->mappings.front().rows;
      }
    }
    if (budget < map_overhead || count > (budget - map_overhead) / sizeof(std::size_t) ||
      !room(map_overhead + count * sizeof(std::size_t), layout))
    {
      return nullptr;
    }
    layout->mappings.emplace_front();
    auto& entry = layout->mappings.front();
    entry.domain = domain;
    entry.rows.resize(count, missing);
    const auto actual = map_overhead + entry.rows.capacity() * sizeof(std::size_t);
    if (actual > budget - used)
    {
      layout->mappings.pop_front();
      return nullptr;
    }
    used += actual;
    bool ordered = count >= layout->rows.size() / 8;
    for (std::size_t i = 1; ordered && i < count; ++i)
    {
      ordered = !(key_at(i) < key_at(i - 1));
    }
    auto source = layout->rows.begin();
    for (std::size_t i = 0; i < count; ++i)
    {
      const auto key = key_at(i);
      if (ordered)
      {
        while (source != layout->rows.end() && source->key < key)
        {
          ++source;
        }
      }
      else
      {
        source = std::lower_bound(layout->rows.begin(), layout->rows.end(), key, [](const Row& row, const Key& value)
        {
          return row.key < value;
        });
      }
      if (source != layout->rows.end() && source->key == key)
      {
        entry.rows[i] = source->data;
      }
    }
    return &entry.rows;
  }

private:
  static constexpr std::size_t layout_overhead = sizeof(Layout) + 2 * sizeof(void*);
  static constexpr std::size_t map_overhead = sizeof(Mapping) + 2 * sizeof(void*);
  std::size_t budget = 0;
  std::size_t used = 0;
  std::list<Layout> layouts;

  bool room(std::size_t requested, Layout* keep)
  {
    if (requested > budget)
    {
      return false;
    }
    while (requested > budget - used)
    {
      auto victim = layouts.end();
      for (auto entry = layouts.begin(); entry != layouts.end(); ++entry)
      {
        if (&*entry != keep)
        {
          victim = entry;
        }
      }
      if (victim != layouts.end())
      {
        used -= layout_overhead + victim->rows.capacity() * sizeof(Row);
        for (const auto& mapping : victim->mappings)
        {
          used -= map_overhead + mapping.rows.capacity() * sizeof(std::size_t);
        }
        layouts.erase(victim);
      }
      else if (keep && !keep->mappings.empty())
      {
        used -= map_overhead + keep->mappings.back().rows.capacity() * sizeof(std::size_t);
        keep->mappings.pop_back();
      }
      else
      {
        return false;
      }
    }
    return true;
  }
};

#endif
