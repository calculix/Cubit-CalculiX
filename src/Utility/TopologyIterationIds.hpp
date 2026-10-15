#ifndef TOPOLOGYITERATIONIDS_HPP
#define TOPOLOGYITERATIONIDS_HPP

#include <limits>
#include <map>
#include <utility>

template<class Value>
bool valid_topology_iteration_keys(const std::map<int, Value>& values)
{
  return values.empty() || values.begin()->first >= 1;
}

// Only call on maps whose outer keys are iteration IDs.
template<class Value>
bool increment_topology_iteration_keys(std::map<int, Value>& values)
{
  for (const auto& entry : values)
  {
    if (entry.first < 0 || entry.first == std::numeric_limits<int>::max())
    {
      return false;
    }
  }

  std::map<int, Value> shifted;
  while (!values.empty())
  {
    auto node = values.extract(values.begin());
    ++node.key();
    shifted.insert(std::move(node));
  }
  values.swap(shifted);
  return true;
}

#endif // TOPOLOGYITERATIONIDS_HPP
