#ifndef CORETOPOLOGYSTRUCTS_HPP
#define CORETOPOLOGYSTRUCTS_HPP

#include <map>
#include <string>
#include <vector>
#include <set>

enum class TopologyMethod
{
  Unspecified,
  Density,
  LevelSet
};

enum class TopologyObjective
{
  Unspecified,
  Compliance,
  Mass,
  Displacement
};

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

enum class TopologyRunState
{
  NotRun,
  Running,
  Converged,
  IterationLimit,
  Failed,
  Killed
};

struct TopologyRunInfo
{
  std::string state;
  int iteration_id = -1;
  int job_id = -1;
  bool active = false;
  bool stop_requested = false;
  bool waiting = false;
};

struct TopologyIterationEvaluation
{
  TopologyObjective objective = TopologyObjective::Unspecified;
  double value = 0.0;
  bool displacement_available = false;
  double displacement_value = 0.0;
  int displacement_node = -1;
  int displacement_nodeset = 0;
  int displacement_component = 0;
  bool convergence_checked = false;
  double maximum_density_change = 0.0;
  double relative_objective_change = 0.0;
  bool converged = false;

  // Ordinary compliance OC proposal, calculated before adaptive restriction.
  bool oc_proposal_residual_available = false;
  double oc_proposal_residual = 0.0;

  // Ordinary displacement projection using the configured maximum move.
  bool projected_proposal_residual_available = false;
  double projected_proposal_residual = 0.0;

  // Global element ID -> derivative with respect to stiffness factor.
  // Preserved elements receive zero because they are fixed.
  std::map<int, double> stiffness_sensitivities;
  // Global element ID -> derivative with respect to solid fraction.
  // Empty when no density design belongs to the iteration.
  std::map<int, double> density_sensitivities;
  std::map<int, double> filtered_density_sensitivities;
  double sensitivity_filter_radius = 0.0;
};

struct TopologyVolumeEvaluation
{
  /*
  Total volume:       Σ V_e
  Solid volume:       Σ ρ_e V_e
  Preserved volume:   Σ V_e for preserved elements
  Volume fraction:    solid volume / total volume
  Minimum fraction:   preserved volume / total volume
  */
  bool available = false;
  double total_volume = 0.0;
  double solid_volume = 0.0;
  double preserved_volume = 0.0;
  double volume_fraction = 0.0;
  double minimum_volume_fraction = 0.0;
  // Includes preserved elements and the density floor of free elements.
  double minimum_design_volume_fraction = 0.0;

  // Derivative of volume fraction with respect to element density.
  // Preserved elements receive zero because they are fixed.
  std::map<int, double> density_sensitivities;
};

struct TopologyDensityDesign
{
  // Store interpolation settings with the iteration.
  double penalization = 0.0;
  double void_ratio = 0.0;
  double minimum_density = 0.0;

  // Describes the update that generated this design; absent for iteration 1.
  bool density_update_available = false;
  double effective_move_limit = 0.0;
  bool update_correlation_available = false;
  double update_correlation = 0.0;

  // Global element ID -> solid fraction.
  std::map<int, double> element_densities;
  TopologyVolumeEvaluation volume;
};

// Transient query data for preservation management in the GUI.
struct TopologyPreservedSetInfo
{
  int set_type = 0; // 1 = block, 2 = nodeset, 3 = sideset.
  int set_id = 0;

  bool automatic = false;
  bool user_added = false;
  bool excluded = false;
  bool effective = false;
};

struct TopologyCase
{
  std::string name;
  TopologyMethod method;
  TopologyObjective objective;

  // Set types follow the step add/remove approach:
  // 1 = block, 2 = nodeset, 3 = sideset.

  std::map<int, std::set<int>> added_preserved_sets;
  std::map<int, std::set<int>> removed_preserved_sets;

  // Global volume element ID -> preserved.
  std::map<int, bool> element_preserved;

  double target_volume_fraction;
  double void_stiffness_ratio;
  int maximum_iterations;
  int material_cap = 1024; // Total generated topology materials, across base materials.
  double density_penalization;
  double minimum_density;
  double filter_radius; // 0 = automatic; positive = model length units.
  double move_limit;
  bool adaptive_move_limits;
  double minimum_move_limit; // 0 = automatic: 10% of move_limit.
  double density_tolerance;
  double compliance_tolerance;
  int displacement_nodeset;
  int displacement_component; // 0 = magnitude, 1 = X, 2 = Y, 3 = Z.

  std::map<int, int> iteration_jobs; // Iteration ID -> physical job ID.
  std::map<int, int> iteration_adjoint_jobs; // Iteration ID -> adjoint job ID.
  int selected_iteration = -1;
  TopologyRunState run_state = TopologyRunState::NotRun;
  std::map<int, std::map<int, double>> iteration_element_energies; // Iteration ID -> global element ID -> internal energy.
  std::map<int, double> iteration_total_energies;
  std::map<int, TopologyIterationEvaluation> iteration_evaluations;
  std::map<int, TopologyDensityDesign> iteration_density_designs;
};

struct TopologyOptions
{
  bool has_name = false;
  std::string name;

  bool has_method = false;
  TopologyMethod method = TopologyMethod::Unspecified;

  bool has_objective = false;
  TopologyObjective objective = TopologyObjective::Unspecified;

  bool has_volume_fraction = false;
  double volume_fraction = 0.0;

  bool has_void_ratio = false;
  double void_ratio = 0.0;

  bool has_material_cap = false;
  int material_cap = 1024;

  bool has_maximum_iterations = false;
  int maximum_iterations = 0;

  bool has_filter_radius = false;
  double filter_radius = 0.0;

  bool has_move_limit = false;
  double move_limit = 0.0;

  bool has_adaptive_move_limits = false;
  bool adaptive_move_limits = false;

  bool has_minimum_move_limit = false;
  double minimum_move_limit = 0.0;

  bool has_density_tolerance = false;
  double density_tolerance = 0.0;

  bool has_compliance_tolerance = false;
  double compliance_tolerance = 0.0;

  bool has_displacement_nodeset = false;
  int displacement_nodeset = 0;

  bool has_displacement_component = false;
  int displacement_component = 0;
};

#endif
