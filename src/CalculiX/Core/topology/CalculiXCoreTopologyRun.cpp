#include "Core/CalculiXCore.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyOptimization.hpp"
#include "CoreTopologyModelValidation.hpp"
#include <QScopedValueRollback>
#include "CubitMessage.hpp"

int CalculiXCore::start_topology_iteration(int topology_id, int iteration_id)
{
  if (!topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return -1;
  }

  return topologyoptimization->start_topology_iteration(topology_id, iteration_id);
}

bool CalculiXCore::create_topology_adjoint_job(int topology_id, int iteration_id, int& job_id, bool& skipped)
{
  job_id = -1;
  skipped = false;

  if (!topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return false;
  }

  return topologyoptimization->create_topology_adjoint_job(topology_id, iteration_id, job_id, skipped);
}

int CalculiXCore::run_topology_iteration(int topology_id, int iteration_id)
{
  if (!topologyoptimization)
  {
    PRINT_ERROR("Topology optimization controller is not initialized.\n");
    return -1;
  }

  return topologyoptimization->run_topology_iteration(topology_id, iteration_id);
}

bool CalculiXCore::finish_topology_iteration(int topology_id, int iteration_id)
{
  return topologyoptimization && topologyoptimization->finish_topology_iteration(topology_id, iteration_id);
}

bool CalculiXCore::run_topology(int topology_id, bool wait)
{
  return topologyoptimization && topologyoptimization->run_topology(topology_id, wait);
}

bool CalculiXCore::wait_topology(int topology_id)
{
  return topologyoptimization && topologyoptimization->wait_topology(topology_id);
}

bool CalculiXCore::kill_topology(int topology_id)
{
  return topologyoptimization && topologyoptimization->kill_topology(topology_id);
}

bool CalculiXCore::get_topology_run_info(int topology_id, TopologyRunInfo& information)
{
  information = {};
  return topologyoptimization && topologyoptimization->get_topology_run_info(topology_id, information);
}

bool CalculiXCore::cancel_topology_runs()
{
  return !topologyoptimization || topologyoptimization->cancel_topology_runs();
}

bool CalculiXCore::prepare_topology_run(int topology_id)
{
  if (!topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return false;
  }

  return topologyoptimization->prepare_topology_run(topology_id);
}

bool CalculiXCore::build_topology_adjoint_load(int topology_id, int iteration_id, double& objective_value, int& node_id, std::array<double, 3>& load)
{
  objective_value = 0.0;
  node_id = -1;
  load.fill(0.0);

  if (!topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return false;
  }

  return topologyoptimization->build_topology_adjoint_load(topology_id, iteration_id, objective_value, node_id, load);
}

bool CalculiXCore::set_topology_run_state(int topology_id, TopologyRunState state)
{
  if (!topology)
  {
    return false;
  }

  return topology->set_topology_run_state(topology_id, state);
}

bool CalculiXCore::begin_topology_run(int topology_id)
{
  if (topology_animation_active())
  {
    PRINT_ERROR("Finish or cancel the topology GIF export before starting topology.\n");
    return false;
  }

  if (!topology || !mat || !sections || !jobs || !steps || !historyoutputs || !me_iface)
  {
    PRINT_ERROR("Topology run cores are not initialized.\n");
    return false;
  }

  if (checking_jobs)
  {
    PRINT_ERROR("Job results are being processed. Try again shortly.\n");
    return false;
  }

  const auto validate_model = [this]()
  {
    return validate_topology_compliance_model();
  };

  if (!CoreTopologyModelValidation::validate_run(*topology, *me_iface, topology_id, validate_model))
  {
    return false;
  }

  const QScopedValueRollback<bool> guard(checking_jobs, true);

  if (!reset_topology_run(topology_id))
  {
    return false;
  }

  return topology->set_topology_run_state(topology_id, TopologyRunState::Running);
}
