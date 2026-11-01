#ifndef CALCULIXTOPOLOGYRUNINTERFACE_HPP
#define CALCULIXTOPOLOGYRUNINTERFACE_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <array>

struct TopologyRunInfo;
enum class TopologyRunState;

class CalculiXTopologyRunInterface
{
public:
  bool begin_topology_run(int topology_id);
  bool set_topology_run_state(int topology_id, TopologyRunState state);
  bool get_topology_base_run_info(int topology_id, TopologyRunInfo& information);
  int get_topology_active_iteration_job(int topology_id, int iteration_id);
  bool prepare_topology_run(int topology_id);
  bool collect_topology_run_sections(int topology_id, std::vector<int>& section_ids, std::set<int>& block_ids);
  bool prepare_topology_history_outputs(int topology_id, const std::set<int>& block_ids);
  bool initialize_topology_run_design(int topology_id, const std::vector<int>& section_ids);
  int start_topology_iteration(int topology_id, int iteration_id);
  int create_topology_iteration_job(int topology_id, int iteration_id, const std::string& job_name);
  bool create_topology_adjoint_job(int topology_id, int iteration_id, int& job_id, bool& skipped);
  int get_topology_adjoint_job_source(int topology_id, int iteration_id);
  bool register_topology_adjoint_job(int topology_id, int iteration_id, int& job_id);
  bool build_topology_adjoint_load(int topology_id, int iteration_id, double& objective_value, int& node_id, std::array<double, 3>& load);
  bool check_topology_displacement_selection(int topology_id, int iteration_id);
  bool check_topology_adjoint_results(int job_id);
  bool read_topology_job_displacements(int job_id, std::map<int, std::array<double, 3>>& displacements);
  bool finish_topology_iteration(int topology_id, int iteration_id);
  bool evaluate_topology_compliance(int topology_id, int iteration_id, const std::map<int, double>& energies);
  bool evaluate_topology_displacement(int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals);
  bool set_topology_iteration_displacement(int topology_id, int iteration_id, double displacement, int node_id);
  bool mark_topology_iteration_completed(int topology_id, int iteration_id);
  bool read_topology_element_energies(int topology_id, int iteration_id, std::map<int, double>& element_energies);
  bool read_topology_job_element_tensors(int job_id, const std::string& key, std::map<int, std::map<int, std::array<double, 6>>>& tensors);
  bool read_topology_displacement(int topology_id, int iteration_id, double& value, int& node_id, std::array<double, 3>* critical_displacement = nullptr);
  bool prepare_topology_density_iteration(int topology_id, int source_iteration, int next_iteration);
  bool collect_topology_element_volumes(int topology_id, std::map<int, double>& element_volumes, std::map<int, std::array<double, 3>>* element_centers = nullptr);
  bool collect_topology_integration_weights(int topology_id, std::map<int, std::map<int, double>>& weights);
  bool filter_topology_density_sensitivities(int topology_id, int iteration_id, const std::map<int, double>& element_volumes, const std::map<int, std::array<double, 3>>& element_centers);
  bool build_topology_density_update(int topology_id, int source_iteration, std::map<int, double>& densities);
  bool evaluate_topology_compliance_oc_residual(int topology_id, int iteration_id);
  bool evaluate_topology_displacement_projected_residual(int topology_id, int iteration_id);
  bool set_topology_iteration_densities(int topology_id, int iteration_id, const std::map<int, double>& densities, int source_iteration = -1);
  bool check_topology_density_convergence(int topology_id, int previous_iteration, int current_iteration, bool& converged);
};

#endif // CALCULIXTOPOLOGYRUNINTERFACE_HPP
