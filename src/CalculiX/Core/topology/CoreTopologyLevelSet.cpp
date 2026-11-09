#include "CoreTopologyLevelSet.hpp"
#include "CoreTopologyTrace.hpp"
#include "CubitMessage.hpp"
#include "CoreTopologyLevelSetInitialization.hpp"
#include "CoreTopologyLevelSetEvaluation.hpp"
#include "CoreTopologyStructs.hpp"
#include <utility>
#include <cmath>

TopologyLevelSetSettings CoreTopologyLevelSet::default_settings()
{
  TopologyLevelSetSettings settings{};
  settings.interface_width = 0.1;
  settings.regularization_length = 0.0;
  settings.move_limit = 0.1;
  settings.field_tolerance = 0.01;
  settings.seed_spacing = 0.0;
  settings.seed_radius = 0.0;
  settings.integration_tolerance = 1.0e-4;
  settings.integration_max_refinements = 4;
  return settings;
}

bool CoreTopologyLevelSet::validate_settings(const TopologyLevelSetSettings& settings)
{
  if (!std::isfinite(settings.interface_width) || settings.interface_width <= 0.0 || settings.interface_width > 1.0 ||
      !std::isfinite(settings.regularization_length) || settings.regularization_length < 0.0 ||
      !std::isfinite(settings.move_limit) || settings.move_limit <= 0.0 || settings.move_limit > 2.0 ||
      !std::isfinite(settings.field_tolerance) || settings.field_tolerance <= 0.0 || settings.field_tolerance > 2.0 ||
      !std::isfinite(settings.seed_spacing) || settings.seed_spacing < 0.0 ||
      !std::isfinite(settings.seed_radius) || settings.seed_radius < 0.0 ||
      !std::isfinite(settings.integration_tolerance) || settings.integration_tolerance <= 0.0 || settings.integration_tolerance >= 1.0 ||
      settings.integration_max_refinements < 1 || settings.integration_max_refinements > 5)
  {
    PRINT_ERROR("Invalid level-set settings.\n");
    return false;
  }
  if (settings.seed_spacing > 0.0 && settings.seed_radius > 0.0 && settings.seed_radius >= 0.5 * settings.seed_spacing)
  {
    PRINT_ERROR("Level-set seed radius must be smaller than half the seed spacing.\n");
    return false;
  }
  return true;
}

bool CoreTopologyLevelSet::build_initial_design(const TopologyLevelSetSettings& settings, double void_ratio, const std::map<int, bool>& preserved, const std::map<int, double>& element_volumes, TopologyLevelSetDesign& design, std::map<int, double>& factors)
{
  design = {};
  factors.clear();
  TopologyLevelSetDesign candidate;
  std::map<int, double> requested;
  CoreTopologyTrace::write("initial_design.initialize.enter");
  if (!CoreTopologyLevelSetInitialization::initialize(settings, void_ratio, preserved, element_volumes, candidate))
  {
    CoreTopologyTrace::write("initial_design.initialize.failed");
    return false;
  }
  CoreTopologyTrace::write("initial_design.initialize.leave");
  CoreTopologyTrace::write("initial_design.geometry.enter");
  if (!CoreTopologyLevelSetEvaluation::build_geometry(candidate, preserved, element_volumes))
  {
    CoreTopologyTrace::write("initial_design.geometry.failed");
    return false;
  }
  CoreTopologyTrace::write("initial_design.geometry.leave");
  CoreTopologyTrace::write("initial_design.factors.enter");
  if (!CoreTopologyLevelSetEvaluation::build_factors(candidate, preserved, requested))
  {
    CoreTopologyTrace::write("initial_design.factors.failed");
    return false;
  }
  CoreTopologyTrace::write("initial_design.factors.leave");
  design = std::move(candidate);
  factors.swap(requested);
  return true;
}

TopologyLevelSetUpdateControls CoreTopologyLevelSet::default_update_controls()
{
  TopologyLevelSetUpdateControls controls{};
  controls.maximum_coordinate_sweeps = 2000;
  controls.maximum_multiplier_iterations = 80;
  controls.maximum_bracket_iterations = 60;
  controls.maximum_backtracking_steps = 16;
  controls.qp_tolerance = 1.0e-8;
  controls.linear_volume_tolerance = 1.0e-8;
  controls.volume_tolerance = 1.0e-6;
  controls.restoration_fraction = 0.1;
  return controls;
}
