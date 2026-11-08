#ifndef CORETOPOLOGYLEVELSETQP_HPP
#define CORETOPOLOGYLEVELSETQP_HPP

#include "CoreTopologyLevelSetStructs.hpp"
#include "Eigen/Sparse"

/** @brief Internal numerical QP inputs; no mesh or Core model is owned. */
struct TopologyLevelSetQPProblem
{
  Eigen::SparseMatrix<double, Eigen::RowMajor> matrix;
  Eigen::VectorXd diagonal;
  Eigen::VectorXd origin;
  Eigen::VectorXd lower;
  Eigen::VectorXd upper;
  Eigen::VectorXd right;
  Eigen::VectorXd volume;
};

class CoreTopologyLevelSetQP
{
public:
  /**
   * @brief Solve the bounded convex quadratic field-update subproblem.
   * @return False with a Cubit error and empty/zero outputs on invalid data or
   * nonconvergence. Direct vector/output-scalar aliasing is rejected before
   * clearing, leaving the aliased inputs unchanged. Outputs must not alias
   * individual input entries or each other's storage.
   * @pre The symmetric matrix is positive definite. Production graph assembly
   * guarantees this through positive proximal masses; no generic SPD proof or
   * factorization is performed here. Stored diagonal must equal the matrix's.
   * @note Minimize 0.5*x^T*A*x-right^T*x with lower<=x<=upper. With c=volume,
   * constrain c^T*x<=c^T*origin+max(target-source,min_box c^T*(x-origin)).
   * The relaxed bound permits move-limited restoration, not necessarily target
   * feasibility. Signed volume derivatives are supported. Uses existing Core
   * limits and coordinate descent/multiplier bisection; no mesh query, physical
   * solve, proposal registration, backtracking or public run bypass is performed.
   * Returned stationarity residual is the diagonal-scaled projected residual,
   * not an independent KKT certificate or the source ordinary proposal residual.
   */
  static bool solve(const TopologyLevelSetQPProblem& problem, double source_volume_fraction, double target_volume_fraction, const TopologyLevelSetUpdateControls& controls, Eigen::VectorXd& values, double& multiplier, double& stationarity_residual);
};

#endif
