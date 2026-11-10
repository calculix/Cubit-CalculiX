#ifndef CORETOPOLOGYLEVELSETMOVES_HPP
#define CORETOPOLOGYLEVELSETMOVES_HPP

#include <vector>

/** @brief Transient state belonging to one active topology run. */
struct TopologyLevelSetMoveState
{
  int last_source_iteration = 0;
  int consecutive_reversals = 0;
  double scale = 1.0;
};

/**
 * @brief Select damping from consecutive accepted field updates.
 * @note Field rows are sorted [global node ID, phi, fixed]. Fixed coefficients
 * are excluded from the direction cosine. Three consecutive cosines <= -0.8
 * halve the scale, down to 0.1. Zero updates reset the reversal streak.
 * Infeasible sources use scale 1 for restoration and reset the streak without
 * increasing the retained adaptive scale. There is no automatic scale increase.
 * This is a heuristic, not a convergence certificate. The ordinary full-move QP
 * proposal and its convergence residual must remain unchanged.
 * @return False with a Cubit error on invalid history/state; next is unchanged
 * and applied_scale is 1. Scale/state aliasing is rejected without touching
 * aliased outputs. On success the
 * caller commits next only after preparing the next design successfully.
 */
class CoreTopologyLevelSetMoves
{
public:
  using Field = std::vector<std::vector<double>>;

  static bool evaluate(int source_iteration, const Field& older, const Field& previous, const Field& current, bool source_feasible, const TopologyLevelSetMoveState& state, TopologyLevelSetMoveState& next, double& applied_scale);
};

#endif
