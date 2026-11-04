#ifndef CORETOPOLOGYOPTIMIZATION_HPP
#define CORETOPOLOGYOPTIMIZATION_HPP

#include "Core/topology/CoreTopologyStructs.hpp"
#include <array>
#include <map>

/** @brief Own transient run contexts and coordinate topology optimization through domain interfaces. */
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
  /**
   * @brief Start, wait for and evaluate one physical iteration.
   * @param topology_id Prepared case without an active controller run.
   * @param iteration_id Positive iteration with a stored design and applied factors.
   * @return Physical job ID on success; -1 if startup, waiting or evaluation fails.
   * @note Does not perform the controller's full physical/adjoint sequence. A
   * nonzero displacement evaluation needs its required adjoint data already present.
   * Created jobs and successful earlier evaluation stages are not rolled back.
   */
  int run_topology_iteration(int topology_id, int iteration_id);
  /**
   * @brief Create and start the physical job without waiting for completion.
   * @param topology_id Existing prepared case; its name supplies the job-name prefix.
   * @param iteration_id Positive iteration with stored design/material data.
   * @return Started job ID, or -1 on failure.
   * @note If job creation succeeds but startup fails, the job is retained. This
   * method does not evaluate results or advance a complete optimization loop.
   */
  int start_topology_iteration(int topology_id, int iteration_id);
  /**
   * @brief Register an adjoint job unless the monitored objective is zero.
   * @param topology_id Prepared displacement case.
   * @param iteration_id Physical iteration with completed required results.
   * @param[out] job_id Registered adjoint job ID; initialized to -1.
   * @param[out] skipped True when a zero objective requires no adjoint solve.
   * @return True for successful registration or a valid skip; false on failure.
   * @note This method does not start the registered job. Callers must inspect
   * skipped before using job_id; success on a skip leaves job_id at -1.
   */
  bool create_topology_adjoint_job(int topology_id, int iteration_id, int& job_id, bool& skipped);
  /**
   * @brief Build the global adjoint force at the current controlling node.
   * @param topology_id Prepared displacement case.
   * @param iteration_id Physical iteration with the required final DAT frame.
   * @param objective_value Output maximum absolute component or vector magnitude.
   * @param node_id Output controlling node ID.
   * @param load Output global X/Y/Z force: component sign or normalized displacement.
   * @return True when selection and calculation succeed, including zero objective.
   * @note Outputs start at zero, -1, and zero vector respectively. At zero objective,
   * the chosen subgradient is zero and the caller can skip the adjoint solve.
   * Equal maxima select the lowest node ID. A missing final-frame quantity is
   * an error; this does not fall back to an earlier output frame.
   */
  bool build_topology_adjoint_load(int topology_id, int iteration_id, double& objective_value, int& node_id, std::array<double, 3>& load);
  /**
   * @brief Prepare outputs, the analysis domain and the initial solid design.
   * @param topology_id Existing case accepted for a density run.
   * @return True after all preparation stages succeed; false otherwise.
   * @note Supports compliance and displacement. Uses existing output definitions
   * or configures required ones through silent commands, then initializes run data.
   * Failure does not restore output definitions changed by an earlier stage.
   */
  bool prepare_topology_run(int topology_id);
  /**
   * @brief Evaluate loaded results and mark an iteration completed.
   * @param topology_id Prepared case with physical and required adjoint results.
   * @param iteration_id Iteration to evaluate exactly once.
   * @param[out] proposal Optional transient level-set proposal for the active controller; empty for density.
   * @return True after all stages succeed; false when any required stage fails.
   * @note Order: raw objective/derivatives, optional displacement monitor, Cubit
   * geometry, density sensitivity filter or level-set QP proposal,
   * completion marker.
   * Convergence is checked by the controller afterwards. Failure does not undo
   * successful earlier stages of this sequence.
   */
  bool finish_topology_iteration(int topology_id, int iteration_id, TopologyLevelSetProposal* proposal = nullptr);
  /**
   * @brief Calculate and store the next density design without starting its job.
   * @param topology_id Prepared density case.
   * @param source_iteration Evaluated design with a finished physical job, filtered
   * sensitivities and volume data.
   * @param next_iteration Unused positive iteration ID greater than source_iteration.
   * @return True when the next design, applied factors and diagnostics are stored.
   * @note Uses the selected effective move. Storage performs its own explicit
   * cleanup checks; this method does not promise a general transaction.
   */
  bool prepare_topology_density_iteration(int topology_id, int source_iteration, int next_iteration);
  /**
   * @brief Combine stored case state with an available transient controller context.
   * @param topology_id Existing topology case.
   * @param[out] information Run snapshot; no ownership of controller state is transferred.
   * @return True for an existing case, including cases without a controller context.
   * @note Historical results do not imply active=true. waiting reflects the
   * controller-wide blocking-wait flag when a context is present.
   */
  bool get_topology_run_info(int topology_id, TopologyRunInfo& information);

  /**
   * @brief Poll controller contexts and advance eligible run phases.
   * @note Rejects reentrant advancement internally. Job/result operations remain
   * on the calling thread; this is not a separate result-reader worker.
   */
  void update();
  /**
   * @brief Terminate active topology jobs and discard transient contexts on success.
   * @return True when required kill requests succeed and contexts are cleared.
   * False retains contexts if a required job termination fails.
   * @note Completed iteration records remain in the domain cores. This is the
   * cleanup operation, distinct from merely forgetting one context.
   */
  bool cancel_topology_runs();
  /**
   * @brief Remove only the transient controller context for a case.
   * @param topology_id Case context to erase; an absent context is harmless.
   * @pre The caller has ensured that no active solver job needs this context.
   * @note Does not kill a process or delete stored iteration jobs/results.
   */
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

  static const char* phase_name(TopologyRunPhase phase);

  struct TopologyRunContext
  {
    unsigned long long serial = 0;
    TopologyRunPhase phase = TopologyRunPhase::Preparing;
    int iteration_id = 1;
    int job_id = -1;
    TopologyLevelSetProposal pending_level_set_proposal;
    bool stop_requested = false;
    bool kill_error_reported = false;
    TopologyRunState stop_state = TopologyRunState::Killed;
  };

  std::map<int, TopologyRunContext> topology_runs;
  unsigned long long topology_run_serial = 0;
  bool waiting_for_topology = false;
  bool updating = false;

  bool prepare_topology_iteration(int topology_id, int source_iteration, int next_iteration);
  bool check_topology_convergence(int topology_id, int previous_iteration, int current_iteration, bool& converged);
  void advance_topology_run(int topology_id, unsigned long long serial);
  bool evaluate_topology_objective(int topology_id, int iteration_id);
  void finish_topology_run(int topology_id, unsigned long long serial, TopologyRunState state);
  void fail_topology_run(int topology_id, unsigned long long serial);
};

#endif // CORETOPOLOGYOPTIMIZATION_HPP
