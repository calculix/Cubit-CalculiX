#ifndef CORETOPOLOGYLEVELSETSTRUCTS_HPP
#define CORETOPOLOGYLEVELSETSTRUCTS_HPP

#include <vector>
#include <map>

/**
 * @brief Parameters for the planned fixed-mesh, nodal level-set method.
 * @note Value initialization supplies empty payloads, not valid configuration
 * defaults. CoreTopologyLevelSet::default_settings() owns the defaults.
 * Presence-marked command options edit case settings. These records do not enable public execution.
 */
struct TopologyLevelSetSettings
{
  double interface_width;
  double regularization_length;
  double move_limit;
  double field_tolerance;
  double seed_spacing;
  double seed_radius;
  double integration_tolerance;
  int integration_max_refinements;
};

/** @brief One element's regularized fraction and discrete nodal derivatives. */
struct TopologyLevelSetElementGeometry
{
  double solid_fraction = 0.0;
  /// Normalizing geometric quadrature volume, not the Cubit constraint volume.
  double integration_volume = 0.0;
  /// Supplied Cubit volume times solid_fraction.
  double solid_volume = 0.0;
  /// Zero-based Cubit expanded-connectivity position -> d(fraction)/d(phi).
  std::vector<double> fraction_derivatives;
  /// Same order, multiplied by the supplied Cubit volume.
  std::vector<double> volume_derivatives;
  int subdivisions = 0;
};

/** @brief Compliance and feasible nodal derivatives; unavailable until complete evaluation. */
struct TopologyLevelSetEvaluation
{
  bool available = false;
  int iteration_id = -1;
  double compliance = 0.0;
  double total_volume = 0.0;
  double solid_volume = 0.0;
  double volume_fraction = 0.0;
  /// True when supplied applied factors differ from the requested continuous factors.
  bool factor_approximation = false;
  /// Global node ID -> derivative; fixed-node entries are explicitly zero.
  std::map<int, double> compliance_derivatives;
  std::map<int, double> volume_derivatives;
  std::map<int, double> volume_fraction_derivatives;
};

/** @brief Internal solver controls; authoritative values come from Core defaults. */
struct TopologyLevelSetUpdateControls
{
  int maximum_coordinate_sweeps;
  int maximum_multiplier_iterations;
  int maximum_bracket_iterations;
  int maximum_backtracking_steps;
  double qp_tolerance;
  double linear_volume_tolerance;
  double volume_tolerance;
  double restoration_fraction;
};

/** @brief A source-iteration proposal and the accepted outgoing step. */
struct TopologyLevelSetUpdateDiagnostics
{
  bool available = false;
  int source_iteration = -1;
  bool volume_restoration = false;
  bool volume_feasible = false;
  /// Ordinary full-move QP proposal; not reduced by nonlinear backtracking.
  double ordinary_proposal_residual = 0.0;
  double maximum_field_change = 0.0;
  double effective_move_limit = 0.0;
  double step_scale = 0.0;
  double target_volume_fraction = 0.0;
  double linearized_volume_fraction = 0.0;
  double actual_volume_fraction = 0.0;
  double qp_stationarity_residual = 0.0;
  double volume_multiplier = 0.0;
  int backtracking_steps = 0;
};

/** @brief Transient QP result; never persisted or used for a different source iteration. */
struct TopologyLevelSetProposal
{
  bool available = false;
  int source_iteration = -1;
  double objective_scale = 0.0;
  double source_volume_fraction = 0.0;
  std::map<int, double> node_values;
  std::map<int, double> volume_fraction_derivatives;
  TopologyLevelSetUpdateDiagnostics diagnostics;
};

#endif
