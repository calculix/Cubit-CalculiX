#ifndef TOPOLOGYITERATIONIDS_HPP
#define TOPOLOGYITERATIONIDS_HPP

#include <map>

template<class Value>
bool valid_topology_iteration_keys(const std::map<int, Value>& values)
{
  return values.empty() || values.begin()->first >= 1;
}

#endif // TOPOLOGYITERATIONIDS_HPP
