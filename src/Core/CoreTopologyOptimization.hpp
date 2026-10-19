#ifndef CORETOPOLOGYOPTIMIZATION_HPP
#define CORETOPOLOGYOPTIMIZATION_HPP

#include "CalculiXCoreInterface.hpp"

#include <array>
#include <map>

class CoreTopologyOptimization
{
public:
  bool run_topology(int topology_id, bool wait = false);
  bool wait_topology(int topology_id);
  bool kill_topology(int topology_id);
  int run_topology_iteration(int topology_id, int iteration_id);
  int start_topology_iteration(int topology_id, int iteration_id);
  bool create_topology_adjoint_job(int topology_id, int iteration_id, int& job_id, bool& skipped);
  bool build_topology_adjoint_load(int topology_id, int iteration_id, double& objective_value, int& node_id, std::array<double, 3>& load);
  bool prepare_topology_run(int topology_id);
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
