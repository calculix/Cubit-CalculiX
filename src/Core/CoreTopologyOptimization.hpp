#ifndef CORETOPOLOGYOPTIMIZATION_HPP
#define CORETOPOLOGYOPTIMIZATION_HPP

#include "CalculiXCoreInterface.hpp"

#include <array>
#include <map>

class CoreTopologyOptimization
{
public:
  /**
   * @brief Start a fresh run using the topology case settings.
   * @param topology_id Existing topology case ID.
   * @param wait Whether to wait for the run to finish.
   * @return True when startup succeeds, or successful completion when waiting.
   * @note With wait=false, success does not mean optimization has completed.
   * @warning Starting again resets this case's previous iteration data.
   */
  bool run_topology(int topology_id, bool wait = false);

  /**
   * @brief Wait for the current topology run.
   * @param topology_id Topology case with a current run context.
   * @return True for convergence or iteration-limit completion; false otherwise.
   * @note Nested waits and waits without a current controller context are rejected.
   */
  bool wait_topology(int topology_id);

  /**
   * @brief Request termination while retaining completed iteration results.
   * @param topology_id Topology case with an active run.
   * @return True when the stop request is accepted; false otherwise.
   * @note Acceptance does not guarantee immediate solver termination.
   */
  bool kill_topology(int topology_id);
  int run_topology_iteration(int topology_id, int iteration_id);
  int start_topology_iteration(int topology_id, int iteration_id);
  bool create_topology_adjoint_job(int topology_id, int iteration_id, int& job_id, bool& skipped);
  /**
   * @brief Build the global adjoint force at the current controlling node.
   * @param topology_id Prepared displacement case.
   * @param iteration_id Physical iteration whose final available results are read.
   * @param objective_value Output maximum absolute component or vector magnitude.
   * @param node_id Output controlling node ID.
   * @param load Output global X/Y/Z force: component sign or normalized displacement.
   * @return True when selection and calculation succeed, including zero objective.
   * @note Outputs start at zero, -1, and zero vector respectively. At zero objective,
   * the chosen subgradient is zero and the caller can skip the adjoint solve.
   */
  bool build_topology_adjoint_load(int topology_id, int iteration_id, double& objective_value, int& node_id, std::array<double, 3>& load);
  bool prepare_topology_run(int topology_id);
  /**
   * @brief Evaluate loaded results and mark an iteration completed.
   * @param topology_id Prepared case with physical and required adjoint results.
   * @param iteration_id Iteration to evaluate exactly once.
   * @return True after all stages succeed; false when any required stage fails.
   * @note Order: raw objective/derivatives, optional displacement monitor, Cubit
   * geometry, sensitivity filter, compliance OC residual, completion marker.
   * Convergence is checked by the controller afterwards. Failure does not undo
   * successful earlier stages of this sequence.
   */
  bool finish_topology_iteration(int topology_id, int iteration_id);
  bool prepare_topology_density_iteration(int topology_id, int source_iteration, int next_iteration);
  bool get_topology_run_info(int topology_id, TopologyRunInfo& information);

  void update();
  bool cancel_topology_runs();
  void forget_topology_run(int topology_id);

private:
  enum class TopologyRunPhase
  {
    Preparing,
    Starting,
    Waiting,
    StartingAdjoint,
    WaitingAdjoint,
    Evaluating,
    Finished
  };

  struct TopologyRunContext
  {
    unsigned long long serial = 0;
    TopologyRunPhase phase = TopologyRunPhase::Preparing;
    int iteration_id = 1;
    int job_id = -1;
    bool stop_requested = false;
    bool kill_error_reported = false;
    TopologyRunState stop_state = TopologyRunState::Killed;
  };

  CalculiXCoreInterface ccx_iface;
  std::map<int, TopologyRunContext> topology_runs;
  unsigned long long topology_run_serial = 0;
  bool waiting_for_topology = false;
  bool updating = false;

  void advance_topology_run(int topology_id, unsigned long long serial);
  bool evaluate_topology_objective(int topology_id, int iteration_id);
  void finish_topology_run(int topology_id, unsigned long long serial, TopologyRunState state);
  void fail_topology_run(int topology_id, unsigned long long serial);
};

#endif // CORETOPOLOGYOPTIMIZATION_HPP
