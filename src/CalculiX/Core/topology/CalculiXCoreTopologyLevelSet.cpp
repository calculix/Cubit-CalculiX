#include "Core/CalculiXCore.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyLevelSet.hpp"
#include "CoreTopologyLevelSetEvaluation.hpp"
#include "CoreTopologyLevelSetUpdate.hpp"
#include "CalculiXTopologyInterface.hpp"
#include "CalculiXTopologyRunInterface.hpp"
#include "Core/materials/CalculiXMaterialsInterface.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "CubitMessage.hpp"
#include <QScopeGuard>
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
TopologyCase* level_set_case(CoreTopology* topology, int topology_id)
{
  if (topology && topology->is_initialized)
  {
    const auto entry = topology->topology_cases.find(topology_id);
    if (entry != topology->topology_cases.end() && entry->second.method == TopologyMethod::LevelSet && entry->second.objective == TopologyObjective::Compliance)
    {
      return &entry->second;
    }
  }
  PRINT_ERROR("A prepared level-set compliance topology case is required.\n");
  return nullptr;
}

bool completed_job(int topology_id, int iteration_id)
{
  const int job = CalculiXTopologyInterface().get_topology_iteration_job_id(topology_id, iteration_id);
  if (job <= 0 || CalculiXJobsInterface().get_job_status(job) != 2)
  {
    PRINT_ERROR("Completed physical job results are required for level-set evaluation/update.\n");
    return false;
  }
  return true;
}

bool store_design(TopologyCase& configuration, int topology_id, TopologyLevelSetDesign&& design, const std::map<int, double>& factors)
{
  const int iteration = design.iteration_id;
  if (iteration < 1 || configuration.iteration_jobs.count(iteration) != 0 || configuration.iteration_evaluations.count(iteration) != 0 ||
      configuration.iteration_density_designs.count(iteration) != 0 || configuration.iteration_level_set_designs.count(iteration) != 0 ||
      factors.size() != configuration.element_preserved.size() || factors.empty())
  {
    PRINT_ERROR("Level-set destination iteration already exists or is incomplete.\n");
    return false;
  }
  std::vector<int> ids;
  std::vector<double> values;
  for (const auto& factor : factors)
  {
    ids.push_back(factor.first);
    values.push_back(factor.second);
  }
  configuration.iteration_level_set_designs.emplace(iteration, std::move(design));
  bool committed = false;
  const auto discard = qScopeGuard([&]()
  {
    if (!committed)
    {
      configuration.iteration_level_set_designs.erase(iteration);
    }
  });
  if (!CalculiXTopologyInterface().set_topology_iteration_factors(topology_id, iteration, ids, values))
  {
    return false;
  }
  committed = true;
  return true;
}
}

bool CalculiXCore::initialize_topology_level_set_design(int topology_id)
{
  auto* configuration = level_set_case(topology, topology_id);
  if (!configuration)
  {
    return false;
  }
  std::map<int, double> volumes;
  if (!CalculiXTopologyRunInterface().collect_topology_element_volumes(topology_id, volumes))
  {
    return false;
  }
  TopologyLevelSetDesign design;
  std::map<int, double> factors;
  if (!CoreTopologyLevelSet::build_initial_design(configuration->level_set_settings, configuration->void_stiffness_ratio, configuration->element_preserved, volumes, design, factors))
  {
    return false;
  }
  const auto controls = CoreTopologyLevelSet::default_update_controls();
  if (!std::isfinite(configuration->target_volume_fraction) || configuration->target_volume_fraction <= 0.0 || configuration->target_volume_fraction > 1.0 ||
      design.volume.minimum_design_volume_fraction > configuration->target_volume_fraction + controls.volume_tolerance)
  {
    PRINT_ERROR("Invalid level-set target or target below the pinned-element volume bound.\n");
    return false;
  }
  return store_design(*configuration, topology_id, std::move(design), factors);
}

bool CalculiXCore::evaluate_topology_level_set_compliance(int topology_id, int iteration_id, const std::map<int, double>& energies, CoreTopologyLevelSetGeometryCache* cache)
{
  auto* configuration = level_set_case(topology, topology_id);
  if (!configuration || !completed_job(topology_id, iteration_id))
  {
    return false;
  }
  const auto design = configuration->iteration_level_set_designs.find(iteration_id);
  if (design == configuration->iteration_level_set_designs.end() || configuration->iteration_evaluations.count(iteration_id) != 0)
  {
    PRINT_ERROR("Missing level-set design or already evaluated iteration.\n");
    return false;
  }
  std::map<int, double> volumes;
  std::map<int, double> factors;
  if (!CalculiXTopologyRunInterface().collect_topology_element_volumes(topology_id, volumes) ||
      !CalculiXMaterialsInterface().get_topology_iteration_factors(topology_id, iteration_id, factors))
  {
    return false;
  }
  TopologyLevelSetEvaluation evaluation;
  if (!CoreTopologyLevelSetEvaluation::evaluate_compliance(design->second, configuration->element_preserved, volumes, energies, factors, evaluation, cache) ||
      !topology->set_iteration_element_energies(topology_id, iteration_id, energies) || !topology->evaluate_compliance(topology_id, iteration_id, factors))
  {
    return false;
  }
  configuration->iteration_evaluations.at(iteration_id).level_set_evaluation = std::move(evaluation);
  return true;
}

bool CalculiXCore::evaluate_topology_level_set_proposal(int topology_id, int iteration_id, TopologyLevelSetProposal& proposal, std::shared_ptr<CoreTopologyLevelSetGeometryCache> source_cache)
{
  proposal = {};
  auto* configuration = level_set_case(topology, topology_id);
  if (!configuration || !completed_job(topology_id, iteration_id))
  {
    return false;
  }
  const auto design = configuration->iteration_level_set_designs.find(iteration_id);
  const auto evaluation = configuration->iteration_evaluations.find(iteration_id);
  if (design == configuration->iteration_level_set_designs.end() || evaluation == configuration->iteration_evaluations.end() ||
      evaluation->second.method != TopologyMethod::LevelSet || !evaluation->second.level_set_evaluation.available)
  {
    PRINT_ERROR("Solved level-set evaluation is missing.\n");
    return false;
  }
  if (!design->second.free_interface_available)
  {
    PRINT_ERROR("Level-set field is saturated without a resolved free interface; convergence cannot be certified.\n");
    return false;
  }
  std::map<int, double> volumes;
  if (!CalculiXTopologyRunInterface().collect_topology_element_volumes(topology_id, volumes))
  {
    return false;
  }
  TopologyLevelSetProposal candidate;
  if (!CoreTopologyLevelSetUpdate::build_proposal(design->second, evaluation->second.level_set_evaluation, configuration->element_preserved, volumes, configuration->target_volume_fraction, candidate, std::move(source_cache)))
  {
    return false;
  }
  evaluation->second.level_set_proposal = candidate.diagnostics;
  proposal = std::move(candidate);
  return true;
}

bool CalculiXCore::prepare_topology_level_set_iteration(int topology_id, int source_iteration, int next_iteration, const TopologyLevelSetProposal& proposal)
{
  auto* configuration = level_set_case(topology, topology_id);
  if (!configuration || source_iteration < 1 || source_iteration == (std::numeric_limits<int>::max)() ||
      next_iteration != source_iteration + 1 || !completed_job(topology_id, source_iteration))
  {
    PRINT_ERROR("Invalid source/destination for level-set iteration preparation.\n");
    return false;
  }
  const auto source = configuration->iteration_level_set_designs.find(source_iteration);
  const auto evaluation = configuration->iteration_evaluations.find(source_iteration);
  if (source == configuration->iteration_level_set_designs.end() || evaluation == configuration->iteration_evaluations.end() ||
      evaluation->second.method != TopologyMethod::LevelSet || !evaluation->second.level_set_evaluation.available ||
      evaluation->second.level_set_evaluation.iteration_id != source_iteration || !evaluation->second.level_set_proposal.available ||
      evaluation->second.level_set_proposal.source_iteration != source_iteration || !proposal.available || proposal.source_iteration != source_iteration ||
      proposal.diagnostics.target_volume_fraction != configuration->target_volume_fraction)
  {
    PRINT_ERROR("Missing/stale level-set proposal or source evaluation.\n");
    return false;
  }
  std::map<int, double> volumes;
  if (!CalculiXTopologyRunInterface().collect_topology_element_volumes(topology_id, volumes))
  {
    return false;
  }
  TopologyLevelSetDesign design;
  std::map<int, double> factors;
  TopologyLevelSetUpdateDiagnostics diagnostics;
  if (!CoreTopologyLevelSetUpdate::accept_proposal(source->second, proposal, configuration->element_preserved, volumes, design, factors, diagnostics))
  {
    return false;
  }
  return store_design(*configuration, topology_id, std::move(design), factors);
}

bool CalculiXCore::check_topology_level_set_convergence(int topology_id, int previous_iteration, int current_iteration, bool& converged)
{
  converged = false;
  auto* configuration = level_set_case(topology, topology_id);
  if (!configuration || previous_iteration < 1 || previous_iteration == (std::numeric_limits<int>::max)() || current_iteration != previous_iteration + 1)
  {
    PRINT_ERROR("Invalid level-set convergence iteration IDs.\n");
    return false;
  }
  const auto previous = configuration->iteration_level_set_designs.find(previous_iteration);
  const auto current = configuration->iteration_level_set_designs.find(current_iteration);
  const auto old_evaluation = configuration->iteration_evaluations.find(previous_iteration);
  const auto evaluation = configuration->iteration_evaluations.find(current_iteration);
  if (previous == configuration->iteration_level_set_designs.end() || current == configuration->iteration_level_set_designs.end() ||
      old_evaluation == configuration->iteration_evaluations.end() || evaluation == configuration->iteration_evaluations.end() ||
      old_evaluation->second.method != TopologyMethod::LevelSet || evaluation->second.method != TopologyMethod::LevelSet ||
      old_evaluation->second.objective != TopologyObjective::Compliance || evaluation->second.objective != TopologyObjective::Compliance ||
      !old_evaluation->second.level_set_evaluation.available || !evaluation->second.level_set_evaluation.available ||
      old_evaluation->second.level_set_evaluation.iteration_id != previous_iteration || evaluation->second.level_set_evaluation.iteration_id != current_iteration ||
      current->second.source_iteration != previous_iteration ||
      !evaluation->second.level_set_proposal.available || evaluation->second.level_set_proposal.source_iteration != current_iteration ||
      evaluation->second.level_set_proposal.target_volume_fraction != configuration->target_volume_fraction ||
      !current->second.volume.available || !current->second.free_interface_available || current->second.node_values.empty() ||
      current->second.node_values.size() != previous->second.node_values.size() || current->second.fixed_nodes != previous->second.fixed_nodes)
  {
    PRINT_ERROR("Incomplete/current level-set convergence data is missing.\n");
    return false;
  }
  const auto& settings = current->second.settings;
  if (!CoreTopologyLevelSet::validate_settings(settings) || !std::isfinite(configuration->compliance_tolerance) || configuration->compliance_tolerance <= 0.0 || configuration->compliance_tolerance > 1.0)
  {
    PRINT_ERROR("Invalid level-set convergence tolerances.\n");
    return false;
  }
  double maximum = 0.0;
  for (const auto& node : current->second.node_values)
  {
    const auto old = previous->second.node_values.find(node.first);
    if (old == previous->second.node_values.end() || !std::isfinite(node.second) || !std::isfinite(old->second) ||
        node.second < -1.0 || node.second > 1.0 || old->second < -1.0 || old->second > 1.0 ||
        (current->second.fixed_nodes.count(node.first) != 0 && (node.second != 1.0 || old->second != 1.0)))
    {
      PRINT_ERROR("Invalid level-set convergence field.\n");
      return false;
    }
    maximum = (std::max)(maximum, std::abs(node.second - old->second));
  }
  const double before = old_evaluation->second.value;
  const double value = evaluation->second.value;
  const double fraction = current->second.volume.volume_fraction;
  const double residual = evaluation->second.level_set_proposal.ordinary_proposal_residual;
  const auto controls = CoreTopologyLevelSet::default_update_controls();
  if (!std::isfinite(before) || before < 0.0 || !std::isfinite(value) || value < 0.0 || !std::isfinite(fraction) || fraction < 0.0 || fraction > 1.0 ||
      !std::isfinite(residual) || residual < 0.0 || !std::isfinite(configuration->target_volume_fraction) || configuration->target_volume_fraction <= 0.0 || configuration->target_volume_fraction > 1.0)
  {
    PRINT_ERROR("Invalid level-set convergence values.\n");
    return false;
  }
  const double scale = (std::max)(before, value);
  const double relative = scale > 0.0 ? std::abs(before / scale - value / scale) : 0.0;
  converged = maximum <= settings.field_tolerance && residual <= settings.field_tolerance &&
      relative <= configuration->compliance_tolerance && fraction <= configuration->target_volume_fraction + controls.volume_tolerance;
  auto& progress = evaluation->second;
  progress.maximum_level_set_change = maximum;
  progress.level_set_change_available = true;
  progress.relative_objective_change = relative;
  progress.convergence_checked = true;
  progress.converged = converged;
  PRINT_INFO("Topology %d iteration %d level-set convergence:\n", topology_id, current_iteration);
  PRINT_INFO("  Maximum field change: %.6g / %.6g [%s]\n", maximum, settings.field_tolerance, maximum <= settings.field_tolerance ? "PASS" : "WAIT");
  PRINT_INFO("  Ordinary field proposal residual: %.6g / %.6g [%s]\n", residual, settings.field_tolerance, residual <= settings.field_tolerance ? "PASS" : "WAIT");
  PRINT_INFO("  Relative compliance change: %.6g / %.6g [%s]\n", relative, configuration->compliance_tolerance, relative <= configuration->compliance_tolerance ? "PASS" : "WAIT");
  PRINT_INFO("  Volume fraction: %.9g / %.9g [%s]\n", fraction, configuration->target_volume_fraction + controls.volume_tolerance, fraction <= configuration->target_volume_fraction + controls.volume_tolerance ? "PASS" : "WAIT");
  PRINT_INFO("  Converged: %s\n", converged ? "YES" : "NO");
  return true;
}
