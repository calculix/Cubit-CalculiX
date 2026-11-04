#include "CoreTopologySaveValidation.hpp"
#include "CoreTopologyLevelSet.hpp"
#include "TopologyIterationIds.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <set>

namespace
{
bool bounded(double value, double lower, double upper)
{
  return std::isfinite(value) && value >= lower && value <= upper;
}

template<class T, class U>
bool same_keys(const std::map<int, T>& values, const std::map<int, U>& reference)
{
  if (values.size() != reference.size())
  {
    return false;
  }
  for (const auto& value : values)
  {
    if (value.first <= 0 || reference.count(value.first) == 0)
    {
      return false;
    }
  }
  return true;
}

bool finite_map(const std::map<int, double>& values)
{
  for (const auto& value : values)
  {
    if (value.first <= 0 || !std::isfinite(value.second))
    {
      return false;
    }
  }
  return true;
}

bool valid_volume(const TopologyVolumeEvaluation& volume)
{
  if (!volume.available)
  {
    return true;
  }
  return std::isfinite(volume.total_volume) && volume.total_volume > 0.0 &&
      bounded(volume.solid_volume, 0.0, volume.total_volume * (1.0 + 1.0e-10)) &&
      bounded(volume.preserved_volume, 0.0, volume.total_volume * (1.0 + 1.0e-10)) &&
      bounded(volume.volume_fraction, 0.0, 1.0 + 1.0e-10) &&
      bounded(volume.minimum_volume_fraction, 0.0, 1.0 + 1.0e-10) &&
      bounded(volume.minimum_design_volume_fraction, 0.0, 1.0 + 1.0e-10) &&
      std::abs(volume.volume_fraction - volume.solid_volume / volume.total_volume) <= 1.0e-10 &&
      std::abs(volume.minimum_volume_fraction - volume.preserved_volume / volume.total_volume) <= 1.0e-10 &&
      volume.minimum_design_volume_fraction + 1.0e-10 >= volume.minimum_volume_fraction && finite_map(volume.density_sensitivities);
}

bool valid_diagnostics(const TopologyLevelSetUpdateDiagnostics& value, int source)
{
  if (!value.available)
  {
    return true;
  }
  return source > 0 && value.source_iteration == source &&
      bounded(value.ordinary_proposal_residual, 0.0, 2.0) && bounded(value.maximum_field_change, 0.0, 2.0) &&
      bounded(value.effective_move_limit, 0.0, 2.0) && bounded(value.step_scale, 0.0, 1.0) &&
      bounded(value.target_volume_fraction, 0.0, 1.0) && std::isfinite(value.linearized_volume_fraction) &&
      bounded(value.actual_volume_fraction, 0.0, 1.0 + 1.0e-10) &&
      std::isfinite(value.qp_stationarity_residual) && value.qp_stationarity_residual >= 0.0 &&
      std::isfinite(value.volume_multiplier) && value.volume_multiplier >= 0.0 && value.backtracking_steps >= 0;
}

const std::map<int, double>* fractions(const TopologyCase& configuration, int iteration)
{
  const auto density = configuration.iteration_density_designs.find(iteration);
  const auto level_set = configuration.iteration_level_set_designs.find(iteration);
  if ((density != configuration.iteration_density_designs.end()) == (level_set != configuration.iteration_level_set_designs.end()))
  {
    return nullptr;
  }
  return density != configuration.iteration_density_designs.end() ? &density->second.element_densities : &level_set->second.element_solid_fractions;
}
}

bool CoreTopologySaveValidation::validate_case(const TopologyCase& configuration)
{
  if (!CoreTopologyLevelSet::validate_settings(configuration.level_set_settings) ||
      !valid_topology_iteration_keys(configuration.iteration_jobs) || !valid_topology_iteration_keys(configuration.iteration_adjoint_jobs) ||
      !valid_topology_iteration_keys(configuration.iteration_density_designs) || !valid_topology_iteration_keys(configuration.iteration_level_set_designs) ||
      !valid_topology_iteration_keys(configuration.iteration_evaluations) || !valid_topology_iteration_keys(configuration.iteration_element_energies) ||
      !valid_topology_iteration_keys(configuration.iteration_total_energies) ||
      configuration.selected_iteration < -1 || configuration.selected_iteration == 0 ||
      static_cast<int>(configuration.run_state) < 0 || configuration.run_state > TopologyRunState::Killed)
  {
    return false;
  }
  for (const auto& element : configuration.element_preserved)
  {
    if (element.first <= 0)
    {
      return false;
    }
  }
  std::set<int> jobs;
  for (const auto* associations : {&configuration.iteration_jobs, &configuration.iteration_adjoint_jobs})
  {
    for (const auto& job : *associations)
    {
      if (job.second <= 0 || !jobs.insert(job.second).second || fractions(configuration, job.first) == nullptr ||
          configuration.iteration_jobs.count(job.first) == 0)
      {
        return false;
      }
    }
  }
  for (const auto& entry : configuration.iteration_density_designs)
  {
    const auto& design = entry.second;
    if (configuration.iteration_level_set_designs.count(entry.first) != 0 ||
        !same_keys(design.element_densities, configuration.element_preserved) || design.element_densities.empty() ||
        !std::isfinite(design.penalization) || design.penalization < 1.0 ||
        !bounded(design.void_ratio, 0.0, 1.0) || design.void_ratio <= 0.0 ||
        !bounded(design.minimum_density, 0.0, 1.0) || design.minimum_density <= 0.0 || !valid_volume(design.volume) ||
        (design.density_update_available && (!bounded(design.effective_move_limit, 0.0, 1.0) || design.effective_move_limit <= 0.0)) ||
        (design.update_correlation_available && !bounded(design.update_correlation, -1.0, 1.0)))
    {
      return false;
    }
    for (const auto& density : design.element_densities)
    {
      if (!bounded(density.second, design.minimum_density, 1.0) || (configuration.element_preserved.at(density.first) && density.second != 1.0))
      {
        return false;
      }
    }
  }
  for (const auto& entry : configuration.iteration_level_set_designs)
  {
    const auto& design = entry.second;
    if (design.iteration_id != entry.first || design.node_values.empty() || design.element_solid_fractions.empty() ||
        !same_keys(design.element_solid_fractions, configuration.element_preserved) ||
        !CoreTopologyLevelSet::validate_settings(design.settings) || !valid_volume(design.volume) ||
        !bounded(design.void_ratio, 0.0, 1.0) || design.void_ratio <= 0.0 ||
        !std::isfinite(design.characteristic_length) || design.characteristic_length <= 0.0 ||
        !std::isfinite(design.objective_scale) || design.objective_scale < 0.0 ||
        !valid_diagnostics(design.incoming_update, design.source_iteration))
    {
      return false;
    }
    if ((entry.first == 1 && (design.source_iteration != -1 || design.incoming_update.available)) ||
        (entry.first > 1 && (design.source_iteration != entry.first - 1 ||
            configuration.iteration_level_set_designs.count(design.source_iteration) == 0 || !design.incoming_update.available)))
    {
      return false;
    }
    for (const double coordinate : design.seed_origin)
    {
      if (!std::isfinite(coordinate))
      {
        return false;
      }
    }
    for (const auto& node : design.node_values)
    {
      if (node.first <= 0 || !bounded(node.second, -1.0, 1.0))
      {
        return false;
      }
    }
    for (const int node : design.fixed_nodes)
    {
      const auto value = design.node_values.find(node);
      if (value == design.node_values.end() || value->second != 1.0)
      {
        return false;
      }
    }
    for (const auto& fraction : design.element_solid_fractions)
    {
      if (!bounded(fraction.second, 0.0, 1.0) || (configuration.element_preserved.at(fraction.first) && fraction.second != 1.0))
      {
        return false;
      }
    }
  }
  for (const auto& entry : configuration.iteration_evaluations)
  {
    const auto& evaluation = entry.second;
    if (configuration.iteration_jobs.count(entry.first) == 0 || !std::isfinite(evaluation.value) ||
        !std::isfinite(evaluation.sensitivity_filter_radius) || evaluation.sensitivity_filter_radius < 0.0 ||
        (evaluation.displacement_available && (!std::isfinite(evaluation.displacement_value) || evaluation.displacement_value < 0.0 ||
            evaluation.displacement_node <= 0 || evaluation.displacement_nodeset <= 0 || evaluation.displacement_component < 0 || evaluation.displacement_component > 3)) ||
        (evaluation.convergence_checked && (!bounded(evaluation.maximum_density_change, 0.0, 1.0) ||
            !std::isfinite(evaluation.relative_objective_change) || evaluation.relative_objective_change < 0.0)) ||
        (evaluation.oc_proposal_residual_available && !bounded(evaluation.oc_proposal_residual, 0.0, 1.0)) ||
        (evaluation.projected_proposal_residual_available && !bounded(evaluation.projected_proposal_residual, 0.0, 1.0)) ||
        !finite_map(evaluation.stiffness_sensitivities) || !finite_map(evaluation.density_sensitivities) || !finite_map(evaluation.filtered_density_sensitivities) ||
        (evaluation.objective != TopologyObjective::Compliance && evaluation.objective != TopologyObjective::Displacement))
    {
      return false;
    }
    if (evaluation.method == TopologyMethod::Density)
    {
      if (configuration.iteration_density_designs.count(entry.first) == 0)
      {
        return false;
      }
    }
    else if (evaluation.method == TopologyMethod::LevelSet)
    {
      const auto design = configuration.iteration_level_set_designs.find(entry.first);
      const auto& nodal = evaluation.level_set_evaluation;
      if (design == configuration.iteration_level_set_designs.end() || evaluation.objective != TopologyObjective::Compliance ||
          !valid_diagnostics(evaluation.level_set_proposal, entry.first) ||
          (evaluation.level_set_change_available && !bounded(evaluation.maximum_level_set_change, 0.0, 2.0)))
      {
        return false;
      }
      // A physical objective may have been stored before nodal evaluation failed.
      if (nodal.available && (nodal.iteration_id != entry.first || !std::isfinite(nodal.compliance) ||
          !std::isfinite(nodal.total_volume) || nodal.total_volume <= 0.0 ||
          !bounded(nodal.solid_volume, 0.0, nodal.total_volume * (1.0 + 1.0e-10)) || !bounded(nodal.volume_fraction, 0.0, 1.0 + 1.0e-10) ||
          !same_keys(nodal.compliance_derivatives, design->second.node_values) || !finite_map(nodal.compliance_derivatives) ||
          !same_keys(nodal.volume_derivatives, design->second.node_values) || !finite_map(nodal.volume_derivatives) ||
          !same_keys(nodal.volume_fraction_derivatives, design->second.node_values) || !finite_map(nodal.volume_fraction_derivatives)))
      {
        return false;
      }
      if (nodal.available)
      {
        for (const int node : design->second.fixed_nodes)
        {
          if (nodal.compliance_derivatives.at(node) != 0.0 || nodal.volume_derivatives.at(node) != 0.0 || nodal.volume_fraction_derivatives.at(node) != 0.0)
          {
            return false;
          }
        }
      }
    }
    else
    {
      return false;
    }
  }
  for (const auto& entry : configuration.iteration_element_energies)
  {
    if (configuration.iteration_jobs.count(entry.first) == 0 || !same_keys(entry.second, configuration.element_preserved) || !finite_map(entry.second))
    {
      return false;
    }
  }
  for (const auto& entry : configuration.iteration_total_energies)
  {
    if (configuration.iteration_jobs.count(entry.first) == 0 || !std::isfinite(entry.second))
    {
      return false;
    }
  }
  const int selected = configuration.selected_iteration;
  return selected == -1 || (configuration.iteration_jobs.count(selected) != 0 &&
      configuration.iteration_evaluations.count(selected) != 0 && fractions(configuration, selected) != nullptr);
}

bool CoreTopologySaveValidation::validate_factors(const TopologyCase& configuration, const std::map<int, int>& assignments, const std::map<int, std::map<int, double>>& factors)
{
  if (!same_keys(assignments, configuration.element_preserved))
  {
    return false;
  }
  for (const auto& assignment : assignments)
  {
    if (assignment.second <= 0)
    {
      return false;
    }
  }
  for (const auto& iteration : factors)
  {
    const auto* values = fractions(configuration, iteration.first);
    if (iteration.first < 1 || values == nullptr || !same_keys(*values, configuration.element_preserved) || !same_keys(iteration.second, *values))
    {
      return false;
    }
    for (const auto& factor : iteration.second)
    {
      if (!bounded(factor.second, 0.0, 1.0) || factor.second <= 0.0 ||
          (configuration.element_preserved.at(factor.first) && factor.second != 1.0))
      {
        return false;
      }
    }
  }
  for (const auto& design : configuration.iteration_density_designs)
  {
    if (factors.count(design.first) == 0)
    {
      return false;
    }
  }
  for (const auto& design : configuration.iteration_level_set_designs)
  {
    if (factors.count(design.first) == 0)
    {
      return false;
    }
  }
  return true;
}
