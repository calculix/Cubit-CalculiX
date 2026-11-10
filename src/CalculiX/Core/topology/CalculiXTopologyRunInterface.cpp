#include "CalculiXTopologyRunInterface.hpp"
#include "Core/topology/CoreTopologyDisplacementResults.hpp"
#include "Core/topology/CoreTopologyModelValidation.hpp"
#include "Core/topology/CoreTopologyTensorResults.hpp"
#include "Core/topology/CoreTopologyResults.hpp"
#include "Core/topology/CoreTopologyJobs.hpp"
#include "Core/topology/CoreTopologyDensity.hpp"
#include "Core/topology/CoreTopologyGeometry.hpp"
#include "Core/topology/CoreTopologyOptimization.hpp"
#include "Core/topology/CoreTopologyEvaluation.hpp"
#include "Core/topology/CoreTopologyIteration.hpp"
#include "Core/topology/CoreTopologyQuery.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <array>
#include <utility>

#include "Core/topology/CoreTopology.hpp"
#include "CubitMessage.hpp"
#include "Core/loadCalculiXCore.hpp"

bool CalculiXTopologyRunInterface::begin_topology_run(int topology_id)
{
  return ccx_core.begin_topology_run(topology_id);
}

bool CalculiXTopologyRunInterface::set_topology_run_state(int topology_id, TopologyRunState state)
{
  if (!ccx_core.topology)
  {
    return false;
  }

  return ccx_core.topology->set_topology_run_state(topology_id, state);
}

bool CalculiXTopologyRunInterface::get_topology_base_run_info(int topology_id, TopologyRunInfo& information)
{
  if (!ccx_core.topology)
  {
    information = {};
    return false;
  }

  return ccx_core.topology->get_topology_base_run_info(topology_id, information);
}

int CalculiXTopologyRunInterface::get_topology_active_iteration_job(int topology_id, int iteration_id)
{
  if (!ccx_core.topology || !ccx_core.jobs)
  {
    return -1;
  }

  return CoreTopologyQuery::get_active_iteration_job(*ccx_core.topology, *ccx_core.jobs, topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::prepare_topology_run(int topology_id)
{
  if (!ccx_core.topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return false;
  }

  return ccx_core.topologyoptimization->prepare_topology_run(topology_id);
}

bool CalculiXTopologyRunInterface::collect_topology_run_sections(int topology_id, std::vector<int>& section_ids, std::set<int>& block_ids)
{
  return ccx_core.collect_topology_run_sections(topology_id, section_ids, block_ids);
}

bool CalculiXTopologyRunInterface::prepare_topology_history_outputs(int topology_id, const std::set<int>& block_ids)
{
  return ccx_core.prepare_topology_history_outputs(topology_id, block_ids);
}

bool CalculiXTopologyRunInterface::initialize_topology_run_design(int topology_id, const std::vector<int>& section_ids)
{
  return ccx_core.initialize_topology_run_design(topology_id, section_ids);
}

int CalculiXTopologyRunInterface::start_topology_iteration(int topology_id, int iteration_id)
{
  if (!ccx_core.topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return -1;
  }

  return ccx_core.topologyoptimization->start_topology_iteration(topology_id, iteration_id);
}

int CalculiXTopologyRunInterface::create_topology_iteration_job(int topology_id, int iteration_id, const std::string& job_name)
{
  return ccx_core.create_topology_iteration_job(topology_id, iteration_id, job_name);
}

bool CalculiXTopologyRunInterface::create_topology_adjoint_job(int topology_id, int iteration_id, int& job_id, bool& skipped)
{
  job_id = -1;
  skipped = false;

  if (!ccx_core.topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return false;
  }

  return ccx_core.topologyoptimization->create_topology_adjoint_job(topology_id, iteration_id, job_id, skipped);
}

int CalculiXTopologyRunInterface::get_topology_adjoint_job_source(int topology_id, int iteration_id)
{
  if (!ccx_core.topology || !ccx_core.jobs)
  {
    PRINT_ERROR("Invalid topology adjoint job request.\n");
    return -1;
  }

  return CoreTopologyQuery::get_adjoint_job_source(*ccx_core.topology, *ccx_core.jobs, topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::register_topology_adjoint_job(int topology_id, int iteration_id, int& job_id)
{
  job_id = -1;

  if (!ccx_core.topology || !ccx_core.jobs)
  {
    PRINT_ERROR("Invalid topology adjoint job request.\n");
    return false;
  }

  return CoreTopologyJobs::register_adjoint_job(*ccx_core.topology, *ccx_core.jobs, topology_id, iteration_id, job_id);
}

bool CalculiXTopologyRunInterface::build_topology_adjoint_load(int topology_id, int iteration_id, double& objective_value, int& node_id, std::array<double, 3>& load)
{
  objective_value = 0.0;
  node_id = -1;
  load.fill(0.0);

  if (!ccx_core.topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return false;
  }

  return ccx_core.topologyoptimization->build_topology_adjoint_load(topology_id, iteration_id, objective_value, node_id, load);
}

bool CalculiXTopologyRunInterface::check_topology_displacement_selection(int topology_id, int iteration_id)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Invalid topology displacement selection request.\n");
    return false;
  }

  return CoreTopologyModelValidation::validate_displacement_selection(*ccx_core.topology, topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::check_topology_adjoint_results(int job_id)
{
  std::map<int, std::array<double, 3>> displacements;
  return read_topology_job_displacements(job_id, displacements);
}

bool CalculiXTopologyRunInterface::read_topology_job_displacements(int job_id, std::map<int, std::array<double, 3>>& displacements)
{
  displacements.clear();

  if (!ccx_core.results || !ccx_core.jobs || !ccx_core.topology || !ccx_core.mat)
  {
    PRINT_ERROR("Topology displacement results require a completed job.\n");
    return false;
  }

  return CoreTopologyResults::read_job_displacements(*ccx_core.results, *ccx_core.jobs, *ccx_core.topology, *ccx_core.mat, ccx_core.cb, job_id, displacements);
}

bool CalculiXTopologyRunInterface::finish_topology_iteration(int topology_id, int iteration_id)
{
  if (!ccx_core.topologyoptimization)
  {
    return false;
  }

  return ccx_core.topologyoptimization->finish_topology_iteration(topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::evaluate_topology_compliance(int topology_id, int iteration_id, const std::map<int, double>& energies, CoreTopologyLevelSetGeometryCache* cache)
{
  if (ccx_core.topology)
  {
    const auto entry = ccx_core.topology->topology_cases.find(topology_id);
    if (entry != ccx_core.topology->topology_cases.end() && entry->second.method == TopologyMethod::LevelSet)
    {
      return ccx_core.evaluate_topology_level_set_compliance(topology_id, iteration_id, energies, cache);
    }
  }

  if (!ccx_core.topology || !ccx_core.mat)
  {
    PRINT_ERROR("Invalid topology compliance evaluation request.\n");
    return false;
  }

  return CoreTopologyEvaluation::evaluate_compliance(*ccx_core.topology, *ccx_core.mat, topology_id, iteration_id, energies);
}

bool CalculiXTopologyRunInterface::evaluate_topology_displacement(int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals)
{
  if (!ccx_core.topology || !ccx_core.mat || !ccx_core.jobs)
  {
    PRINT_ERROR("Invalid topology displacement evaluation request.\n");
    return false;
  }

  return CoreTopologyEvaluation::evaluate_displacement(*ccx_core.topology, *ccx_core.mat, *ccx_core.jobs, topology_id, iteration_id, value, node_id, element_integrals);
}

bool CalculiXTopologyRunInterface::set_topology_iteration_displacement(int topology_id, int iteration_id, double displacement, int node_id)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Invalid topology displacement measurement.\n");
    return false;
  }

  return CoreTopologyIteration::set_displacement(*ccx_core.topology, topology_id, iteration_id, displacement, node_id);
}

bool CalculiXTopologyRunInterface::mark_topology_iteration_completed(int topology_id, int iteration_id)
{
  if (!ccx_core.topology || !ccx_core.jobs)
  {
    PRINT_ERROR("Invalid topology iteration completion request.\n");
    return false;
  }

  return CoreTopologyIteration::mark_completed(*ccx_core.topology, *ccx_core.jobs, topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::read_topology_element_energies(int topology_id, int iteration_id, std::map<int, double>& element_energies)
{
  element_energies.clear();

  if (!ccx_core.topology || !ccx_core.mat || !ccx_core.results || !ccx_core.jobs)
  {
    PRINT_ERROR("Invalid topology energy query.\n");
    return false;
  }

  return CoreTopologyResults::read_iteration_energies(*ccx_core.results, *ccx_core.jobs, *ccx_core.topology, *ccx_core.mat, topology_id, iteration_id, element_energies);
}

bool CalculiXTopologyRunInterface::read_topology_job_element_tensors(int job_id, const std::string& key, std::map<int, std::map<int, std::array<double, 6>>>& tensors)
{
  tensors.clear();

  if (key != "S" && key != "ME")
  {
    PRINT_ERROR("Topology tensor reader supports S and ME only.\n");
    return false;
  }

  if (!ccx_core.topology || !ccx_core.mat || !ccx_core.results || !ccx_core.jobs)
  {
    PRINT_ERROR("Topology tensor results require a completed job.\n");
    return false;
  }

  return CoreTopologyTensorResults::read_job(*ccx_core.results, *ccx_core.jobs, *ccx_core.topology, *ccx_core.mat, job_id, key, tensors);
}

bool CalculiXTopologyRunInterface::read_topology_displacement(int topology_id, int iteration_id, double& value, int& node_id, std::array<double, 3>* critical_displacement)
{
  value = 0.0;
  node_id = -1;

  if (critical_displacement)
  {
    critical_displacement->fill(0.0);
  }

  if (!ccx_core.topology || !ccx_core.results || !ccx_core.jobs || !ccx_core.me_iface)
  {
    PRINT_ERROR("Topology displacement interfaces are not initialized.\n");
    return false;
  }

  return CoreTopologyDisplacementResults::measure_iteration(*ccx_core.results, *ccx_core.jobs, *ccx_core.topology, *ccx_core.me_iface, topology_id, iteration_id, value, node_id, critical_displacement);
}

bool CalculiXTopologyRunInterface::prepare_topology_density_iteration(int topology_id, int source_iteration, int next_iteration)
{
  if (!ccx_core.topologyoptimization)
  {
    return false;
  }

  return ccx_core.topologyoptimization->prepare_topology_density_iteration(topology_id, source_iteration, next_iteration);
}

bool CalculiXTopologyRunInterface::collect_topology_element_volumes(int topology_id, std::map<int, double>& element_volumes, std::map<int, std::array<double, 3>>* element_centers)
{
  element_volumes.clear();

  if (element_centers)
  {
    element_centers->clear();
  }

  if (!ccx_core.mat)
  {
    PRINT_ERROR("Material core is not initialized.\n");
    return false;
  }

  if (!ccx_core.cb)
  {
    PRINT_ERROR("Block core is not initialized.\n");
    return false;
  }

  return CoreTopologyGeometry::collect_element_volumes(*ccx_core.mat, *ccx_core.cb, topology_id, element_volumes, element_centers);
}

bool CalculiXTopologyRunInterface::collect_topology_integration_weights(int topology_id, std::map<int, std::map<int, double>>& weights)
{
  weights.clear();

  if (!ccx_core.topology || !ccx_core.mat || !ccx_core.cb)
  {
    PRINT_ERROR("Topology integration weight interfaces are not initialized.\n");
    return false;
  }

  return CoreTopologyGeometry::collect_integration_weights(*ccx_core.topology, *ccx_core.mat, *ccx_core.cb, topology_id, weights);
}

bool CalculiXTopologyRunInterface::filter_topology_density_sensitivities(int topology_id, int iteration_id, const std::map<int, double>& element_volumes, const std::map<int, std::array<double, 3>>& element_centers)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->filter_density_sensitivities(topology_id, iteration_id, element_volumes, element_centers);
}

bool CalculiXTopologyRunInterface::build_topology_density_update(int topology_id, int source_iteration, std::map<int, double>& densities)
{
  densities.clear();

  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->build_density_update(topology_id, source_iteration, densities);
}

bool CalculiXTopologyRunInterface::evaluate_topology_compliance_oc_residual(int topology_id, int iteration_id)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->evaluate_compliance_oc_residual(topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::set_topology_iteration_densities(int topology_id, int iteration_id, const std::map<int, double>& densities, int source_iteration)
{
  if (!ccx_core.topology || !ccx_core.mat)
  {
    PRINT_ERROR("Topology/material cores are not initialized.\n");
    return false;
  }

  if (!ccx_core.cb)
  {
    PRINT_ERROR("Block core is not initialized.\n");
    return false;
  }

  return CoreTopologyDensity::set_densities(*ccx_core.topology, *ccx_core.mat, *ccx_core.cb, topology_id, iteration_id, densities, source_iteration);
}

bool CalculiXTopologyRunInterface::check_topology_density_convergence(int topology_id, int previous_iteration, int current_iteration, bool& converged)
{
  converged = false;

  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->check_density_convergence(topology_id, previous_iteration, current_iteration, converged);
}

bool CalculiXTopologyRunInterface::evaluate_topology_displacement_projected_residual(int topology_id, int iteration_id)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->evaluate_displacement_projected_residual(topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::initialize_topology_level_set_design(int topology_id)
{
  return ccx_core.initialize_topology_level_set_design(topology_id);
}

bool CalculiXTopologyRunInterface::evaluate_topology_level_set_compliance(int topology_id, int iteration_id, const std::map<int, double>& energies, CoreTopologyLevelSetGeometryCache* cache)
{
  return ccx_core.evaluate_topology_level_set_compliance(topology_id, iteration_id, energies, cache);
}

bool CalculiXTopologyRunInterface::evaluate_topology_level_set_proposal(int topology_id, int iteration_id, TopologyLevelSetProposal& proposal, std::shared_ptr<CoreTopologyLevelSetGeometryCache> source_cache)
{
  return ccx_core.evaluate_topology_level_set_proposal(topology_id, iteration_id, proposal, std::move(source_cache));
}

bool CalculiXTopologyRunInterface::prepare_topology_level_set_iteration(int topology_id, int source_iteration, int next_iteration, const TopologyLevelSetProposal& proposal, double initial_step_scale)
{
  return ccx_core.prepare_topology_level_set_iteration(topology_id, source_iteration, next_iteration, proposal, initial_step_scale);
}

bool CalculiXTopologyRunInterface::check_topology_level_set_convergence(int topology_id, int previous_iteration, int current_iteration, bool& converged)
{
  return ccx_core.check_topology_level_set_convergence(topology_id, previous_iteration, current_iteration, converged);
}
