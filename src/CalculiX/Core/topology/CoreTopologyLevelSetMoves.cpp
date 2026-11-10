#include "CoreTopologyLevelSetMoves.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
  bool valid_field(const CoreTopologyLevelSetMoves::Field& field)
  {
    double previous_id = 0.0;
    for (const auto& row : field)
    {
      if (row.size() != 3 || !std::isfinite(row[0]) || row[0] <= previous_id ||
          row[0] > (std::numeric_limits<int>::max)() || std::floor(row[0]) != row[0] ||
          !std::isfinite(row[1]) || row[1] < -1.0 || row[1] > 1.0 ||
          (row[2] != 0.0 && row[2] != 1.0) || (row[2] == 1.0 && row[1] != 1.0))
      {
        return false;
      }
      previous_id = row[0];
    }
    return !field.empty();
  }
}

bool CoreTopologyLevelSetMoves::evaluate(int source_iteration, const Field& older, const Field& previous, const Field& current, bool source_feasible, const TopologyLevelSetMoveState& state, TopologyLevelSetMoveState& next, double& applied_scale)
{
  // Verification policy; these are not convergence tolerances.
  constexpr double reversal_threshold = -0.8;
  constexpr int required_reversals = 3;
  constexpr double reduction = 0.5;
  constexpr double minimum_scale = 0.1;

  if (&applied_scale == &state.scale || &applied_scale == &next.scale)
  {
    PRINT_ERROR("Level-set move scale output must not alias its state.\n");
    return false;
  }
  applied_scale = 1.0;
  if (&state == &next || state.last_source_iteration < 0 ||
      state.last_source_iteration >= (std::numeric_limits<int>::max)() ||
      source_iteration != state.last_source_iteration + 1 ||
      state.consecutive_reversals < 0 || state.consecutive_reversals >= required_reversals ||
      !std::isfinite(state.scale) || state.scale < minimum_scale || state.scale > 1.0 ||
      !valid_field(current))
  {
    PRINT_ERROR("Invalid level-set move-control state or field.\n");
    return false;
  }
  if ((source_iteration == 1 && !previous.empty()) ||
      (source_iteration < 3 && !older.empty()) ||
      (source_iteration >= 2 && (!valid_field(previous) || previous.size() != current.size())) ||
      (source_iteration >= 3 && (!valid_field(older) || older.size() != current.size())))
  {
    PRINT_ERROR("Incomplete level-set move-control history.\n");
    return false;
  }

  double product = 0.0;
  double current_norm_squared = 0.0;
  double previous_norm_squared = 0.0;
  for (std::size_t index = 0; index < current.size(); ++index)
  {
    const auto& row = current[index];
    if ((source_iteration >= 2 && (previous[index][0] != row[0] || previous[index][2] != row[2])) ||
        (source_iteration >= 3 && (older[index][0] != row[0] || older[index][2] != row[2])))
    {
      PRINT_ERROR("Level-set move-control node domains changed.\n");
      return false;
    }
    if (source_iteration < 3 || row[2] == 1.0)
    {
      continue;
    }
    const double update = row[1] - previous[index][1];
    const double preceding = previous[index][1] - older[index][1];
    product += update * preceding;
    current_norm_squared += update * update;
    previous_norm_squared += preceding * preceding;
  }

  auto candidate = state;
  candidate.last_source_iteration = source_iteration;
  candidate.consecutive_reversals = 0;
  if (source_feasible && source_iteration >= 3 && current_norm_squared > 0.0 && previous_norm_squared > 0.0)
  {
    const double cosine = product / (std::sqrt(current_norm_squared) * std::sqrt(previous_norm_squared));
    if (!std::isfinite(cosine))
    {
      PRINT_ERROR("Invalid level-set update-direction correlation.\n");
      return false;
    }
    if (cosine <= reversal_threshold)
    {
      candidate.consecutive_reversals = state.consecutive_reversals + 1;
      if (candidate.consecutive_reversals >= required_reversals)
      {
        candidate.scale = (std::max)(minimum_scale, state.scale * reduction);
        candidate.consecutive_reversals = 0;
      }
    }
  }
  applied_scale = source_feasible ? candidate.scale : 1.0;
  next = candidate;
  return true;
}
