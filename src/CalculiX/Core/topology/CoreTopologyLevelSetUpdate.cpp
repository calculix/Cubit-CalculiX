#include "CoreTopologyLevelSetUpdate.hpp"
#include "CoreTopologyLevelSet.hpp"
#include "CoreTopologyLevelSetEvaluation.hpp"
#include "CoreTopologyLevelSetGeometry.hpp"
#include "CoreTopologyStructs.hpp"
#include "CubitMessage.hpp"
#include "Eigen/Sparse"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
using Sparse = Eigen::SparseMatrix<double, Eigen::RowMajor>;
using Vector = Eigen::VectorXd;

struct Problem
{
  Sparse matrix;
  Vector diagonal;
  Vector mass;
  Vector origin;
  Vector lower;
  Vector upper;
  Vector right;
  Vector volume;
  std::map<int, int> indices;
};

double clamp(double value, double lower, double upper)
{
  return (std::max)(lower, (std::min)(upper, value));
}

bool build_problem(const TopologyLevelSetDesign& source, const TopologyLevelSetEvaluation& evaluation, const std::map<int, double>& volumes, double objective_scale, Problem& problem)
{
  if (source.node_values.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)()))
  {
    PRINT_ERROR("Too many level-set nodes for sparse operator indexing.\n");
    return false;
  }
  const int count = static_cast<int>(source.node_values.size());
  problem.mass = Vector::Zero(count);
  problem.origin.resize(count);
  problem.lower.resize(count);
  problem.upper.resize(count);
  problem.right.resize(count);
  problem.volume.resize(count);
  int index = 0;
  for (const auto& node : source.node_values)
  {
    const auto objective = evaluation.compliance_derivatives.find(node.first);
    const auto volume = evaluation.volume_fraction_derivatives.find(node.first);
    if (objective == evaluation.compliance_derivatives.end() || volume == evaluation.volume_fraction_derivatives.end() ||
        !std::isfinite(objective->second) || !std::isfinite(volume->second))
    {
      PRINT_ERROR("Incomplete/non-finite level-set update derivatives.\n");
      return false;
    }
    const bool fixed = source.fixed_nodes.count(node.first) != 0;
    if (fixed && (objective->second != 0.0 || volume->second != 0.0))
    {
      PRINT_ERROR("Fixed level-set nodes must have zero feasible derivatives.\n");
      return false;
    }
    problem.indices.emplace(node.first, index);
    problem.origin[index] = node.second;
    problem.lower[index] = fixed ? 1.0 : (std::max)(-1.0, node.second - source.settings.move_limit);
    problem.upper[index] = fixed ? 1.0 : (std::min)(1.0, node.second + source.settings.move_limit);
    problem.right[index] = -objective->second / objective_scale;
    problem.volume[index] = volume->second;
    ++index;
  }
  std::vector<Eigen::Triplet<double>> entries;
  for (const auto& element : volumes)
  {
    std::string type;
    int cubit_type = -1;
    std::vector<int> nodes;
    std::vector<std::array<double, 3>> coordinates;
    if (!CoreTopologyLevelSetGeometry::query_element(element.first, type, cubit_type, nodes, coordinates))
    {
      return false;
    }
    const double normalized_volume = element.second / evaluation.total_volume;
    const double pairs = 0.5 * static_cast<double>(nodes.size()) * static_cast<double>(nodes.size() - 1);
    for (std::size_t a = 0; a < nodes.size(); ++a)
    {
      const auto first = problem.indices.find(nodes[a]);
      if (first == problem.indices.end())
      {
        PRINT_ERROR("Cubit connectivity changed during level-set operator assembly.\n");
        return false;
      }
      const int row = first->second;
      problem.mass[row] += normalized_volume / static_cast<double>(nodes.size());
      for (std::size_t b = a + 1; b < nodes.size(); ++b)
      {
        const auto second = problem.indices.find(nodes[b]);
        if (second == problem.indices.end())
        {
          PRINT_ERROR("Missing level-set graph node.\n");
          return false;
        }
        const double distance = std::hypot(coordinates[a][0] - coordinates[b][0], coordinates[a][1] - coordinates[b][1], coordinates[a][2] - coordinates[b][2]);
        if (!std::isfinite(distance) || distance <= 0.0)
        {
          PRINT_ERROR("Coincident/non-finite nodes in the level-set regularization graph.\n");
          return false;
        }
        const double ratio = source.settings.regularization_length / distance;
        const double weight = (normalized_volume / pairs) * ratio * ratio;
        if (!std::isfinite(weight) || weight < 0.0)
        {
          PRINT_ERROR("Invalid level-set graph regularization weight.\n");
          return false;
        }
        const int column = second->second;
        entries.emplace_back(row, row, weight);
        entries.emplace_back(column, column, weight);
        entries.emplace_back(row, column, -weight);
        entries.emplace_back(column, row, -weight);
      }
    }
  }
  problem.matrix.resize(count, count);
  problem.matrix.setFromTriplets(entries.begin(), entries.end());
  for (int node = 0; node < count; ++node)
  {
    if (!std::isfinite(problem.mass[node]) || problem.mass[node] <= 0.0)
    {
      PRINT_ERROR("Invalid level-set proximal mass.\n");
      return false;
    }
    const double regularization_diagonal = problem.matrix.coeff(node, node);
    problem.matrix.coeffRef(node, node) += problem.mass[node];
    if (problem.matrix.coeff(node, node) <= regularization_diagonal)
    {
      PRINT_ERROR("Level-set graph scaling loses the positive proximal mass.\n");
      return false;
    }
    problem.right[node] += problem.mass[node] * problem.origin[node];
    if (!std::isfinite(problem.right[node]))
    {
      PRINT_ERROR("Non-finite normalized level-set objective derivative.\n");
      return false;
    }
  }
  problem.matrix.makeCompressed();
  problem.diagonal = problem.matrix.diagonal();
  for (int row = 0; row < count; ++row)
  {
    if (!std::isfinite(problem.diagonal[row]) || problem.diagonal[row] <= 0.0)
    {
      PRINT_ERROR("Invalid level-set QP diagonal.\n");
      return false;
    }
    for (Sparse::InnerIterator entry(problem.matrix, row); entry; ++entry)
    {
      if (!std::isfinite(entry.value()))
      {
        PRINT_ERROR("Non-finite level-set sparse operator.\n");
        return false;
      }
    }
  }
  return true;
}

bool solve_box(const Problem& problem, double multiplier, const TopologyLevelSetUpdateControls& controls, Vector& values, double& residual)
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

bool solve_proposal(const Problem& problem, double source_volume, double target, const TopologyLevelSetUpdateControls& controls, Vector& values, double& multiplier, double& residual)
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

bool CoreTopologyLevelSetUpdate::build_proposal(const TopologyLevelSetDesign& source, const TopologyLevelSetEvaluation& evaluation, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, double target_volume_fraction, TopologyLevelSetProposal& proposal)
{
  proposal = {};
  const auto controls = CoreTopologyLevelSet::default_update_controls();
  if (!evaluation.available || evaluation.iteration_id != source.iteration_id || source.iteration_id < 1 ||
      !source.volume.available ||
      !std::isfinite(target_volume_fraction) || target_volume_fraction <= 0.0 || target_volume_fraction > 1.0 ||
      !std::isfinite(evaluation.compliance) || evaluation.compliance < 0.0 ||
      !std::isfinite(evaluation.total_volume) || evaluation.total_volume <= 0.0 ||
      !std::isfinite(evaluation.volume_fraction) || evaluation.volume_fraction < 0.0 || evaluation.volume_fraction > 1.0 ||
      evaluation.compliance_derivatives.size() != source.node_values.size() ||
      evaluation.volume_fraction_derivatives.size() != source.node_values.size() ||
      !std::isfinite(source.objective_scale) || source.objective_scale < 0.0)
  {
    PRINT_ERROR("Invalid solved source data or target for level-set update.\n");
    return false;
  }
  auto prepared = source;
  if (!CoreTopologyLevelSetEvaluation::build_geometry(prepared, preserved, element_volumes))
  {
    return false;
  }
  if (std::abs(prepared.volume.volume_fraction - evaluation.volume_fraction) > controls.volume_tolerance ||
      std::abs(prepared.volume.total_volume - evaluation.total_volume) / evaluation.total_volume > controls.volume_tolerance)
  {
    PRINT_ERROR("Level-set source evaluation disagrees with the current geometry.\n");
    return false;
  }
  if (prepared.volume.minimum_design_volume_fraction > target_volume_fraction + controls.volume_tolerance)
  {
    PRINT_ERROR("Level-set target volume is below the fully pinned-element lower bound.\n");
    return false;
  }
  const double objective_scale = source.objective_scale > 0.0 ? source.objective_scale : evaluation.compliance > 0.0 ? evaluation.compliance : 1.0;
  Problem problem;
  if (!build_problem(prepared, evaluation, element_volumes, objective_scale, problem))
  {
    return false;
  }
  Vector values;
  double multiplier = 0.0;
  double residual = 0.0;
  const double source_volume = prepared.volume.volume_fraction;
  if (!solve_proposal(problem, source_volume, target_volume_fraction, controls, values, multiplier, residual))
  {
    return false;
  }
  const Vector direction = values - problem.origin;
  const double ordinary_residual = direction.cwiseAbs().maxCoeff();
  const double predicted_change = problem.volume.dot(direction);
  if (!direction.allFinite() || !std::isfinite(ordinary_residual) || !std::isfinite(predicted_change))
  {
    PRINT_ERROR("Invalid level-set proposal direction.\n");
    return false;
  }
  TopologyLevelSetProposal candidate;
  candidate.available = true;
  candidate.source_iteration = source.iteration_id;
  candidate.objective_scale = objective_scale;
  candidate.source_volume_fraction = source_volume;
  candidate.volume_fraction_derivatives = evaluation.volume_fraction_derivatives;
  for (const auto& node : problem.indices)
  {
    candidate.node_values.emplace(node.first, values[node.second]);
  }
  candidate.diagnostics.available = true;
  candidate.diagnostics.source_iteration = source.iteration_id;
  candidate.diagnostics.target_volume_fraction = target_volume_fraction;
  candidate.diagnostics.actual_volume_fraction = source_volume;
  candidate.diagnostics.volume_restoration = source_volume > target_volume_fraction + controls.volume_tolerance;
  candidate.diagnostics.volume_feasible = !candidate.diagnostics.volume_restoration;
  candidate.diagnostics.ordinary_proposal_residual = ordinary_residual;
  candidate.diagnostics.qp_stationarity_residual = residual;
  candidate.diagnostics.volume_multiplier = multiplier;
  candidate.diagnostics.linearized_volume_fraction = source_volume + predicted_change;
  proposal = std::move(candidate);
  return true;
}

bool CoreTopologyLevelSetUpdate::accept_proposal(const TopologyLevelSetDesign& source, const TopologyLevelSetProposal& proposal, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, TopologyLevelSetDesign& next, std::map<int, double>& factors, TopologyLevelSetUpdateDiagnostics& diagnostics)
{
  if (&next == &source || &factors == &source.node_values || &factors == &source.element_solid_fractions ||
      &factors == &proposal.node_values || &factors == &proposal.volume_fraction_derivatives || &factors == &element_volumes ||
      &factors == &next.node_values || &factors == &next.element_solid_fractions || &factors == &source.volume.density_sensitivities ||
      &factors == &next.volume.density_sensitivities || &diagnostics == &source.incoming_update || &diagnostics == &next.incoming_update || &diagnostics == &proposal.diagnostics)
  {
    PRINT_ERROR("Level-set acceptance outputs must not alias input records or each other.\n");
    return false;
  }
  next = {};
  factors.clear();
  diagnostics = {};
  const auto controls = CoreTopologyLevelSet::default_update_controls();
  const double target_volume_fraction = proposal.diagnostics.target_volume_fraction;
  if (!proposal.available || !proposal.diagnostics.available || proposal.source_iteration != source.iteration_id ||
      proposal.diagnostics.source_iteration != source.iteration_id || source.iteration_id < 1 || source.iteration_id == (std::numeric_limits<int>::max)() ||
      proposal.node_values.size() != source.node_values.size() || proposal.volume_fraction_derivatives.size() != source.node_values.size() ||
      !std::isfinite(proposal.objective_scale) || proposal.objective_scale <= 0.0 ||
      !std::isfinite(proposal.source_volume_fraction) || proposal.source_volume_fraction < 0.0 || proposal.source_volume_fraction > 1.0 ||
      !std::isfinite(target_volume_fraction) || target_volume_fraction <= 0.0 || target_volume_fraction > 1.0 ||
      !std::isfinite(proposal.diagnostics.qp_stationarity_residual) || proposal.diagnostics.qp_stationarity_residual < 0.0 || proposal.diagnostics.qp_stationarity_residual > controls.qp_tolerance ||
      !std::isfinite(proposal.diagnostics.volume_multiplier) || proposal.diagnostics.volume_multiplier < 0.0)
  {
    PRINT_ERROR("Invalid or stale level-set proposal.\n");
    return false;
  }
  auto prepared = source;
  if (!CoreTopologyLevelSetEvaluation::build_geometry(prepared, preserved, element_volumes))
  {
    return false;
  }
  const double source_volume = prepared.volume.volume_fraction;
  if (std::abs(source_volume - proposal.source_volume_fraction) > controls.volume_tolerance ||
      prepared.volume.minimum_design_volume_fraction > target_volume_fraction + controls.volume_tolerance)
  {
    PRINT_ERROR("Level-set proposal no longer matches the source volume/target.\n");
    return false;
  }
  double ordinary_residual = 0.0;
  for (const auto& node : source.node_values)
  {
    const auto value = proposal.node_values.find(node.first);
    const auto volume = proposal.volume_fraction_derivatives.find(node.first);
    const bool fixed = source.fixed_nodes.count(node.first) != 0;
    const double lower = fixed ? 1.0 : (std::max)(-1.0, node.second - source.settings.move_limit);
    const double upper = fixed ? 1.0 : (std::min)(1.0, node.second + source.settings.move_limit);
    if (value == proposal.node_values.end() || volume == proposal.volume_fraction_derivatives.end() ||
        !std::isfinite(value->second) || value->second < lower || value->second > upper || !std::isfinite(volume->second) || (fixed && volume->second != 0.0))
    {
      PRINT_ERROR("Invalid level-set proposal field/bounds.\n");
      return false;
    }
    ordinary_residual = (std::max)(ordinary_residual, std::abs(value->second - node.second));
  }
  if (!std::isfinite(proposal.diagnostics.ordinary_proposal_residual) ||
      std::abs(ordinary_residual - proposal.diagnostics.ordinary_proposal_residual) > controls.qp_tolerance)
  {
    PRINT_ERROR("Level-set proposal residual disagrees with its field.\n");
    return false;
  }
  const double objective_scale = proposal.objective_scale;
  const bool restoring = source_volume > target_volume_fraction + controls.volume_tolerance;
  double scale = 1.0;
  for (int backtrack = 0; backtrack <= controls.maximum_backtracking_steps; ++backtrack)
  {
    auto candidate = prepared;
    candidate.iteration_id = source.iteration_id + 1;
    candidate.source_iteration = source.iteration_id;
    candidate.objective_scale = objective_scale;
    candidate.incoming_update = {};
    double maximum_change = 0.0;
    double linear_change = 0.0;
    for (const auto& node : source.node_values)
    {
      const bool fixed = source.fixed_nodes.count(node.first) != 0;
      const double lower = fixed ? 1.0 : (std::max)(-1.0, node.second - source.settings.move_limit);
      const double upper = fixed ? 1.0 : (std::min)(1.0, node.second + source.settings.move_limit);
      const double value = clamp(node.second + scale * (proposal.node_values.at(node.first) - node.second), lower, upper);
      candidate.node_values.at(node.first) = value;
      maximum_change = (std::max)(maximum_change, std::abs(value - node.second));
      linear_change += proposal.volume_fraction_derivatives.at(node.first) * (value - node.second);
    }
    if (!std::isfinite(linear_change))
    {
      PRINT_ERROR("Non-finite accepted-field linearized volume change.\n");
      return false;
    }
    if (!CoreTopologyLevelSetEvaluation::build_geometry(candidate, preserved, element_volumes))
    {
      return false;
    }
    const double measured = candidate.volume.volume_fraction;
    const bool feasible = measured <= target_volume_fraction + controls.volume_tolerance;
    bool accept = feasible;
    if (restoring && !feasible)
    {
      const double expected = -linear_change;
      const double required = (std::min)(controls.restoration_fraction * expected, 0.5 * (source_volume - target_volume_fraction));
      const double resolved = (std::min)(0.1 * controls.volume_tolerance, 0.5 * (source_volume - target_volume_fraction));
      accept = expected > 0.0 && required > 0.0 && measured <= source_volume - required && measured <= source_volume - resolved;
    }
    if (accept)
    {
      std::map<int, double> requested;
      if (!CoreTopologyLevelSetEvaluation::build_factors(candidate, preserved, requested))
      {
        return false;
      }
      TopologyLevelSetUpdateDiagnostics information;
      information.available = true;
      information.source_iteration = source.iteration_id;
      information.volume_restoration = restoring;
      information.volume_feasible = feasible;
      information.ordinary_proposal_residual = ordinary_residual;
      information.maximum_field_change = maximum_change;
      information.effective_move_limit = scale * source.settings.move_limit;
      information.step_scale = scale;
      information.target_volume_fraction = target_volume_fraction;
      information.linearized_volume_fraction = source_volume + linear_change;
      information.actual_volume_fraction = measured;
      information.qp_stationarity_residual = proposal.diagnostics.qp_stationarity_residual;
      information.volume_multiplier = proposal.diagnostics.volume_multiplier;
      information.backtracking_steps = backtrack;
      candidate.incoming_update = information;
      next = std::move(candidate);
      factors.swap(requested);
      diagnostics = information;
      return true;
    }
    scale *= 0.5;
  }
  PRINT_ERROR("Level-set backtracking could not produce a volume-feasible or volume-reducing step.\n");
  return false;
}

bool CoreTopologyLevelSetUpdate::build_update(const TopologyLevelSetDesign& source, const TopologyLevelSetEvaluation& evaluation, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, double target_volume_fraction, TopologyLevelSetDesign& next, std::map<int, double>& factors, TopologyLevelSetUpdateDiagnostics& diagnostics)
{
  if (&next == &source || &factors == &source.element_solid_fractions || &factors == &source.node_values ||
      &factors == &next.node_values || &factors == &next.element_solid_fractions ||
      &factors == &element_volumes || &factors == &source.volume.density_sensitivities || &factors == &next.volume.density_sensitivities ||
      &factors == &evaluation.compliance_derivatives || &factors == &evaluation.volume_derivatives || &factors == &evaluation.volume_fraction_derivatives ||
      &diagnostics == &source.incoming_update || &diagnostics == &next.incoming_update)
  {
    PRINT_ERROR("Level-set update outputs must not alias source records.\n");
    return false;
  }
  next = {};
  factors.clear();
  diagnostics = {};
  TopologyLevelSetProposal proposal;
  if (!build_proposal(source, evaluation, preserved, element_volumes, target_volume_fraction, proposal))
  {
    return false;
  }
  return accept_proposal(source, proposal, preserved, element_volumes, next, factors, diagnostics);
}
