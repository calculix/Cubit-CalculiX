#include "CoreTopologyLevelSetQP.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
using Sparse = Eigen::SparseMatrix<double, Eigen::RowMajor>;
using Vector = Eigen::VectorXd;

double clamp(double value, double lower, double upper)
{
  return (std::max)(lower, (std::min)(upper, value));
}

bool solve_box(const TopologyLevelSetQPProblem& problem, double multiplier, const TopologyLevelSetUpdateControls& controls, Vector& values, double& residual)
{
  const Vector right = problem.right - multiplier * problem.volume;
  if (!right.allFinite())
  {
    PRINT_ERROR("Non-finite level-set volume multiplier right-hand side.\n");
    return false;
  }
  for (int sweep = 0; sweep < controls.maximum_coordinate_sweeps; ++sweep)
  {
    for (int row = 0; row < values.size(); ++row)
    {
      if (problem.lower[row] == problem.upper[row])
      {
        values[row] = problem.lower[row];
        continue;
      }
      double off_diagonal = 0.0;
      for (Sparse::InnerIterator entry(problem.matrix, row); entry; ++entry)
      {
        if (entry.col() != row)
        {
          off_diagonal += entry.value() * values[entry.col()];
        }
      }
      const double value = (right[row] - off_diagonal) / problem.diagonal[row];
      if (!std::isfinite(value))
      {
        PRINT_ERROR("Non-finite level-set QP coordinate.\n");
        return false;
      }
      values[row] = clamp(value, problem.lower[row], problem.upper[row]);
    }
    const Vector gradient = problem.matrix * values - right;
    if (!gradient.allFinite())
    {
      PRINT_ERROR("Non-finite level-set QP gradient.\n");
      return false;
    }
    residual = 0.0;
    for (int node = 0; node < values.size(); ++node)
    {
      const double projected = clamp(values[node] - gradient[node] / problem.diagonal[node], problem.lower[node], problem.upper[node]);
      residual = (std::max)(residual, std::abs(values[node] - projected));
    }
    if (residual <= controls.qp_tolerance)
    {
      return true;
    }
  }
  PRINT_ERROR("Level-set box QP did not converge within %d sweeps.\n", controls.maximum_coordinate_sweeps);
  return false;
}

bool solve_volume(const TopologyLevelSetQPProblem& problem, double source_volume, double target, const TopologyLevelSetUpdateControls& controls, Vector& values, double& multiplier, double& residual)
{
  values = problem.origin;
  multiplier = 0.0;
  double minimum_change = 0.0;
  for (int node = 0; node < values.size(); ++node)
  {
    const double endpoint = problem.volume[node] >= 0.0 ? problem.lower[node] : problem.upper[node];
    minimum_change += problem.volume[node] * (endpoint - problem.origin[node]);
  }
  if (!std::isfinite(minimum_change))
  {
    PRINT_ERROR("Non-finite linearized level-set volume bound.\n");
    return false;
  }
  if (source_volume > target + controls.volume_tolerance && minimum_change >= -controls.linear_volume_tolerance)
  {
    PRINT_ERROR("No resolved free linearized volume-reduction direction for the level-set design.\n");
    return false;
  }
  // Restoration can require several physical iterations because of the move limit.
  const double bound = problem.volume.dot(problem.origin) + (std::max)(target - source_volume, minimum_change);
  if (!std::isfinite(bound) || !solve_box(problem, 0.0, controls, values, residual))
  {
    return false;
  }
  double value = problem.volume.dot(values);
  if (!std::isfinite(value))
  {
    PRINT_ERROR("Non-finite linearized level-set volume.\n");
    return false;
  }
  if (value <= bound + controls.linear_volume_tolerance)
  {
    return true;
  }
  double lower = 0.0;
  double upper = 1.0;
  bool bracketed = false;
  for (int iteration = 0; iteration < controls.maximum_bracket_iterations; ++iteration)
  {
    if (!solve_box(problem, upper, controls, values, residual))
    {
      return false;
    }
    value = problem.volume.dot(values);
    if (!std::isfinite(value))
    {
      PRINT_ERROR("Non-finite bracketed level-set volume.\n");
      return false;
    }
    if (value <= bound + controls.linear_volume_tolerance)
    {
      bracketed = true;
      break;
    }
    lower = upper;
    upper *= 2.0;
  }
  if (!bracketed)
  {
    PRINT_ERROR("Unable to bracket the linearized level-set volume multiplier.\n");
    return false;
  }
  for (int iteration = 0; iteration < controls.maximum_multiplier_iterations; ++iteration)
  {
    multiplier = 0.5 * lower + 0.5 * upper;
    if (!solve_box(problem, multiplier, controls, values, residual))
    {
      return false;
    }
    value = problem.volume.dot(values);
    if (!std::isfinite(value))
    {
      PRINT_ERROR("Non-finite multiplier level-set volume.\n");
      return false;
    }
    if (std::abs(value - bound) <= controls.linear_volume_tolerance)
    {
      return true;
    }
    if (value > bound)
    {
      lower = multiplier;
    }
    else
    {
      upper = multiplier;
    }
  }
  PRINT_ERROR("Linearized level-set volume multiplier did not converge.\n");
  return false;
}
}

bool CoreTopologyLevelSetQP::solve(const TopologyLevelSetQPProblem& problem, double source_volume_fraction, double target_volume_fraction, const TopologyLevelSetUpdateControls& controls, Eigen::VectorXd& values, double& multiplier, double& stationarity_residual)
{
  if (&values == &problem.diagonal || &values == &problem.origin || &values == &problem.lower ||
      &values == &problem.upper || &values == &problem.right || &values == &problem.volume || &multiplier == &stationarity_residual)
  {
    PRINT_ERROR("Level-set QP outputs must not alias inputs or each other.\n");
    return false;
  }
  values.resize(0);
  multiplier = 0.0;
  stationarity_residual = 0.0;
  const auto count = problem.origin.size();
  if (count < 1 || count > (std::numeric_limits<int>::max)() || problem.matrix.rows() != count || problem.matrix.cols() != count ||
      problem.diagonal.size() != count || problem.lower.size() != count || problem.upper.size() != count ||
      problem.right.size() != count || problem.volume.size() != count ||
      !problem.origin.allFinite() || !problem.diagonal.allFinite() || !problem.lower.allFinite() || !problem.upper.allFinite() ||
      !problem.right.allFinite() || !problem.volume.allFinite() ||
      !std::isfinite(source_volume_fraction) || source_volume_fraction < 0.0 || source_volume_fraction > 1.0 ||
      !std::isfinite(target_volume_fraction) || target_volume_fraction <= 0.0 || target_volume_fraction > 1.0 ||
      controls.maximum_coordinate_sweeps < 1 || controls.maximum_bracket_iterations < 1 || controls.maximum_multiplier_iterations < 1 ||
      !std::isfinite(controls.qp_tolerance) || controls.qp_tolerance <= 0.0 ||
      !std::isfinite(controls.linear_volume_tolerance) || controls.linear_volume_tolerance <= 0.0 ||
      !std::isfinite(controls.volume_tolerance) || controls.volume_tolerance <= 0.0)
  {
    PRINT_ERROR("Invalid dimensions, numeric inputs or controls for level-set QP.\n");
    return false;
  }
  for (int row = 0; row < count; ++row)
  {
    if (problem.diagonal[row] <= 0.0 || problem.diagonal[row] != problem.matrix.coeff(row, row) ||
        problem.lower[row] > problem.upper[row] || problem.origin[row] < problem.lower[row] || problem.origin[row] > problem.upper[row])
    {
      PRINT_ERROR("Invalid level-set QP diagonal, bounds or source field.\n");
      return false;
    }
    for (Sparse::InnerIterator entry(problem.matrix, row); entry; ++entry)
    {
      const double transposed = problem.matrix.coeff(entry.col(), entry.row());
      const double scale = (std::max)(1.0, (std::max)(std::abs(entry.value()), std::abs(transposed)));
      if (!std::isfinite(entry.value()) || !std::isfinite(transposed) || std::abs(entry.value() - transposed) > 1.0e-12 * scale)
      {
        PRINT_ERROR("Non-finite or nonsymmetric level-set QP operator.\n");
        return false;
      }
    }
  }
  Vector candidate;
  double candidate_multiplier = 0.0;
  double candidate_residual = 0.0;
  if (!solve_volume(problem, source_volume_fraction, target_volume_fraction, controls, candidate, candidate_multiplier, candidate_residual))
  {
    return false;
  }
  if (!candidate.allFinite() || !std::isfinite(candidate_multiplier) || candidate_multiplier < 0.0 ||
      !std::isfinite(candidate_residual) || candidate_residual < 0.0 || candidate_residual > controls.qp_tolerance)
  {
    PRINT_ERROR("Invalid solved level-set QP outputs.\n");
    return false;
  }
  values = std::move(candidate);
  multiplier = candidate_multiplier;
  stationarity_residual = candidate_residual;
  return true;
}
