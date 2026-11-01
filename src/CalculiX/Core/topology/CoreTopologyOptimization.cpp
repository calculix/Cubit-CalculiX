#include "Core/topology/CoreTopologyOptimization.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include <string>
#include <map>
#include <set>
#include <array>
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "Core/topology/CalculiXTopologyRunInterface.hpp"
#include "CubitMessage.hpp"

#include <QCoreApplication>
#include <QEventLoop>
#include <QScopedValueRollback>
#include <QThread>

#include <cmath>
#include <exception>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

bool CoreTopologyOptimization::run_topology(int topology_id, bool wait)
{
  CalculiXJobsInterface ccx_jobs_iface;
  CalculiXTopologyInterface ccx_topology_iface;
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  if (!ccx_topology_run_iface.begin_topology_run(topology_id))
  {
    return false;
  }

  TopologyRunContext context;
  context.serial = ++topology_run_serial;
  topology_runs.insert_or_assign(topology_id, context);

  if (!ccx_jobs_iface.check_jobs())
  {
    return false;
  }

  const auto started = topology_runs.find(topology_id);

  if (started == topology_runs.end())
  {
    return false;
  }

  if (started->second.phase == TopologyRunPhase::Finished)
  {
    const std::string state = ccx_topology_iface.get_topology_run_state(topology_id);
    return state == "converged" || state == "iteration_limit";
  }

  return wait ? wait_topology(topology_id) : true;
}

void CoreTopologyOptimization::update()
{
  if (updating)
  {
    return;
  }

  const QScopedValueRollback<bool> guard(updating, true);
  std::vector<std::pair<int, unsigned long long>> active;

  for (const auto& entry : topology_runs)
  {
    if (entry.second.phase != TopologyRunPhase::Finished)
    {
      active.emplace_back(entry.first, entry.second.serial);
    }
  }

  for (const auto& entry : active)
  {
    try
    {
      advance_topology_run(entry.first, entry.second);
    }
    catch (const std::exception& error)
    {
      PRINT_ERROR("Topology %d failed: %s\n", entry.first, error.what());
      fail_topology_run(entry.first, entry.second);
    }
    catch (...)
    {
      PRINT_ERROR("Topology %d failed with an unexpected exception.\n", entry.first);
      fail_topology_run(entry.first, entry.second);
    }
  }
}

void CoreTopologyOptimization::fail_topology_run(int topology_id, unsigned long long serial)
{
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  const auto run = topology_runs.find(topology_id);

  if (run == topology_runs.end() || run->second.serial != serial)
  {
    return;
  }

  run->second.stop_requested = true;
  run->second.stop_state = TopologyRunState::Failed;

  const int active_job = ccx_topology_run_iface.get_topology_active_iteration_job(topology_id, run->second.iteration_id);

  if (active_job > 0)
  {
    run->second.job_id = active_job;
  }
}

void CoreTopologyOptimization::finish_topology_run(int topology_id, unsigned long long serial, TopologyRunState state)
{
  CalculiXTopologyInterface ccx_topology_iface;
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  const auto run = topology_runs.find(topology_id);

  if (run == topology_runs.end() || run->second.serial != serial)
  {
    return;
  }

  run->second.phase = TopologyRunPhase::Finished;

  if (ccx_topology_run_iface.set_topology_run_state(topology_id, state))
  {
    PRINT_INFO("Topology %d: %s.\n", topology_id, ccx_topology_iface.get_topology_run_state(topology_id).c_str());
  }
}

bool CoreTopologyOptimization::get_topology_run_info(int topology_id, TopologyRunInfo& information)
{
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  if (!ccx_topology_run_iface.get_topology_base_run_info(topology_id, information))
  {
    return false;
  }

  const auto run = topology_runs.find(topology_id);

  if (run != topology_runs.end())
  {
    information.iteration_id = run->second.iteration_id;
    information.job_id = run->second.job_id;
    information.active = run->second.phase != TopologyRunPhase::Finished;
    information.stop_requested = run->second.stop_requested;
    information.waiting = waiting_for_topology;
  }

  return true;
}

void CoreTopologyOptimization::forget_topology_run(int topology_id)
{
  topology_runs.erase(topology_id);
}

int CoreTopologyOptimization::run_topology_iteration(int topology_id, int iteration_id)
{
  CalculiXJobsInterface ccx_jobs_iface;

  const auto run = topology_runs.find(topology_id);
  if (run != topology_runs.end() && run->second.phase != TopologyRunPhase::Finished)
  {
    PRINT_ERROR("Topology %d is already running.\n", topology_id);
    return -1;
  }
  const int job_id = start_topology_iteration(topology_id, iteration_id);
  if (job_id <= 0 || !ccx_jobs_iface.wait_job(job_id) ||
      !finish_topology_iteration(topology_id, iteration_id))
  {
    return -1;
  }
  return job_id;
}

void CoreTopologyOptimization::advance_topology_run(int topology_id, unsigned long long serial)
{
  CalculiXJobsInterface ccx_jobs_iface;
  CalculiXTopologyInterface ccx_topology_iface;
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  for (int transition = 0; transition < 4; ++transition)
  {
    auto run = topology_runs.find(topology_id);
    if (run == topology_runs.end() || run->second.serial != serial ||
        run->second.phase == TopologyRunPhase::Finished)
    {
      return;
    }
    const TopologyRunContext current = run->second;
    if (current.stop_requested)
    {
      if (current.job_id > 0)
      {
        int status = ccx_jobs_iface.get_job_status(current.job_id);
        if (status == 1 || status == 5)
        {
          const bool killed = ccx_jobs_iface.kill_job(current.job_id);
          status = ccx_jobs_iface.get_job_status(current.job_id);
          if (!killed || status == 1 || status == 5)
          {
            if (!current.kill_error_reported)
            {
              PRINT_ERROR("Topology %d: solver termination is still pending.\n", topology_id);
            }
            run = topology_runs.find(topology_id);
            if (run != topology_runs.end() && run->second.serial == serial)
            {
              run->second.kill_error_reported = true;
            }
            return;
          }
        }
      }
      finish_topology_run(topology_id, serial, current.stop_state);
      return;
    }
    if (current.phase == TopologyRunPhase::Preparing)
    {
      const bool prepared = current.iteration_id == 1
        ? prepare_topology_run(topology_id)
        : prepare_topology_density_iteration(topology_id, current.iteration_id - 1, current.iteration_id);
      if (!prepared)
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Failed);
        return;
      }
      run = topology_runs.find(topology_id);
      if (run == topology_runs.end() || run->second.serial != serial)
      {
        return;
      }
      run->second.phase = TopologyRunPhase::Starting;
    }
    else if (current.phase == TopologyRunPhase::Starting)
    {
      const int job_id = start_topology_iteration(topology_id, current.iteration_id);
      run = topology_runs.find(topology_id);
      if (run == topology_runs.end() || run->second.serial != serial)
      {
        return;
      }
      run->second.job_id = job_id;
      if (job_id <= 0)
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Failed);
        return;
      }
      run->second.phase = TopologyRunPhase::Waiting;
      if (!run->second.stop_requested)
      {
        return;
      }
    }
    else if (current.phase == TopologyRunPhase::Waiting)
    {
      const int status = ccx_jobs_iface.get_job_status(current.job_id);
      if (status == 1 || status == 5)
      {
        return;
      }
      if (status == 3)
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Killed);
        return;
      }
      if (status != 2)
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Failed);
        return;
      }
      TopologyOptions options;

      if (!ccx_topology_iface.get_topology_settings(topology_id, options))
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Failed);
        return;
      }

      run->second.phase = options.objective == TopologyObjective::Displacement ? TopologyRunPhase::StartingAdjoint : TopologyRunPhase::Evaluating;
    }
    else if (current.phase == TopologyRunPhase::StartingAdjoint)
    {
      int job_id = -1;
      bool skipped = false;

      const bool created = create_topology_adjoint_job(topology_id, current.iteration_id, job_id, skipped);

      run = topology_runs.find(topology_id);

      if (run == topology_runs.end() || run->second.serial != serial)
      {
        return;
      }

      if (!created)
      {
        run->second.stop_requested = true;
        run->second.stop_state = TopologyRunState::Failed;
        continue;
      }

      if (run->second.stop_requested)
      {
        continue;
      }

      if (skipped)
      {
        run->second.phase = TopologyRunPhase::Evaluating;
        continue;
      }

      if (job_id <= 0)
      {
        PRINT_ERROR("Adjoint job creation returned an invalid job ID.\n");
        run->second.stop_requested = true;
        run->second.stop_state = TopologyRunState::Failed;
        continue;
      }

      // Track the adjoint before export/start can process events or fail.
      run->second.job_id = job_id;
      run->second.phase = TopologyRunPhase::WaitingAdjoint;

      const bool started = ccx_jobs_iface.run_job(job_id, -1);

      run = topology_runs.find(topology_id);

      if (run == topology_runs.end() || run->second.serial != serial)
      {
        return;
      }

      if (!started)
      {
        PRINT_ERROR("Topology %d iteration %d: adjoint job %d could not start.\n", topology_id, current.iteration_id, job_id);
        run->second.stop_requested = true;
        run->second.stop_state = TopologyRunState::Failed;
        continue;
      }

      if (run->second.stop_requested)
      {
        continue;
      }

      PRINT_INFO("Topology %d iteration %d: adjoint job %d started.\n", topology_id, current.iteration_id, job_id);
      return;
    }
    else if (current.phase == TopologyRunPhase::WaitingAdjoint)
    {
      const int status = ccx_jobs_iface.get_job_status(current.job_id);

      if (status == 1 || status == 5)
      {
        return;
      }

      if (status == 3)
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Killed);
        return;
      }

      if (status != 2 || !ccx_topology_run_iface.check_topology_adjoint_results(current.job_id))
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Failed);
        return;
      }

      const int physical_job = ccx_topology_iface.get_topology_iteration_job_id(topology_id, current.iteration_id);

      if (physical_job <= 0)
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Failed);
        return;
      }

      PRINT_INFO("Topology %d iteration %d: adjoint job %d completed; results are loaded.\n", topology_id, current.iteration_id, current.job_id);

      run->second.job_id = physical_job;
      run->second.phase = TopologyRunPhase::Evaluating;
    }
    else if (current.phase == TopologyRunPhase::Evaluating)
    {
      if (!finish_topology_iteration(topology_id, current.iteration_id))
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Failed);
        return;
      }
      run = topology_runs.find(topology_id);
      if (run == topology_runs.end() || run->second.serial != serial)
      {
        return;
      }
      if (run->second.stop_requested)
      {
        continue;
      }
      bool converged = false;
      if (current.iteration_id > 1 && !ccx_topology_run_iface.check_topology_density_convergence(
            topology_id, current.iteration_id - 1, current.iteration_id, converged))
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Failed);
        return;
      }
      TopologyOptions options;

      if (!ccx_topology_iface.get_topology_settings(topology_id, options))
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Failed);
        return;
      }
      if (converged)
      {
        finish_topology_run(topology_id, serial, TopologyRunState::Converged);
        return;
      }
      if (current.iteration_id >= options.maximum_iterations)
      {
        finish_topology_run(topology_id, serial, TopologyRunState::IterationLimit);
        return;
      }
      run->second.iteration_id = current.iteration_id + 1;
      run->second.job_id = -1;
      run->second.phase = TopologyRunPhase::Preparing;
    }
  }
}

bool CoreTopologyOptimization::wait_topology(int topology_id)
{
  CalculiXJobsInterface ccx_jobs_iface;
  CalculiXTopologyInterface ccx_topology_iface;

  if (ccx_jobs_iface.is_checking_jobs() || waiting_for_topology)
  {
    PRINT_ERROR("A topology wait cannot be nested.\n");
    return false;
  }
  const auto initial = topology_runs.find(topology_id);
  if (initial == topology_runs.end())
  {
    PRINT_ERROR("Topology %d has no current run.\n", topology_id);
    return false;
  }
  const unsigned long long serial = initial->second.serial;
  const QScopedValueRollback<bool> guard(waiting_for_topology, true);
  while (true)
  {
    if (!ccx_jobs_iface.check_jobs())
    {
      PRINT_ERROR("Unable to poll topology jobs.\n");
      return false;
    }
    const auto run = topology_runs.find(topology_id);
    if (run == topology_runs.end() || run->second.serial != serial)
    {
      return false;
    }
    if (run->second.phase == TopologyRunPhase::Finished)
    {
      const std::string state = ccx_topology_iface.get_topology_run_state(topology_id);
      return state == "converged" || state == "iteration_limit";
    }
    if (QCoreApplication::instance() != nullptr)
    {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 25);
    }
    QThread::msleep(20);
  }
}

bool CoreTopologyOptimization::kill_topology(int topology_id)
{
  CalculiXJobsInterface ccx_jobs_iface;

  const auto run = topology_runs.find(topology_id);
  if (run == topology_runs.end() || run->second.phase == TopologyRunPhase::Finished)
  {
    PRINT_ERROR("Topology %d is not running.\n", topology_id);
    return false;
  }
  run->second.stop_requested = true;
  if (!ccx_jobs_iface.is_checking_jobs())
  {
    ccx_jobs_iface.check_jobs();
  }
  return true;
}

bool CoreTopologyOptimization::cancel_topology_runs()
{
  CalculiXJobsInterface ccx_jobs_iface;

  bool stopped = true;
  for (const auto& entry : topology_runs)
  {
    if (entry.second.phase != TopologyRunPhase::Finished && entry.second.job_id > 0)
    {
      const int status = ccx_jobs_iface.get_job_status(entry.second.job_id);
      if (status == 1 || status == 5)
      {
        if (!ccx_jobs_iface.kill_job(entry.second.job_id))
        {
          PRINT_ERROR("Unable to kill topology job %d during cleanup.\n", entry.second.job_id);
          stopped = false;
        }
      }
    }
  }
  if (!stopped)
  {
    return false;
  }
  topology_runs.clear();
  ++topology_run_serial;
  return true;
}

bool CoreTopologyOptimization::evaluate_topology_objective(int topology_id, int iteration_id)
{
  CalculiXTopologyInterface ccx_topology_iface;
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  if (iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology objective evaluation request.\n");
    return false;
  }

  TopologyOptions options;

  if (!ccx_topology_iface.get_topology_settings(topology_id, options))
  {
    return false;
  }

  switch (options.objective)
  {
    case TopologyObjective::Compliance:
    {
      std::map<int, double> energies;

      if (!ccx_topology_run_iface.read_topology_element_energies(topology_id, iteration_id, energies))
      {
        return false;
      }

      return ccx_topology_run_iface.evaluate_topology_compliance(topology_id, iteration_id, energies);
    }
    case TopologyObjective::Mass:
    {
      PRINT_ERROR("Mass optimization is not implemented.\n");
      return false;
    }
    case TopologyObjective::Displacement:
    {
      double objective_value = 0.0;
      int node_id = -1;
      std::array<double, 3> load{};

      if (!build_topology_adjoint_load(topology_id, iteration_id, objective_value, node_id, load))
      {
        return false;
      }

      std::map<int, std::map<int, double>> weights;

      if (!ccx_topology_run_iface.collect_topology_integration_weights(topology_id, weights))
      {
        return false;
      }

      std::map<int, double> integrals;

      // The established zero-subgradient convention needs no adjoint.
      if (objective_value == 0.0)
      {
        for (const auto& element : weights)
        {
          integrals.emplace(element.first, 0.0);
        }

        return ccx_topology_run_iface.evaluate_topology_displacement(topology_id, iteration_id, objective_value, node_id, integrals);
      }

      const int physical_job = ccx_topology_iface.get_topology_iteration_job_id(topology_id, iteration_id);
      const int adjoint_job = ccx_topology_iface.get_topology_iteration_adjoint_job_id(topology_id, iteration_id);

      if (physical_job <= 0 || adjoint_job <= 0 || physical_job == adjoint_job)
      {
        PRINT_ERROR("Invalid physical or adjoint job for displacement evaluation.\n");
        return false;
      }

      std::map<int, std::map<int, std::array<double, 6>>> stresses;
      std::map<int, std::map<int, std::array<double, 6>>> strains;

      if (!ccx_topology_run_iface.read_topology_job_element_tensors(physical_job, "S", stresses) ||
          !ccx_topology_run_iface.read_topology_job_element_tensors(adjoint_job, "ME", strains))
      {
        return false;
      }

      if (weights.empty() || stresses.size() != weights.size() || strains.size() != weights.size())
      {
        PRINT_ERROR("Incomplete element coverage for displacement integration.\n");
        return false;
      }

      for (const auto& element : weights)
      {
        const int element_id = element.first;
        const auto stress = stresses.find(element_id);
        const auto strain = strains.find(element_id);

        if (stress == stresses.end() ||
            strain == strains.end() ||
            element.second.empty() ||
            stress->second.size() != element.second.size() ||
            strain->second.size() != element.second.size())
        {
          PRINT_ERROR("Incomplete integration point coverage for element %d.\n", element_id);
          return false;
        }

        double integral = 0.0;

        for (const auto& point : element.second)
        {
          const auto stress_point = stress->second.find(point.first);
          const auto strain_point = strain->second.find(point.first);

          if (stress_point == stress->second.end() ||
              strain_point == strain->second.end() ||
              !std::isfinite(point.second) ||
              point.second <= 0.0)
          {
            PRINT_ERROR("Invalid integration point %d for element %d.\n", point.first, element_id);
            return false;
          }

          // DAT order: xx, yy, zz, xy, xz, yz.
          // ME contains tensor shear strains, requiring the factor of 2.
          double contraction = 0.0;

          for (std::size_t component = 0; component < 6; ++component)
          {
            const double stress_value = stress_point->second[component];
            const double strain_value = strain_point->second[component];

            if (!std::isfinite(stress_value) || !std::isfinite(strain_value))
            {
              PRINT_ERROR("Non-finite tensor at integration point %d of element %d.\n", point.first, element_id);
              return false;
            }

            const double multiplier = component < 3 ? 1.0 : 2.0;
            contraction += multiplier * stress_value * strain_value;
          }

          const double contribution = contraction * point.second;

          if (!std::isfinite(contribution))
          {
            PRINT_ERROR("Non-finite displacement integral contribution for element %d.\n", element_id);
            return false;
          }

          integral += contribution;
        }

        if (!std::isfinite(integral))
        {
          PRINT_ERROR("Non-finite displacement integral for element %d.\n", element_id);
          return false;
        }

        integrals.emplace(element_id, integral);
      }

      return ccx_topology_run_iface.evaluate_topology_displacement(topology_id, iteration_id, objective_value, node_id, integrals);
    }
    default:
    {
      PRINT_ERROR("Invalid topology objective.\n");
      return false;
    }
  }
}

bool CoreTopologyOptimization::prepare_topology_run(int topology_id)
{
  CalculiXTopologyInterface ccx_topology_iface;
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  TopologyOptions options;

  if (!ccx_topology_iface.get_topology_settings(topology_id, options))
  {
    return false;
  }

  if (options.method != TopologyMethod::Density ||
      (options.objective != TopologyObjective::Compliance &&
       options.objective != TopologyObjective::Displacement))
  {
    PRINT_ERROR("The topology iteration loop supports density optimization with compliance or displacement objectives.\n");
    return false;
  }

  if (options.objective == TopologyObjective::Displacement &&
      (options.displacement_nodeset <= 0 ||
       options.displacement_component < 0 ||
       options.displacement_component > 3))
  {
    PRINT_ERROR("Displacement optimization requires a displacement nodeset and valid component.\n");
    return false;
  }

  std::vector<int> section_ids;
  std::set<int> block_ids;

  if (!ccx_topology_run_iface.collect_topology_run_sections(topology_id, section_ids, block_ids))
  {
    return false;
  }

  if (!ccx_topology_run_iface.prepare_topology_history_outputs(topology_id, block_ids))
  {
    return false;
  }

  return ccx_topology_run_iface.initialize_topology_run_design(topology_id, section_ids);
}

bool CoreTopologyOptimization::build_topology_adjoint_load(int topology_id, int iteration_id, double& objective_value, int& node_id, std::array<double, 3>& load)
{
  CalculiXTopologyInterface ccx_topology_iface;
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  objective_value = 0.0;
  node_id = -1;
  load.fill(0.0);

  if (iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology adjoint load request.\n");
    return false;
  }

  TopologyOptions options;

  if (!ccx_topology_iface.get_topology_settings(topology_id, options))
  {
    return false;
  }

  const int component = options.displacement_component;

  if (options.displacement_nodeset <= 0 || component < 0 || component > 3)
  {
    PRINT_ERROR("Adjoint load calculation requires a displacement nodeset and valid component.\n");
    return false;
  }

  if (!ccx_topology_run_iface.check_topology_displacement_selection(topology_id, iteration_id))
  {
    return false;
  }

  std::array<double, 3> displacement{};

  if (!ccx_topology_run_iface.read_topology_displacement(topology_id, iteration_id, objective_value, node_id, &displacement))
  {
    return false;
  }

  // At zero displacement, use the zero subgradient.
  if (objective_value == 0.0)
  {
    PRINT_INFO("Topology %d iteration %d: monitored displacement is zero; no adjoint load is required.\n", topology_id, iteration_id);
    return true;
  }

  if (component == 0)
  {
    for (std::size_t axis = 0; axis < load.size(); ++axis)
    {
      load[axis] = displacement[axis] / objective_value;
    }
  }
  else
  {
    const std::size_t axis = static_cast<std::size_t>(component - 1);
    load[axis] = displacement[axis] > 0.0 ? 1.0 : -1.0;
  }

  return true;
}

bool CoreTopologyOptimization::create_topology_adjoint_job(int topology_id, int iteration_id, int& job_id, bool& skipped)
{
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  job_id = -1;
  skipped = false;

  if (ccx_topology_run_iface.get_topology_adjoint_job_source(topology_id, iteration_id) <= 0)
  {
    return false;
  }

  double objective_value = 0.0;
  int node_id = -1;
  std::array<double, 3> load{};

  if (!build_topology_adjoint_load(topology_id, iteration_id, objective_value, node_id, load))
  {
    return false;
  }

  if (objective_value == 0.0)
  {
    skipped = true;
    return true;
  }

  return ccx_topology_run_iface.register_topology_adjoint_job(topology_id, iteration_id, job_id);
}

int CoreTopologyOptimization::start_topology_iteration(int topology_id, int iteration_id)
{
  CalculiXJobsInterface ccx_jobs_iface;
  CalculiXTopologyInterface ccx_topology_iface;
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  if (iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology iteration request.\n");
    return -1;
  }

  TopologyOptions options;

  if (!ccx_topology_iface.get_topology_settings(topology_id, options))
  {
    return -1;
  }

  std::string filename_name = options.name;

  for (char& character : filename_name)
  {
    const bool allowed =
        (character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z') ||
        (character >= '0' && character <= '9') ||
        character == '_';

    if (!allowed)
    {
      character = '_';
    }
  }

  std::ostringstream name;
  name << "topo_" << topology_id << "_" << filename_name << "_iter_" << std::setfill('0') << std::setw(4) << iteration_id;

  const int job_id = ccx_topology_run_iface.create_topology_iteration_job(topology_id, iteration_id, name.str());

  if (job_id <= 0)
  {
    return -1;
  }

  if (!ccx_jobs_iface.run_job(job_id, -1))
  {
    PRINT_ERROR("Topology %d iteration %d could not start. Job %d is retained.\n", topology_id, iteration_id, job_id);
    return -1;
  }

  return job_id;
}

bool CoreTopologyOptimization::prepare_topology_density_iteration(int topology_id, int source_iteration, int next_iteration)
{
  CalculiXJobsInterface ccx_jobs_iface;
  CalculiXTopologyInterface ccx_topology_iface;
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  if (source_iteration < 1 || next_iteration <= source_iteration)
  {
    PRINT_ERROR("Invalid next topology iteration request.\n");
    return false;
  }

  const char* stage = "checking the source job";

  try
  {
    const int job_id = ccx_topology_iface.get_topology_iteration_job_id(topology_id, source_iteration);

    if (job_id <= 0 || ccx_jobs_iface.get_job_status(job_id) != 2)
    {
      PRINT_ERROR("The source topology job has not completed.\n");
      return false;
    }

    stage = "calculating the density update";
    PRINT_INFO("Topology %d iteration %d: %s.\n", topology_id, next_iteration, stage);

    std::map<int, double> densities;

    if (!ccx_topology_run_iface.build_topology_density_update(topology_id, source_iteration, densities))
    {
      return false;
    }

    stage = "storing the next density design";
    PRINT_INFO("Topology %d iteration %d: %s.\n", topology_id, next_iteration, stage);

    return ccx_topology_run_iface.set_topology_iteration_densities(topology_id, next_iteration, densities, source_iteration);
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Topology %d iteration %d failed while %s: %s.\n", topology_id, next_iteration, stage, error.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Topology %d iteration %d failed while %s.\n", topology_id, next_iteration, stage);
    return false;
  }
}

bool CoreTopologyOptimization::finish_topology_iteration(int topology_id, int iteration_id)
{
  CalculiXJobsInterface ccx_jobs_iface;
  CalculiXTopologyInterface ccx_topology_iface;
  CalculiXTopologyRunInterface ccx_topology_run_iface;

  const int job_id = ccx_topology_iface.get_topology_iteration_job_id(topology_id, iteration_id);

  if (job_id <= 0 || ccx_jobs_iface.get_job_status(job_id) != 2)
  {
    PRINT_ERROR("Topology iteration results are not ready.\n");
    return false;
  }

  if (!evaluate_topology_objective(topology_id, iteration_id))
  {
    return false;
  }

  TopologyOptions options;

  if (!ccx_topology_iface.get_topology_settings(topology_id, options))
  {
    return false;
  }

  if (options.displacement_nodeset > 0)
  {
    double displacement = 0.0;
    int node_id = -1;

    if (!ccx_topology_run_iface.read_topology_displacement(topology_id, iteration_id, displacement, node_id))
    {
      return false;
    }

    if (!ccx_topology_run_iface.set_topology_iteration_displacement(topology_id, iteration_id, displacement, node_id))
    {
      return false;
    }

    PRINT_INFO("Topology %d iteration %d: maximum monitored displacement %.9g at node %d.\n", topology_id, iteration_id, displacement, node_id);
  }

  std::map<int, double> element_volumes;
  std::map<int, std::array<double, 3>> element_centers;

  PRINT_INFO("Topology %d iteration %d: preparing filtered sensitivities.\n", topology_id, iteration_id);

  if (!ccx_topology_run_iface.collect_topology_element_volumes(topology_id, element_volumes, &element_centers))
  {
    return false;
  }

  if (!ccx_topology_run_iface.filter_topology_density_sensitivities(topology_id, iteration_id, element_volumes, element_centers))
  {
    return false;
  }

  if (options.objective == TopologyObjective::Compliance)
  {
    if (!ccx_topology_run_iface.evaluate_topology_compliance_oc_residual(topology_id, iteration_id))
    {
      return false;
    }
  }
  else if (options.objective == TopologyObjective::Displacement)
  {
    if (!ccx_topology_run_iface.evaluate_topology_displacement_projected_residual(topology_id, iteration_id))
    {
      return false;
    }
  }

  if (!ccx_topology_run_iface.mark_topology_iteration_completed(topology_id, iteration_id))
  {
    return false;
  }

  PRINT_INFO("Topology %d iteration %d completed as job %d; results are loaded.\n", topology_id, iteration_id, job_id);
  return true;
}
