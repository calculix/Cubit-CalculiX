#include "CoreTopology.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

#include <cmath>
#include <set>
#include <utility>
#include <algorithm>

// HELPERS
namespace
{
void ApplyTopologyOptions(
    TopologyCase& configuration,
    const TopologyOptions& options)
{
  if (options.has_name)
    configuration.name = options.name;

  if (options.has_method)
    configuration.method = options.method;

  if (options.has_objective)
    configuration.objective = options.objective;

  if (options.has_volume_fraction)
    configuration.target_volume_fraction = options.volume_fraction;

  if (options.has_void_ratio)
    configuration.void_stiffness_ratio = options.void_ratio;

  if (options.has_maximum_iterations)
    configuration.maximum_iterations = options.maximum_iterations;
}

bool ValidateTopologyConfiguration(const TopologyCase& configuration)
{
  const bool valid_method =
    configuration.method == TopologyMethod::Density ||
    configuration.method == TopologyMethod::LevelSet;

  const bool valid_objective =
      configuration.objective == TopologyObjective::Compliance ||
      configuration.objective == TopologyObjective::Mass ||
      configuration.objective == TopologyObjective::Displacement;

  if (!valid_method || !valid_objective)
    return false;

  return
      configuration.name.find_first_not_of(" \t\r\n") != std::string::npos &&
      std::isfinite(configuration.target_volume_fraction) &&
      configuration.target_volume_fraction > 0.0 &&
      configuration.target_volume_fraction <= 1.0 &&
      std::isfinite(configuration.void_stiffness_ratio) &&
      configuration.void_stiffness_ratio > 0.0 &&
      configuration.void_stiffness_ratio < 1.0 &&
      configuration.maximum_iterations > 0 &&
      std::isfinite(configuration.minimum_density) &&
      configuration.minimum_density > 0.0 &&
      configuration.minimum_density < 1.0;
}
}
// HELPERS END

CoreTopology::CoreTopology()
{
  ccx_iface = new CalculiXCoreInterface();
}

CoreTopology::~CoreTopology()
{
  delete ccx_iface;
}

bool CoreTopology::init()
{
  if (is_initialized)
    return false;

  is_initialized = true;
  return true;
}

bool CoreTopology::update()
{
  // Case configuration currently changes only through explicit methods.
  return is_initialized;
}

bool CoreTopology::reset()
{
  topology_cases.clear();
  return true;
}

bool CoreTopology::check_initialized()
{
  return is_initialized;
}

bool CoreTopology::create_topology(int topology_id,const TopologyOptions& options)
{
  if (!is_initialized || topology_id <= 0)
  {
    PRINT_ERROR("Invalid topology initialization or ID.\n");
    return false;
  }

  if (topology_cases.count(topology_id) != 0)
  {
    PRINT_ERROR("Topology case %d already exists.\n", topology_id);
    return false;
  }

  // Defaults belong to CoreTopology.
  TopologyCase configuration;
  configuration.name = "topology_" + std::to_string(topology_id);
  configuration.method = TopologyMethod::Density;
  configuration.objective = TopologyObjective::Compliance;
  configuration.target_volume_fraction = 0.3;
  configuration.void_stiffness_ratio = 1.0e-3;
  configuration.maximum_iterations = 100;
  configuration.density_penalization = 3.0;
  configuration.minimum_density = 1.0e-3;

  ApplyTopologyOptions(configuration, options);

  if (!ValidateTopologyConfiguration(configuration))
  {
    PRINT_ERROR("Invalid topology case configuration.\n");
    return false;
  }

  topology_cases.emplace(topology_id, configuration);
  return true;
}

bool CoreTopology::modify_topology(
    int topology_id,
    const TopologyOptions& options)
{
  if (!is_initialized)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  // Preserve selections, element flags, and iteration records.
  TopologyCase configuration = entry->second;
  ApplyTopologyOptions(configuration, options);

  if (!ValidateTopologyConfiguration(configuration))
  {
    PRINT_ERROR("Invalid topology case configuration.\n");
    return false;
  }

  entry->second = configuration;
  return true;
}

bool CoreTopology::delete_topology(int topology_id)
{
  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  // The coordinating core must remove linked jobs first.
  if (!entry->second.iteration_jobs.empty())
  {
    PRINT_ERROR("Topology case %d still has linked iteration jobs.\n",
                topology_id);
    return false;
  }

  topology_cases.erase(entry);
  return true;
}

bool CoreTopology::resolve_preserved_elements(int topology_id, const std::vector<int>& analysis_element_ids, const std::map<int, std::set<int>>& default_sets, const std::set<int>& default_elements)
{
  if (!is_initialized || !ccx_iface)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (analysis_element_ids.empty())
  {
    PRINT_ERROR("Topology %d has no analysis elements.\n", topology_id);
    return false;
  }

  std::map<int, bool> flags;

  for (int element_id : analysis_element_ids)
  {
    if (element_id <= 0 ||
        !flags.emplace(element_id, false).second)
    {
      PRINT_ERROR("Invalid or duplicate topology element %d.\n",
                  element_id);
      return false;
    }
  }

  const auto& configuration = entry->second;
  auto effective_sets = default_sets;

  for (const auto& group : configuration.added_preserved_sets)
    effective_sets[group.first].insert(
        group.second.begin(), group.second.end());

  for (const auto& group : configuration.removed_preserved_sets)
    for (int set_id : group.second)
      effective_sets[group.first].erase(set_id);

  for (const auto& group : effective_sets)
  {
    const int set_type = group.first;

    if (set_type < 1 || set_type > 3)
    {
      PRINT_ERROR("Invalid default preserved set type %d.\n",
                  set_type);
      return false;
    }

    for (int set_id : group.second)
    {
      std::vector<int> element_ids;

      if (!ccx_iface->get_topology_set_element_ids(
              topology_id, set_type, set_id, element_ids))
        return false;

      for (int element_id : element_ids)
      {
        const auto flag = flags.find(element_id);

        if (flag == flags.end())
        {
          PRINT_ERROR("Unexpected topology element %d.\n", element_id);
          return false;
        }

        flag->second = true;
      }
    }
  }

  for (int element_id : default_elements)
  {
    if (element_id <= 0)
    {
      PRINT_ERROR("Invalid default preserved element %d.\n",
                  element_id);
      return false;
    }

    const auto flag = flags.find(element_id);

    // Only elements participating in this topology analysis are marked.
    if (flag != flags.end())
      flag->second = true;
  }

  // Commit only after all queries succeed.
  entry->second.element_preserved.swap(flags);
  return true;
}

bool CoreTopology::set_iteration_element_energies(int topology_id,int iteration_id,const std::map<int, double>& element_energies)
{
  if (!is_initialized || iteration_id < 0)
  {
    PRINT_ERROR("Invalid topology energy storage request.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;

  if (configuration.iteration_jobs.count(iteration_id) == 0 ||
    configuration.iteration_element_energies.count(iteration_id) != 0 ||
    configuration.iteration_total_energies.count(iteration_id) != 0)
  {
    PRINT_ERROR("Missing job or already stored iteration energies.\n");
    return false;
  }

  if (element_energies.empty() || element_energies.size() != configuration.element_preserved.size())
  {
    PRINT_ERROR("Incomplete topology iteration energies.\n");
    return false;
  }

  double total_energy = 0.0;

  for (const auto& energy : element_energies)
  {
    if (configuration.element_preserved.count(energy.first) == 0 || !std::isfinite(energy.second))
    {
      PRINT_ERROR("Invalid energy for topology element %d.\n", energy.first);
      return false;
    }

    total_energy += energy.second;
    if (!std::isfinite(total_energy))
    {
      PRINT_ERROR("Invalid total energy for topology %d iteration %d.\n", topology_id, iteration_id);
      return false;
    }
  }

  configuration.iteration_element_energies.emplace(iteration_id, element_energies);
  configuration.iteration_total_energies.emplace(iteration_id, total_energy);
  PRINT_INFO("Topology %d iteration %d: total internal energy %.9g.\n", topology_id, iteration_id, total_energy);

  return true;
}

bool CoreTopology::evaluate_compliance(int topology_id,int iteration_id,const std::map<int, double>& stiffness_factors)
{
  if (!is_initialized || iteration_id < 0)
  {
    PRINT_ERROR("Invalid topology compliance evaluation request.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;
  const auto energies = configuration.iteration_element_energies.find(iteration_id);
  const auto total = configuration.iteration_total_energies.find(iteration_id);

  if (configuration.objective != TopologyObjective::Compliance ||
      configuration.iteration_jobs.count(iteration_id) == 0 ||
      energies == configuration.iteration_element_energies.end() ||
      total == configuration.iteration_total_energies.end() ||
      configuration.iteration_evaluations.count(iteration_id) != 0)
  {
    PRINT_ERROR("Missing data or already evaluated topology iteration.\n");
    return false;
  }

  if (energies->second.empty() ||
      energies->second.size() != stiffness_factors.size() ||
      energies->second.size() != configuration.element_preserved.size())
  {
    PRINT_ERROR("Incomplete topology compliance data.\n");
    return false;
  }

  TopologyIterationEvaluation evaluation;
  evaluation.objective = TopologyObjective::Compliance;
  evaluation.value = 2.0 * total->second;

  if (!std::isfinite(evaluation.value) || evaluation.value < 0.0)
  {
    PRINT_ERROR("Invalid topology compliance value.\n");
    return false;
  }

  for (const auto& energy : energies->second)
  {
    const auto factor = stiffness_factors.find(energy.first);
    const auto preserved = configuration.element_preserved.find(energy.first);

    if (factor == stiffness_factors.end() ||
        preserved == configuration.element_preserved.end() ||
        !std::isfinite(factor->second) ||
        factor->second <= 0.0 ||
        factor->second > 1.0 ||
        !std::isfinite(energy.second) ||
        energy.second < 0.0)
    {
      PRINT_ERROR("Invalid compliance data for element %d.\n", energy.first);
      return false;
    }

    double sensitivity = 0.0;

    if (!preserved->second)
    {
      sensitivity = -2.0 * energy.second / factor->second;
    }

    if (!std::isfinite(sensitivity))
    {
      PRINT_ERROR("Invalid compliance sensitivity for element %d.\n", energy.first);
      return false;
    }

    evaluation.stiffness_sensitivities.emplace(energy.first, sensitivity);
  }

  /*
  s(ρ) = v + (1 − v)ρᵖ
  dC/dρ = (dC/ds) × (1 − v)pρᵖ⁻¹
  */
  const auto density_design = configuration.iteration_density_designs.find(iteration_id);

  if (density_design != configuration.iteration_density_designs.end())
  {
    const auto& design = density_design->second;

    if (design.element_densities.size() !=
            evaluation.stiffness_sensitivities.size() ||
        !std::isfinite(design.penalization) ||
        design.penalization < 1.0 ||
        !std::isfinite(design.void_ratio) ||
        design.void_ratio <= 0.0 ||
        design.void_ratio >= 1.0)
    {
      PRINT_ERROR("Invalid stored topology density design.\n");
      return false;
    }

    for (const auto& density : design.element_densities)
    {
      const auto stiffness = evaluation.stiffness_sensitivities.find(density.first);
      const auto preserved = configuration.element_preserved.find(density.first);

      if (stiffness == evaluation.stiffness_sensitivities.end() ||
          preserved == configuration.element_preserved.end() ||
          !std::isfinite(density.second) ||
          density.second < 0.0 ||
          density.second > 1.0)
      {
        PRINT_ERROR("Invalid density sensitivity data for element %d.\n", density.first);
        return false;
      }

      double sensitivity = 0.0;

      if (!preserved->second)
      {
        // Handle p == 1 explicitly, including rho == 0.
        double power = 1.0;

        if (design.penalization > 1.0)
        {
          power = std::pow(density.second, design.penalization - 1.0);
        }

        const double factor_derivative = (1.0 - design.void_ratio) * design.penalization * power;

        sensitivity = stiffness->second * factor_derivative;
      }

      if (!std::isfinite(sensitivity))
      {
        PRINT_ERROR("Invalid density sensitivity for element %d.\n", density.first);
        return false;
      }

      evaluation.density_sensitivities.emplace(density.first, sensitivity);
    }
  }

  configuration.iteration_evaluations.emplace(iteration_id, evaluation);
  PRINT_INFO("Topology %d iteration %d: compliance %.9g.\n", topology_id, iteration_id, evaluation.value);

  return true;
}

bool CoreTopology::build_density_design(int topology_id,int iteration_id,const std::map<int, double>& densities,TopologyDensityDesign& design,std::map<int, double>& stiffness_factors)
{
  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized || iteration_id < 0 || entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid topology density request.\n");
    return false;
  }

  const auto& configuration = entry->second;

  if (configuration.method != TopologyMethod::Density ||
      configuration.iteration_jobs.count(iteration_id) != 0 ||
      configuration.iteration_density_designs.count(iteration_id) != 0)
  {
    PRINT_ERROR("Density method required or iteration already exists.\n");
    return false;
  }

  if (densities.empty() ||
      densities.size() != configuration.element_preserved.size() ||
      !std::isfinite(configuration.density_penalization) ||
      configuration.density_penalization < 1.0 ||
      !std::isfinite(configuration.void_stiffness_ratio) ||
      configuration.void_stiffness_ratio <= 0.0 ||
      configuration.void_stiffness_ratio >= 1.0 ||
      !std::isfinite(configuration.minimum_density) ||
      configuration.minimum_density <= 0.0 ||
      configuration.minimum_density >= 1.0)
  {
    PRINT_ERROR("Invalid density data or interpolation settings.\n");
    return false;
  }

  TopologyDensityDesign candidate;
  candidate.penalization = configuration.density_penalization;
  candidate.void_ratio = configuration.void_stiffness_ratio;
  candidate.minimum_density = configuration.minimum_density;
  std::map<int, double> factors;

  for (const auto& density : densities)
  {
    const auto preserved = configuration.element_preserved.find(density.first);

    if (preserved == configuration.element_preserved.end() ||
        !std::isfinite(density.second) ||
        density.second < 0.0 || density.second > 1.0)
    {
      PRINT_ERROR("Invalid density for element %d.\n", density.first);
      return false;
    }

    if (!preserved->second && density.second < candidate.minimum_density)
    {
      PRINT_ERROR("Density for element %d is below the minimum %.9g.\n", density.first, candidate.minimum_density);
      return false;
    }

    const double rho = preserved->second ? 1.0 : density.second;

    const double factor = candidate.void_ratio + (1.0 - candidate.void_ratio) * std::pow(rho, candidate.penalization);

    if (!std::isfinite(factor) || factor <= 0.0 || factor > 1.0)
    {
      PRINT_ERROR("Invalid stiffness factor for element %d.\n", density.first);
      return false;
    }

    candidate.element_densities.emplace(density.first, rho);
    factors.emplace(density.first, factor);
  }

  design = std::move(candidate);
  stiffness_factors.swap(factors);
  return true;
}

bool CoreTopology::evaluate_density_volume(int topology_id,int iteration_id,const std::map<int, double>& element_volumes)
{
  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized || iteration_id < 0 || entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid topology volume evaluation request.\n");
    return false;
  }

  auto& configuration = entry->second;
  const auto design = configuration.iteration_density_designs.find(iteration_id);

  if (design == configuration.iteration_density_designs.end())
  {
    PRINT_ERROR("Missing topology density design.\n");
    return false;
  }

  const auto& densities = design->second.element_densities;
  const double minimum_density = design->second.minimum_density;

  if (!std::isfinite(minimum_density) ||
      minimum_density <= 0.0 ||
      minimum_density >= 1.0)
  {
    PRINT_ERROR("Invalid stored minimum density.\n");
    return false;
  }

  if (element_volumes.empty() ||
      element_volumes.size() != densities.size() ||
      element_volumes.size() != configuration.element_preserved.size())
  {
    PRINT_ERROR("Incomplete topology element volumes.\n");
    return false;
  }

  TopologyVolumeEvaluation evaluation;

  for (const auto& element : element_volumes)
  {
    const auto density = densities.find(element.first);
    const auto preserved = configuration.element_preserved.find(element.first);

    if (density == densities.end() ||
        preserved == configuration.element_preserved.end() ||
        !std::isfinite(element.second) ||
        element.second <= 0.0 ||
        !std::isfinite(density->second) ||
        density->second < minimum_density ||
        density->second > 1.0)
    {
      PRINT_ERROR("Invalid volume data for element %d.\n", element.first);
      return false;
    }

    if (preserved->second && density->second != 1.0)
    {
      PRINT_ERROR("Preserved element %d must have density 1.\n", element.first);
      return false;
    }

    evaluation.total_volume += element.second;
    evaluation.solid_volume += density->second * element.second;

    if (preserved->second)
    {
      evaluation.preserved_volume += element.second;
    }
  }

  if (!std::isfinite(evaluation.total_volume) ||
      evaluation.total_volume <= 0.0 ||
      !std::isfinite(evaluation.solid_volume) ||
      !std::isfinite(evaluation.preserved_volume))
  {
    PRINT_ERROR("Invalid accumulated topology volumes.\n");
    return false;
  }

  evaluation.volume_fraction = evaluation.solid_volume / evaluation.total_volume;
  /*
    minimum fraction = preserved fraction + minimum density × free fraction
  */
  evaluation.minimum_volume_fraction = evaluation.preserved_volume / evaluation.total_volume;
  evaluation.minimum_design_volume_fraction = evaluation.minimum_volume_fraction + minimum_density * (1.0 - evaluation.minimum_volume_fraction);

  for (const auto& element : element_volumes)
  {
    const bool preserved = configuration.element_preserved.find(element.first)->second;

    const double sensitivity = preserved ? 0.0 : element.second / evaluation.total_volume;

    evaluation.density_sensitivities.emplace(element.first, sensitivity);
  }

  evaluation.available = true;
  design->second.volume = std::move(evaluation);

  PRINT_INFO(
      "Topology %d iteration %d: volume fraction %.9g; "
      "preserved minimum %.9g.\n",
      topology_id, iteration_id,
      design->second.volume.volume_fraction,
      design->second.volume.minimum_volume_fraction);

  return true;
}

bool CoreTopology::build_density_update(int topology_id,int source_iteration,std::map<int, double>& densities)
{
  densities.clear();

  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized || source_iteration < 0 || entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid topology density update request.\n");
    return false;
  }

  const auto& configuration = entry->second;
  const auto design = configuration.iteration_density_designs.find(source_iteration);
  const auto evaluation = configuration.iteration_evaluations.find(source_iteration);

  if (configuration.method != TopologyMethod::Density ||
      configuration.objective != TopologyObjective::Compliance ||
      design == configuration.iteration_density_designs.end() ||
      evaluation == configuration.iteration_evaluations.end() ||
      evaluation->second.objective != TopologyObjective::Compliance ||
      !design->second.volume.available)
  {
    PRINT_ERROR("Missing evaluated compliance density iteration.\n");
    return false;
  }

  const auto& previous = design->second.element_densities;
  const auto& gradients = evaluation->second.density_sensitivities;
  const auto& volume = design->second.volume;
  const double minimum_density = design->second.minimum_density;

  if (previous.empty() ||
    previous.size() != gradients.size() ||
    previous.size() != volume.density_sensitivities.size() ||
    previous.size() != configuration.element_preserved.size() ||
    !std::isfinite(configuration.target_volume_fraction) ||
    configuration.target_volume_fraction <= 0.0 ||
    configuration.target_volume_fraction > 1.0 ||
    !std::isfinite(minimum_density) ||
    minimum_density <= 0.0 ||
    minimum_density >= 1.0 ||
    !std::isfinite(volume.minimum_volume_fraction) ||
    volume.minimum_volume_fraction < 0.0 ||
    volume.minimum_volume_fraction > 1.0 ||
    !std::isfinite(volume.minimum_design_volume_fraction) ||
    volume.minimum_design_volume_fraction < 0.0 ||
    volume.minimum_design_volume_fraction > 1.0 ||
    configuration.target_volume_fraction + 1.0e-12 <
        volume.minimum_design_volume_fraction)
  {
    PRINT_ERROR("Invalid topology density update data.\n");
    return false;
  }

  // Initial algorithm defaults belong to CoreTopology.
  constexpr double move_limit = 0.2;
  constexpr double volume_tolerance = 1.0e-10;

  struct Variable
  {
    int id;
    double density;
    double lower;
    double upper;
    double volume_weight;
    double benefit;
  };

  std::vector<Variable> variables;
  std::map<int, double> fixed;
  double minimum_reachable = volume.minimum_volume_fraction;
  double maximum_benefit = 0.0;

  for (const auto& density : previous)
  {
    const auto preserved = configuration.element_preserved.find(density.first);
    const auto gradient = gradients.find(density.first);
    const auto weight = volume.density_sensitivities.find(density.first);

    if (preserved == configuration.element_preserved.end() ||
        gradient == gradients.end() ||
        weight == volume.density_sensitivities.end() ||
        !std::isfinite(density.second) ||
        density.second < minimum_density || density.second > 1.0 ||
        !std::isfinite(gradient->second) ||
        gradient->second > 0.0 ||
        !std::isfinite(weight->second))
    {
      PRINT_ERROR("Invalid update data for element %d.\n", density.first);
      return false;
    }

    if (preserved->second)
    {
      if (density.second != 1.0 || weight->second != 0.0)
      {
        PRINT_ERROR("Invalid preserved element %d.\n", density.first);
        return false;
      }

      fixed.emplace(density.first, 1.0);
      continue;
    }

    if (weight->second <= 0.0)
    {
      PRINT_ERROR("Invalid volume derivative for element %d.\n", density.first);
      return false;
    }

    const double lower = std::max(minimum_density, density.second - move_limit);
    const double upper = std::min(1.0, density.second + move_limit);
    const double benefit = -gradient->second / weight->second;

    if (!std::isfinite(benefit))
    {
      PRINT_ERROR("Invalid update benefit for element %d.\n", density.first);
      return false;
    }

    variables.push_back({density.first, density.second, lower, upper, weight->second, benefit});
    minimum_reachable += lower * weight->second;
    maximum_benefit = std::max(maximum_benefit, benefit);
  }

  if (!std::isfinite(minimum_reachable))
  {
    PRINT_ERROR("Invalid reachable topology volume.\n");
    return false;
  }

  // Respect the move limit while approaching the final target.
  const double target = std::max(configuration.target_volume_fraction, minimum_reachable);

  // Normalize benefits to reduce dependence on model units.
  for (auto& variable : variables)
  {
    if (maximum_benefit > 0.0)
    {
      variable.benefit /= maximum_benefit;
    }
  }

  const auto make_trial = [&](double multiplier, std::map<int, double>& trial)
  {
    trial = fixed;
    double fraction = volume.minimum_volume_fraction;

    for (const auto& variable : variables)
    {
      double density = variable.lower;

      if (variable.benefit > 0.0)
      {
        if (multiplier == 0.0)
        {
          density = variable.upper;
        }
        else
        {
          density = variable.density * std::sqrt(variable.benefit / multiplier);

          density = std::max(variable.lower, std::min(variable.upper, density));
        }
      }
      trial.emplace(variable.id, density);
      fraction += density * variable.volume_weight;
    }
    return fraction;
  };

  std::map<int, double> candidate;
  double fraction = make_trial(0.0, candidate);

  if (fraction <= target + volume_tolerance)
  {
    densities.swap(candidate);
    return true;
  }

  double lower_multiplier = 0.0;
  double upper_multiplier = 1.0;

  // Find a multiplier yielding a volume-feasible candidate.
  for (int i = 0; i < 100; ++i)
  {
    fraction = make_trial(upper_multiplier, candidate);
    if (fraction <= target + volume_tolerance)
    {
      break;
    }

    upper_multiplier *= 2.0;
  }

  if (!std::isfinite(fraction) || fraction > target + volume_tolerance)
  {
    PRINT_ERROR("Unable to bracket the topology volume constraint.\n");
    return false;
  }

  // Keep the feasible candidate while refining the multiplier.
  std::map<int, double> feasible = candidate;

  for (int i = 0; i < 100; ++i)
  {
    const double multiplier = 0.5 * (lower_multiplier + upper_multiplier);

    fraction = make_trial(multiplier, candidate);

    if (!std::isfinite(fraction))
    {
      PRINT_ERROR("Invalid trial topology volume.\n");
      return false;
    }

    if (fraction > target)
    {
      lower_multiplier = multiplier;
    }
    else
    {
      upper_multiplier = multiplier;
      feasible = candidate;

      if (target - fraction <= volume_tolerance)
      {
        break;
      }
    }
  }

  densities.swap(feasible);
  return true;
}

bool CoreTopology::check_density_convergence(int topology_id,int previous_iteration, int current_iteration, bool& converged)
{
  converged = false;

  const auto entry = topology_cases.find(topology_id);

  if (!is_initialized ||
      previous_iteration < 0 ||
      current_iteration <= previous_iteration ||
      entry == topology_cases.end())
  {
    PRINT_ERROR("Invalid topology convergence request.\n");
    return false;
  }

  const auto& configuration = entry->second;
  const auto previous = configuration.iteration_density_designs.find(previous_iteration);
  const auto current = configuration.iteration_density_designs.find(current_iteration);
  const auto previous_evaluation = configuration.iteration_evaluations.find(previous_iteration);
  const auto current_evaluation = configuration.iteration_evaluations.find(current_iteration);

  if (configuration.method != TopologyMethod::Density ||
      configuration.objective != TopologyObjective::Compliance ||
      previous == configuration.iteration_density_designs.end() ||
      current == configuration.iteration_density_designs.end() ||
      previous_evaluation == configuration.iteration_evaluations.end() ||
      current_evaluation == configuration.iteration_evaluations.end() ||
      previous_evaluation->second.objective != TopologyObjective::Compliance ||
      current_evaluation->second.objective != TopologyObjective::Compliance ||
      !current->second.volume.available)
  {
    PRINT_ERROR("Missing topology convergence data.\n");
    return false;
  }

  const auto& previous_densities = previous->second.element_densities;
  const auto& current_densities = current->second.element_densities;

  if (previous_densities.empty() ||
      previous_densities.size() != current_densities.size())
  {
    PRINT_ERROR("Incomplete topology convergence densities.\n");
    return false;
  }

  double maximum_change = 0.0;

  for (const auto& density : current_densities)
  {
    const auto old = previous_densities.find(density.first);

    if (old == previous_densities.end() ||
        !std::isfinite(density.second) ||
        !std::isfinite(old->second) ||
        density.second < 0.0 || density.second > 1.0 ||
        old->second < 0.0 || old->second > 1.0)
    {
      PRINT_ERROR("Invalid convergence density for element %d.\n",
                  density.first);
      return false;
    }

    maximum_change = std::max(maximum_change, std::abs(density.second - old->second));
  }

  const double previous_value = previous_evaluation->second.value;
  const double current_value = current_evaluation->second.value;
  const double fraction = current->second.volume.volume_fraction;

  if (!std::isfinite(previous_value) ||
      !std::isfinite(current_value) ||
      previous_value < 0.0 || current_value < 0.0 ||
      !std::isfinite(fraction) ||
      fraction < 0.0 || fraction > 1.0 ||
      !std::isfinite(configuration.target_volume_fraction) ||
      configuration.target_volume_fraction <= 0.0 ||
      configuration.target_volume_fraction > 1.0)
  {
    PRINT_ERROR("Invalid topology convergence values.\n");
    return false;
  }

  const double scale = std::max(previous_value, current_value);
  double relative_change = 0.0;

  if (scale > 0.0)
  {
    relative_change = std::abs(current_value / scale - previous_value / scale);
  }

  constexpr double density_tolerance = 0.01;
  constexpr double compliance_tolerance = 0.001;
  constexpr double volume_tolerance = 1.0e-6;

  converged =
      maximum_change <= density_tolerance &&
      relative_change <= compliance_tolerance &&
      fraction <= configuration.target_volume_fraction + volume_tolerance;

  PRINT_INFO(
      "Topology %d iteration %d: density change %.9g; "
      "relative compliance change %.9g; volume fraction %.9g.\n",
      topology_id, current_iteration,
      maximum_change, relative_change, fraction);

  return true;
}

std::vector<std::vector<std::string>> CoreTopology::get_tree_data() const
{
  std::vector<std::vector<std::string>> rows;

  for (const auto& entry : topology_cases)
  {
    rows.push_back({std::to_string(entry.first), entry.second.name});
  }

  return rows;
}

bool CoreTopology::change_preserved_sets(int topology_id,int set_type,const std::vector<int>& ids,bool adding)
{
  if (!is_initialized)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (set_type < 1 || set_type > 3)
  {
    PRINT_ERROR("Invalid topology preserved set type.\n");
    return false;
  }

  // Validate the complete list before changing configuration.
  for (int id : ids)
  {
    bool exists = false;

    if (id > 0)
    {
      if (set_type == 1)
        exists = ccx_iface->check_block_exists(id);
      else if (set_type == 2)
        exists = ccx_iface->check_nodeset_exists(id);
      else
        exists = ccx_iface->check_sideset_exists(id);
    }

    if (!exists)
    {
      PRINT_ERROR("Preserved set type %d, ID %d does not exist.\n", set_type, id);
      return false;
    }
  }

  auto& configuration = entry->second;

  for (int id : ids)
  {
    if (adding)
    {
      configuration.removed_preserved_sets[set_type].erase(id);
      configuration.added_preserved_sets[set_type].insert(id);
    }
    else
    {
      configuration.added_preserved_sets[set_type].erase(id);
      configuration.removed_preserved_sets[set_type].insert(id);
    }
  }

  return true;
}

bool CoreTopology::add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  return change_preserved_sets(topology_id, set_type, ids, true);
}

bool CoreTopology::remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  return change_preserved_sets(topology_id, set_type, ids, false);
}