#include "CalculiXTopologyRunInterface.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <array>

#include "Core/loadCalculiXCore.hpp"

bool CalculiXTopologyRunInterface::begin_topology_run(int topology_id)
{
  return ccx_core.begin_topology_run(topology_id);
}

bool CalculiXTopologyRunInterface::set_topology_run_state(int topology_id, TopologyRunState state)
{
  return ccx_core.set_topology_run_state(topology_id, state);
}

bool CalculiXTopologyRunInterface::get_topology_base_run_info(int topology_id, TopologyRunInfo& information)
{
  return ccx_core.get_topology_base_run_info(topology_id, information);
}

int CalculiXTopologyRunInterface::get_topology_active_iteration_job(int topology_id, int iteration_id)
{
  return ccx_core.get_topology_active_iteration_job(topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::prepare_topology_run(int topology_id)
{
  return ccx_core.prepare_topology_run(topology_id);
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
  return ccx_core.start_topology_iteration(topology_id, iteration_id);
}

int CalculiXTopologyRunInterface::create_topology_iteration_job(int topology_id, int iteration_id, const std::string& job_name)
{
  return ccx_core.create_topology_iteration_job(topology_id, iteration_id, job_name);
}

bool CalculiXTopologyRunInterface::create_topology_adjoint_job(int topology_id, int iteration_id, int& job_id, bool& skipped)
{
  return ccx_core.create_topology_adjoint_job(topology_id, iteration_id, job_id, skipped);
}

int CalculiXTopologyRunInterface::get_topology_adjoint_job_source(int topology_id, int iteration_id)
{
  return ccx_core.get_topology_adjoint_job_source(topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::register_topology_adjoint_job(int topology_id, int iteration_id, int& job_id)
{
  return ccx_core.register_topology_adjoint_job(topology_id, iteration_id, job_id);
}

bool CalculiXTopologyRunInterface::build_topology_adjoint_load(int topology_id, int iteration_id, double& objective_value, int& node_id, std::array<double, 3>& load)
{
  return ccx_core.build_topology_adjoint_load(topology_id, iteration_id, objective_value, node_id, load);
}

bool CalculiXTopologyRunInterface::check_topology_displacement_selection(int topology_id, int iteration_id)
{
  return ccx_core.check_topology_displacement_selection(topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::check_topology_adjoint_results(int job_id)
{
  return ccx_core.check_topology_adjoint_results(job_id);
}

bool CalculiXTopologyRunInterface::read_topology_job_displacements(int job_id, std::map<int, std::array<double, 3>>& displacements)
{
  return ccx_core.read_topology_job_displacements(job_id, displacements);
}

bool CalculiXTopologyRunInterface::finish_topology_iteration(int topology_id, int iteration_id)
{
  return ccx_core.finish_topology_iteration(topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::evaluate_topology_compliance(int topology_id, int iteration_id, const std::map<int, double>& energies)
{
  return ccx_core.evaluate_topology_compliance(topology_id, iteration_id, energies);
}

bool CalculiXTopologyRunInterface::evaluate_topology_displacement(int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals)
{
  return ccx_core.evaluate_topology_displacement(topology_id, iteration_id, value, node_id, element_integrals);
}

bool CalculiXTopologyRunInterface::set_topology_iteration_displacement(int topology_id, int iteration_id, double displacement, int node_id)
{
  return ccx_core.set_topology_iteration_displacement(topology_id, iteration_id, displacement, node_id);
}

bool CalculiXTopologyRunInterface::mark_topology_iteration_completed(int topology_id, int iteration_id)
{
  return ccx_core.mark_topology_iteration_completed(topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::read_topology_element_energies(int topology_id, int iteration_id, std::map<int, double>& element_energies)
{
  return ccx_core.read_topology_element_energies(topology_id, iteration_id, element_energies);
}

bool CalculiXTopologyRunInterface::read_topology_job_element_tensors(int job_id, const std::string& key, std::map<int, std::map<int, std::array<double, 6>>>& tensors)
{
  return ccx_core.read_topology_job_element_tensors(job_id, key, tensors);
}

bool CalculiXTopologyRunInterface::read_topology_displacement(int topology_id, int iteration_id, double& value, int& node_id, std::array<double, 3>* critical_displacement)
{
  return ccx_core.read_topology_displacement(topology_id, iteration_id, value, node_id, critical_displacement);
}

bool CalculiXTopologyRunInterface::prepare_topology_density_iteration(int topology_id, int source_iteration, int next_iteration)
{
  return ccx_core.prepare_topology_density_iteration(topology_id, source_iteration, next_iteration);
}

bool CalculiXTopologyRunInterface::collect_topology_element_volumes(int topology_id, std::map<int, double>& element_volumes, std::map<int, std::array<double, 3>>* element_centers)
{
  return ccx_core.collect_topology_element_volumes(topology_id, element_volumes, element_centers);
}

bool CalculiXTopologyRunInterface::collect_topology_integration_weights(int topology_id, std::map<int, std::map<int, double>>& weights)
{
  return ccx_core.collect_topology_integration_weights(topology_id, weights);
}

bool CalculiXTopologyRunInterface::filter_topology_density_sensitivities(int topology_id, int iteration_id, const std::map<int, double>& element_volumes, const std::map<int, std::array<double, 3>>& element_centers)
{
  return ccx_core.filter_topology_density_sensitivities(topology_id, iteration_id, element_volumes, element_centers);
}

bool CalculiXTopologyRunInterface::build_topology_density_update(int topology_id, int source_iteration, std::map<int, double>& densities)
{
  return ccx_core.build_topology_density_update(topology_id, source_iteration, densities);
}

bool CalculiXTopologyRunInterface::evaluate_topology_compliance_oc_residual(int topology_id, int iteration_id)
{
  return ccx_core.evaluate_topology_compliance_oc_residual(topology_id, iteration_id);
}

bool CalculiXTopologyRunInterface::set_topology_iteration_densities(int topology_id, int iteration_id, const std::map<int, double>& densities, int source_iteration)
{
  return ccx_core.set_topology_iteration_densities(topology_id, iteration_id, densities, source_iteration);
}

bool CalculiXTopologyRunInterface::check_topology_density_convergence(int topology_id, int previous_iteration, int current_iteration, bool& converged)
{
  return ccx_core.check_topology_density_convergence(topology_id, previous_iteration, current_iteration, converged);
}
