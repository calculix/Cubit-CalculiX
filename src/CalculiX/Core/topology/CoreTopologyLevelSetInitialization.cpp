#include "CoreTopologyLevelSetInitialization.hpp"
#include "CoreTopologyLevelSet.hpp"
#include "CoreTopologyLevelSetGeometry.hpp"
#include "CoreTopologyStructs.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <utility>

bool CoreTopologyLevelSetInitialization::resolve_settings(const TopologyLevelSetSettings& requested, const std::map<int, double>& element_volumes, TopologyLevelSetSettings& resolved, double& characteristic_length)
{
  resolved = {};
  characteristic_length = 0.0;
  if (!CoreTopologyLevelSet::validate_settings(requested) || element_volumes.empty())
  {
    PRINT_ERROR("Level-set length resolution requires valid settings and Cubit volumes.\n");
    return false;
  }
  double total = 0.0;
  for (const auto& entry : element_volumes)
  {
    if (entry.first <= 0 || !std::isfinite(entry.second) || entry.second <= 0.0)
    {
      PRINT_ERROR("Invalid Cubit element volume for level-set initialization.\n");
      return false;
    }
    total += entry.second;
  }
  const double h = std::cbrt(total / static_cast<double>(element_volumes.size()));
  if (!std::isfinite(total) || !std::isfinite(h) || h <= 0.0)
  {
    PRINT_ERROR("Invalid characteristic length for level-set initialization.\n");
    return false;
  }
  auto candidate = requested;
  if (candidate.regularization_length == 0.0)
  {
    candidate.regularization_length = 1.5 * h;
  }
  if (candidate.seed_spacing == 0.0)
  {
    candidate.seed_spacing = 6.0 * h;
  }
  if (candidate.seed_radius == 0.0)
  {
    candidate.seed_radius = 0.25 * candidate.seed_spacing;
  }
  if (!CoreTopologyLevelSet::validate_settings(candidate) || candidate.regularization_length <= 0.0 || candidate.seed_spacing <= 0.0 || candidate.seed_radius <= 0.0)
  {
    PRINT_ERROR("Unable to resolve consistent positive level-set lengths.\n");
    return false;
  }
  resolved = candidate;
  characteristic_length = h;
  return true;
}

bool CoreTopologyLevelSetInitialization::initialize(const TopologyLevelSetSettings& settings, double void_ratio, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, TopologyLevelSetDesign& design)
{
  design = {};
  if (!std::isfinite(void_ratio) || void_ratio <= 0.0 || void_ratio >= 1.0 || preserved.size() != element_volumes.size())
  {
    PRINT_ERROR("Invalid level-set initialization domain or void ratio.\n");
    return false;
  }
  TopologyLevelSetDesign candidate;
  if (!resolve_settings(settings, element_volumes, candidate.settings, candidate.characteristic_length))
  {
    return false;
  }
  std::map<int, std::array<double, 3>> coordinates;
  for (const auto& entry : element_volumes)
  {
    const auto flag = preserved.find(entry.first);
    if (flag == preserved.end())
    {
      PRINT_ERROR("Missing level-set preservation flag for element %d.\n", entry.first);
      return false;
    }
    std::string type;
    int cubit_type = -1;
    std::vector<int> nodes;
    std::vector<std::array<double, 3>> points;
    if (!CoreTopologyLevelSetGeometry::query_element(entry.first, type, cubit_type, nodes, points))
    {
      return false;
    }
    for (std::size_t node = 0; node < nodes.size(); ++node)
    {
      const auto inserted = coordinates.emplace(nodes[node], points[node]);
      if (!inserted.second && inserted.first->second != points[node])
      {
        PRINT_ERROR("Cubit mesh changed during level-set initialization.\n");
        return false;
      }
      if (flag->second)
      {
        candidate.fixed_nodes.insert(nodes[node]);
      }
    }
  }
  if (coordinates.empty() || coordinates.size() == candidate.fixed_nodes.size())
  {
    PRINT_ERROR("Level-set preservation leaves no free nodal design variables.\n");
    return false;
  }
  auto lower = coordinates.begin()->second;
  auto upper = lower;
  for (const auto& entry : coordinates)
  {
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      lower[axis] = (std::min)(lower[axis], entry.second[axis]);
      upper[axis] = (std::max)(upper[axis], entry.second[axis]);
    }
  }
  std::array<double, 3> center{};
  for (std::size_t axis = 0; axis < 3; ++axis)
  {
    center[axis] = 0.5 * lower[axis] + 0.5 * upper[axis];
  }
  int anchor = -1;
  double nearest = (std::numeric_limits<double>::max)();
  for (const auto& entry : coordinates)
  {
    if (candidate.fixed_nodes.count(entry.first) != 0)
    {
      continue;
    }
    const double distance = std::hypot(entry.second[0] - center[0], entry.second[1] - center[1], entry.second[2] - center[2]);
    if (std::isfinite(distance) && distance < nearest)
    {
      nearest = distance;
      anchor = entry.first;
      candidate.seed_origin = entry.second;
    }
  }
  if (anchor <= 0)
  {
    PRINT_ERROR("Unable to locate a finite free level-set seed anchor.\n");
    return false;
  }
  bool transition = false;
  int fallback = -1;
  double closest = (std::numeric_limits<double>::max)();
  for (const auto& entry : coordinates)
  {
    double value = 1.0;
    if (candidate.fixed_nodes.count(entry.first) == 0)
    {
      std::array<double, 3> delta{};
      for (std::size_t axis = 0; axis < 3; ++axis)
      {
        delta[axis] = std::remainder(entry.second[axis] - candidate.seed_origin[axis], candidate.settings.seed_spacing);
      }
      const double distance = std::hypot(delta[0], delta[1], delta[2]);
      const double scaled = (distance - candidate.settings.seed_radius) / candidate.settings.seed_radius;
      if (!std::isfinite(scaled))
      {
        PRINT_ERROR("Invalid periodic level-set seed distance.\n");
        return false;
      }
      value = (std::max)(-1.0, (std::min)(1.0, scaled));
      transition = transition || std::abs(value) < candidate.settings.interface_width;
      if (entry.first != anchor && std::abs(value) < closest)
      {
        closest = std::abs(value);
        fallback = entry.first;
      }
    }
    candidate.node_values.emplace(entry.first, value);
  }
  if (!transition)
  {
    candidate.node_values.at(fallback > 0 ? fallback : anchor) = 0.0;
    candidate.transition_node_inserted = true;
  }
  candidate.iteration_id = 1;
  candidate.source_iteration = -1;
  candidate.void_ratio = void_ratio;
  design = std::move(candidate);
  return true;
}
