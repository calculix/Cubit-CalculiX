#ifndef CORETOPOLOGYSTRUCTS_HPP
#define CORETOPOLOGYSTRUCTS_HPP

#include "CoreTopologyLevelSetStructs.hpp"

#include <array>
#include <map>
#include <string>
#include <vector>
#include <set>

/** @brief Configuration method identifiers; public execution currently supports Density only. */
enum class TopologyMethod
{
  Unspecified, ///< Omitted option sentinel; not a runnable method.
  Density, ///< Implemented density optimization.
  LevelSet ///< Internal controller implementation is staged; public execution remains gated.
};

/** @brief Configuration objective identifiers; current density runs support Compliance and Displacement. */
enum class TopologyObjective
{
  Unspecified, ///< Omitted option sentinel; not a runnable objective.
  Compliance, ///< Minimize twice the physical internal strain energy under the supported assumptions.
  Mass, ///< Reserved objective choice; not implemented by the current controller.
  Displacement ///< Minimize the configured maximum absolute component or displacement magnitude.
};

/** @brief Return the objective keyword, or nullptr for an unspecified/unknown value; this is not a run-support check. */
inline const char *topology_objective_name(TopologyObjective objective)
{
  switch (objective)
  {
    case TopologyObjective::Compliance:
    {
      return "compliance";
    }
    case TopologyObjective::Mass:
    {
      return "mass";
    }
    case TopologyObjective::Displacement:
    {
      return "displacement";
    }
    default:
    {
      return nullptr;
    }
  }
}

/** @brief Stored case state; IterationLimit is successful completion without a convergence certificate. */
enum class TopologyRunState
{
  NotRun, ///< No run has been started for the current reset run data.
  Running, ///< An optimization run is in progress; not proof that a solver process is active at this instant.
  Converged, ///< All implemented convergence checks passed; not proof of a global optimum.
  IterationLimit, ///< The configured evaluated-design limit was reached.
  Failed, ///< Preparation, solving, evaluation or another run stage failed.
  Killed ///< The controller reached its requested killed terminal state.
};

/** @brief A transient snapshot combining persisted case state and an optional controller context. */
struct TopologyRunInfo
{
  /// Stored case state name; available historical records do not imply an active run.
  std::string state;
  /// One-based controller iteration ID, or -1 when no controller context supplies it.
  int iteration_id = -1;
  /// Controller job ID, or -1 when unavailable; it can identify the active physical or adjoint phase.
  int job_id = -1;
  /// Whether a present controller context has not reached its Finished phase.
  bool active = false;
  /// Whether termination has been requested; not confirmation that a process has exited.
  bool stop_requested = false;
  /// Controller-wide blocking-wait flag, supplied when this case has a context.
  bool waiting = false;
};

/** @brief Stored objective and sensitivity data for one iteration; availability flags govern diagnostics. */
struct TopologyIterationEvaluation
{
  /// Method captured at evaluation; historical records do not follow case edits.
  TopologyMethod method = TopologyMethod::Unspecified;
  TopologyLevelSetEvaluation level_set_evaluation;
  TopologyLevelSetUpdateDiagnostics level_set_proposal;
  bool level_set_change_available = false;
  double maximum_level_set_change = 0.0;
  /// Objective evaluated by this record; independent from a later case-setting edit.
  TopologyObjective objective = TopologyObjective::Unspecified;
  /// Evaluated objective: force times length for compliance, model length for displacement.
  double value = 0.0;
  /// Whether the monitored displacement fields below contain a measured result.
  bool displacement_available = false;
  /// Maximum absolute component or norm in model lengths; meaningful when displacement_available.
  double displacement_value = 0.0;
  /// Global controlling node ID; meaningful when displacement_available.
  int displacement_node = -1;
  /// Monitored nodeset ID captured with the displacement measurement.
  int displacement_nodeset = 0;
  /// Measured selection: 0=magnitude, 1=X, 2=Y, 3=Z.
  int displacement_component = 0;
  /// Whether the density/objective changes and converged flag have been evaluated.
  bool convergence_checked = false;
  /// Maximum absolute density change from the compared design; valid when convergence_checked.
  double maximum_density_change = 0.0;
  /// Absolute objective change divided by the larger objective, or zero for two zero objectives.
  double relative_objective_change = 0.0;
  /// Whether all implemented convergence tests passed; meaningful when convergence_checked.
  bool converged = false;

  /// Whether an ordinary compliance OC proposal residual has been calculated.
  bool oc_proposal_residual_available = false;
  /// Maximum absolute OC proposal density change using the configured maximum move; valid when available.
  double oc_proposal_residual = 0.0;

  /// Whether an ordinary displacement projected proposal residual has been calculated.
  bool projected_proposal_residual_available = false;
  /// Maximum absolute projected density change using the configured maximum move; valid when available.
  double projected_proposal_residual = 0.0;

  /// Global element ID -> independent applied-factor sensitivity; preserved entries are zero.
  std::map<int, double> stiffness_sensitivities;

  /// Global element ID -> unfiltered density sensitivity; grouped solves use a SIMP surrogate. Empty without a density design.
  std::map<int, double> density_sensitivities;
  /// Global element ID -> filtered density sensitivity; preserved entries are zero. Not generally an exact objective derivative.
  std::map<int, double> filtered_density_sensitivities;
  /// Resolved filter radius in model lengths; meaningful after successful filtering.
  double sensitivity_filter_radius = 0.0;
};

/** @brief Cubit-volume diagnostics; numeric fields and derivatives are meaningful only when available. */
struct TopologyVolumeEvaluation
{

  /// Whether volume diagnostics were successfully collected and evaluated.
  bool available = false;
  /// Sum of participating Cubit element volumes, in model length cubed.
  double total_volume = 0.0;
  /// Sum of density times Cubit volume, in model length cubed; not a mass.
  double solid_volume = 0.0;
  /// Sum of preserved-element volumes, in model length cubed.
  double preserved_volume = 0.0;
  /// solid_volume / total_volume; a dimensionless design fraction.
  double volume_fraction = 0.0;
  /// Preserved volume / total volume; does not include free-element density floors.
  double minimum_volume_fraction = 0.0;

  /// Density: preserved fraction plus the free density floor. Level set: conservative fully-pinned-element volume fraction.
  double minimum_design_volume_fraction = 0.0;

  /// Global element ID -> volume-fraction derivative V_e / total_volume for free elements, zero for preserved elements.
  std::map<int, double> density_sensitivities;
};

/** @brief Stored densities and interpolation settings for one iteration; incoming update diagnostics are optional. */
struct TopologyDensityDesign
{

  /// SIMP exponent captured when this design was constructed.
  double penalization = 0.0;
  /// Requested void stiffness ratio captured with the design.
  double void_ratio = 0.0;
  /// Lower density bound for free elements, captured with the design.
  double minimum_density = 0.0;

  /// Whether incoming update diagnostics exist; false for the initial solid design.
  bool density_update_available = false;
  /// Move selected at the source iteration to generate this design; meaningful when density_update_available.
  double effective_move_limit = 0.0;
  /// Whether the incoming update has a valid density-change correlation.
  bool update_correlation_available = false;
  /// Weighted density-change correlation in [-1,1]; meaningful when update_correlation_available.
  double update_correlation = 0.0;

  /// Global volume-element ID -> dimensionless design solid fraction; preserved values are one.
  std::map<int, double> element_densities;
  /// Separately evaluated Cubit-volume diagnostics; inspect volume.available before use.
  TopologyVolumeEvaluation volume;
};

/** @brief Stored level-set design and captured interpolation settings. */
struct TopologyLevelSetDesign
{
  int iteration_id = -1;
  int source_iteration = -1;
  /// Void stiffness ratio captured with this level-set design.
  double void_ratio = 0.0;
  /// cbrt(mean participating Cubit element volume), in model lengths.
  double characteristic_length = 0.0;
  /// Actual domain-node anchor of the periodic spherical seed lattice.
  std::array<double, 3> seed_origin{};
  /// Whether one free coefficient was placed at zero to avoid nodal saturation.
  bool transition_node_inserted = false;
  /// Geometry found a nonzero fraction derivative for at least one free node.
  bool free_interface_available = false;
  /// Initial physical compliance scale, fixed by the first outgoing update; zero until then.
  double objective_scale = 0.0;
  /// Diagnostics of the source iteration that produced this design, not its own residual.
  TopologyLevelSetUpdateDiagnostics incoming_update;
  /// Union of expanded-connectivity nodes of preserved elements; coefficients stay +1.
  std::set<int> fixed_nodes;
  /// Captured settings; zero initialization is not a valid default configuration.
  TopologyLevelSetSettings settings{};
  /// Global Cubit mesh node ID -> dimensionless coefficient, positive for solid.
  std::map<int, double> node_values;
  /// Global element ID -> regularized solid fraction in [0,1].
  std::map<int, double> element_solid_fractions;
  TopologyVolumeEvaluation volume;
};

/** @brief Transient preservation-management query row; not a historical element-flag snapshot. */
struct TopologyPreservedSetInfo
{
  /// Preservation kind: 1=block, 2=nodeset, 3=sideset; not a Cubit element-type ID.
  int set_type = 0;
  /// Positive set ID scoped by set_type.
  int set_id = 0;

  /// Whether the current model supplies this set as an automatic preservation source.
  bool automatic = false;
  /// Whether an explicit inclusion rule exists for this set.
  bool user_added = false;
  /// Whether an explicit exclusion rule exists for this set.
  bool excluded = false;
  /// Whether (automatic or user_added) and not excluded holds for this set rule.
  bool effective = false;
};

/** @brief Core-owned case settings and run records; creation/loading supplies validated settings, not this aggregate alone. */
struct TopologyCase
{
  /// Case name used for display and a sanitized iteration-job prefix.
  std::string name;
  /// Configured method; enum membership alone does not establish run support.
  TopologyMethod method;
  /// Configured objective; previous evaluations retain their own objective tag.
  TopologyObjective objective;

  /// Explicit inclusion rules: preservation kind -> set IDs; kinds are 1=block, 2=nodeset, 3=sideset.
  std::map<int, std::set<int>> added_preserved_sets;
  /// Explicit exclusion rules by preservation kind, applied before element union.
  std::map<int, std::set<int>> removed_preserved_sets;

  /// Resolved run-domain global volume-element ID -> preserved flag, including false entries.
  std::map<int, bool> element_preserved;

  /// Configured upper bound on density-weighted volume divided by participating volume.
  double target_volume_fraction;
  /// Configured SIMP void ratio; stored designs capture their own interpolation settings.
  double void_stiffness_ratio;
  /// Maximum evaluated designs, including the initial solid design as iteration 1.
  int maximum_iterations;
  /// Total generated topology material-group budget across all participating base materials.
  int material_cap = 1024;
  /// Configured SIMP exponent for newly constructed designs.
  double density_penalization;
  /// Configured free-element density floor for newly constructed designs.
  double minimum_density;
  /// Configured radius in model lengths; zero selects the automatic radius.
  double filter_radius;
  /// Configured maximum per-element absolute density change for an update.
  double move_limit;
  /// Whether the density-history correlation selects a smaller effective move.
  bool adaptive_move_limits;
  /// Adaptive lower move bound; zero selects ten percent of move_limit.
  double minimum_move_limit;
  /// Tolerance for maximum density change and the objective-specific ordinary proposal residual.
  double density_tolerance;
  /// Relative objective-change tolerance for both compliance and displacement; historical field name.
  double compliance_tolerance;
  /// Monitored nodeset ID; a positive value also enables optional displacement monitoring.
  int displacement_nodeset;
  /// Monitored selection: 0=magnitude, 1=X, 2=Y, 3=Z.
  int displacement_component;

  /// Core initializes these settings; optional command edits affect future designs. Public runs remain gated.
  TopologyLevelSetSettings level_set_settings{};

  /// One-based iteration ID -> physical job ID; job IDs are a separate identifier domain.
  std::map<int, int> iteration_jobs;
  /// One-based iteration ID -> adjoint job ID; absent when no adjoint is required.
  std::map<int, int> iteration_adjoint_jobs;
  /// Stored core iteration selection; -1 means none. GUI browsing can display a different iteration.
  int selected_iteration = -1;
  /// Stored run state; loading a saved active run does not recreate its controller context.
  TopologyRunState run_state = TopologyRunState::NotRun;
  /// Iteration ID -> global element ID -> physical whole-element internal strain energy.
  std::map<int, std::map<int, double>> iteration_element_energies;
  /// Iteration ID -> sum of stored physical element internal energies.
  std::map<int, double> iteration_total_energies;
  /// Iteration ID -> objective/sensitivity record; presence does not make every optional diagnostic available.
  std::map<int, TopologyIterationEvaluation> iteration_evaluations;
  /// Iteration ID -> density design; applied stiffness factors belong to the material core.
  std::map<int, TopologyDensityDesign> iteration_density_designs;
  /// Separate in-memory records; no density/SIMP interpretation is implied.
  std::map<int, TopologyLevelSetDesign> iteration_level_set_designs;
};

/**
 * @brief Presence-marked case edits; payload initializers are not core configuration defaults.
 * @note Only has_* entries participate in create/modify. Creation applies them to
 * CoreTopology defaults; modification applies them to the stored case. An explicit
 * zero and an omitted option are different requests where zero is supported.
 */
struct TopologyOptions
{
  /// Whether name was explicitly supplied; false leaves the case/default unchanged.
  bool has_name = false;
  /// Optional name payload; applied only when has_name is true.
  std::string name;

  /// Whether method was explicitly supplied; false leaves the case/default unchanged.
  bool has_method = false;
  /// Optional method payload; applied only when has_method is true.
  TopologyMethod method = TopologyMethod::Unspecified;

  /// Whether objective was explicitly supplied; false leaves the case/default unchanged.
  bool has_objective = false;
  /// Optional objective payload; applied only when has_objective is true.
  TopologyObjective objective = TopologyObjective::Unspecified;

  /// Whether volume_fraction was explicitly supplied; false leaves the case/default unchanged.
  bool has_volume_fraction = false;
  /// Optional volume_fraction payload; applied only when has_volume_fraction is true.
  double volume_fraction = 0.0;

  /// Whether void_ratio was explicitly supplied; false leaves the case/default unchanged.
  bool has_void_ratio = false;
  /// Optional void_ratio payload; applied only when has_void_ratio is true.
  double void_ratio = 0.0;

  /// Whether material_cap was explicitly supplied; false leaves the case/default unchanged.
  bool has_material_cap = false;
  /// Optional material_cap payload; applied only when has_material_cap is true.
  int material_cap = 1024;

  /// Whether maximum_iterations was explicitly supplied; false leaves the case/default unchanged.
  bool has_maximum_iterations = false;
  /// Optional maximum_iterations payload; applied only when has_maximum_iterations is true.
  int maximum_iterations = 0;

  /// Whether filter_radius was explicitly supplied; false leaves the case/default unchanged.
  bool has_filter_radius = false;
  /// Optional filter_radius payload; applied only when has_filter_radius is true.
  double filter_radius = 0.0;

  /// Whether move_limit was explicitly supplied; false leaves the case/default unchanged.
  bool has_move_limit = false;
  /// Optional move_limit payload; applied only when has_move_limit is true.
  double move_limit = 0.0;

  /// Whether adaptive_move_limits was explicitly supplied; false leaves the case/default unchanged.
  bool has_adaptive_move_limits = false;
  /// Optional adaptive_move_limits payload; applied only when has_adaptive_move_limits is true.
  bool adaptive_move_limits = false;

  /// Whether minimum_move_limit was explicitly supplied; false leaves the case/default unchanged.
  bool has_minimum_move_limit = false;
  /// Optional minimum_move_limit payload; applied only when has_minimum_move_limit is true.
  double minimum_move_limit = 0.0;

  /// Whether density_tolerance was explicitly supplied; false leaves the case/default unchanged.
  bool has_density_tolerance = false;
  /// Optional density_tolerance payload; applied only when has_density_tolerance is true.
  double density_tolerance = 0.0;

  /// Whether compliance_tolerance was explicitly supplied; false leaves the case/default unchanged.
  bool has_compliance_tolerance = false;
  /// Optional compliance_tolerance payload; applied only when has_compliance_tolerance is true.
  double compliance_tolerance = 0.0;

  /// Whether displacement_nodeset was explicitly supplied; false leaves the case/default unchanged.
  bool has_displacement_nodeset = false;
  /// Optional displacement_nodeset payload; applied only when has_displacement_nodeset is true.
  int displacement_nodeset = 0;

  /// Whether displacement_component was explicitly supplied; false leaves the case/default unchanged.
  bool has_displacement_component = false;
  /// Optional displacement_component payload; applied only when has_displacement_component is true.
  int displacement_component = 0;

  /// Presence flags apply individual edits; payload initialization is not a Core default.
  bool has_level_set_interface_width = false;
  bool has_level_set_regularization_length = false;
  bool has_level_set_move_limit = false;
  bool has_level_set_field_tolerance = false;
  bool has_level_set_seed_spacing = false;
  bool has_level_set_seed_radius = false;
  bool has_level_set_integration_tolerance = false;
  bool has_level_set_integration_max_refinements = false;
  TopologyLevelSetSettings level_set_settings{};

};

#endif
