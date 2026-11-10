#include "CalculiXTopologyInterface.hpp"
#include "Core/topology/CoreTopologyJobs.hpp"
#include "Core/topology/CoreTopologyOptimization.hpp"
#include "Core/topology/CoreTopologyPreservation.hpp"
#include "Core/topology/CoreTopologyModelPreparation.hpp"
#include "Core/topology/CoreTopologyIteration.hpp"
#include "Core/topology/CoreTopologyQuery.hpp"
#include <string>
#include <vector>
#include <set>

#include "Core/topology/CoreTopology.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "CubitMessage.hpp"
#include "Core/loadCalculiXCore.hpp"

bool CalculiXTopologyInterface::run_topology(int topology_id, bool wait)
{
  if (!ccx_core.topologyoptimization)
  {
    return false;
  }

  return ccx_core.topologyoptimization->run_topology(topology_id, wait);
}

bool CalculiXTopologyInterface::wait_topology(int topology_id)
{
  if (!ccx_core.topologyoptimization)
  {
    return false;
  }

  return ccx_core.topologyoptimization->wait_topology(topology_id);
}

bool CalculiXTopologyInterface::kill_topology(int topology_id)
{
  if (!ccx_core.topologyoptimization)
  {
    return false;
  }

  return ccx_core.topologyoptimization->kill_topology(topology_id);
}

bool CalculiXTopologyInterface::get_topology_run_info(int topology_id, TopologyRunInfo& information)
{
  information = {};

  if (!ccx_core.topologyoptimization)
  {
    return false;
  }

  return ccx_core.topologyoptimization->get_topology_run_info(topology_id, information);
}

int CalculiXTopologyInterface::run_topology_iteration(int topology_id, int iteration_id)
{
  if (!ccx_core.topologyoptimization)
  {
    PRINT_ERROR("Topology optimization controller is not initialized.\n");
    return -1;
  }

  return ccx_core.topologyoptimization->run_topology_iteration(topology_id, iteration_id);
}

bool CalculiXTopologyInterface::create_topology(int topology_id, const TopologyOptions& options)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->create_topology(topology_id, options);
}

bool CalculiXTopologyInterface::modify_topology(int topology_id, const TopologyOptions& options)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->modify_topology(topology_id, options);
}

bool CalculiXTopologyInterface::delete_topology(int topology_id)
{
  if (!ccx_core.topology || !ccx_core.jobs)
  {
    PRINT_ERROR("Topology or job core is not initialized.\n");
    return false;
  }

  return CoreTopologyJobs::delete_topology(*ccx_core.topology, *ccx_core.jobs, ccx_core.mat, ccx_core.sections, ccx_core.topologyoptimization, topology_id);
}

bool CalculiXTopologyInterface::get_topology_preserved_sets(
  int topology_id,
  std::vector<TopologyPreservedSetInfo>& sets,
  std::set<int>& direct_elements)
{
  return ccx_core.get_topology_preserved_sets(
    topology_id, sets, direct_elements);
}

bool CalculiXTopologyInterface::reset_preserved_sets(int topology_id)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->reset_preserved_sets(topology_id);
}

bool CalculiXTopologyInterface::add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->add_preserved_sets(topology_id, set_type, ids);
}

bool CalculiXTopologyInterface::remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->remove_preserved_sets(topology_id, set_type, ids);
}

bool CalculiXTopologyInterface::get_topology_set_element_ids(int topology_id, int set_type, int set_id, std::vector<int>& element_ids)
{
  element_ids.clear();

  if (!ccx_core.mat || !ccx_core.me_iface)
  {
    PRINT_ERROR("Topology mesh/material interfaces are not initialized.\n");
    return false;
  }

  if (!ccx_core.cb)
  {
    PRINT_ERROR("Block core is not initialized.\n");
    return false;
  }

  return CoreTopologyPreservation::get_set_element_ids(*ccx_core.mat, *ccx_core.cb, *ccx_core.me_iface, topology_id, set_type, set_id, element_ids);
}

bool CalculiXTopologyInterface::initialize_topology(int topology_id, const std::vector<int>& section_ids)
{
  if (!ccx_core.mat || !ccx_core.sections || !ccx_core.cb || !ccx_core.me_iface)
  {
    PRINT_ERROR("Topology initialization interfaces are not initialized.\n");
    return false;
  }

  return CoreTopologyModelPreparation::initialize(*ccx_core.mat, *ccx_core.sections, *ccx_core.cb, *ccx_core.me_iface, topology_id, section_ids);
}

std::vector<std::vector<int>> CalculiXTopologyInterface::get_topology_element_preservation(int topology_id)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return {};
  }

  return ccx_core.topology->get_topology_element_preservation(topology_id);
}

bool CalculiXTopologyInterface::set_topology_iteration_factors(int topology_id, int iteration_id, const std::vector<int>& element_ids, const std::vector<double>& stiffness_factors)
{
  if (!ccx_core.topology || !ccx_core.mat)
  {
    PRINT_ERROR("Topology/material cores are not initialized.\n");
    return false;
  }

  return CoreTopologyIteration::set_factors(*ccx_core.topology, topology_id, iteration_id, element_ids, stiffness_factors);
}

bool CalculiXTopologyInterface::check_topology_material_assignment(int topology_id, int iteration_id, int element_id)
{
  if (!ccx_core.mat)
  {
    PRINT_ERROR("Topology material core is not initialized.\n");
    return false;
  }

  return ccx_core.mat->check_topology_material_assignment(topology_id, iteration_id, element_id);
}

std::string CalculiXTopologyInterface::get_topology_material_name(int topology_id, int element_id)
{
  if (!ccx_core.mat)
  {
    PRINT_ERROR("Topology material core is not initialized.\n");
    return "";
  }

  return ccx_core.mat->get_topology_material_name(topology_id, element_id);
}

bool CalculiXTopologyInterface::get_topology_group_material_name(int topology_id, int iteration_id, int element_id, std::string& material_name)
{
  if (!ccx_core.mat)
  {
    PRINT_ERROR("Topology material core is not initialized.\n");
    return false;
  }

  return ccx_core.mat->get_topology_group_material_name(topology_id, iteration_id, element_id, material_name);
}

std::vector<double> CalculiXTopologyInterface::get_topology_parameters(int topology_id)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return {};
  }

  return ccx_core.topology->get_topology_parameters(topology_id);
}

bool CalculiXTopologyInterface::get_topology_settings(int topology_id, TopologyOptions& options)
{
  if (!ccx_core.topology)
  {
    options = {};
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return ccx_core.topology->get_topology_settings(topology_id, options);
}

int CalculiXTopologyInterface::get_topology_iteration_job_id(int topology_id, int iteration_id)
{
  if (!ccx_core.topology || !ccx_core.jobs)
  {
    PRINT_ERROR("Invalid topology iteration query.\n");
    return -1;
  }

  return CoreTopologyQuery::get_iteration_job_id(*ccx_core.topology, *ccx_core.jobs, topology_id, iteration_id);
}

int CalculiXTopologyInterface::get_topology_iteration_adjoint_job_id(int topology_id, int iteration_id)
{
  if (!ccx_core.topology || !ccx_core.jobs)
  {
    PRINT_ERROR("Invalid topology adjoint job query.\n");
    return -1;
  }

  return CoreTopologyQuery::get_iteration_adjoint_job_id(*ccx_core.topology, *ccx_core.jobs, topology_id, iteration_id);
}

std::vector<std::vector<std::string>> CalculiXTopologyInterface::get_topology_iteration_data(int topology_id)
{
  if (!ccx_core.topology || !ccx_core.jobs)
  {
    PRINT_ERROR("Topology/job cores are not initialized.\n");
    return {};
  }

  return CoreTopologyQuery::get_iteration_data(*ccx_core.topology, *ccx_core.jobs, topology_id);
}

bool CalculiXTopologyInterface::select_topology_iteration(int topology_id, int iteration_id)
{
  if (!ccx_core.topology || !ccx_core.mat || !ccx_core.jobs)
  {
    PRINT_ERROR("Invalid topology iteration selection request.\n");
    return false;
  }

  return CoreTopologyIteration::select_iteration(*ccx_core.topology, *ccx_core.mat, *ccx_core.jobs, topology_id, iteration_id);
}

std::string CalculiXTopologyInterface::get_topology_run_state(int topology_id)
{
  if (!ccx_core.topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return "";
  }

  return ccx_core.topology->get_topology_run_state(topology_id);
}

bool CalculiXTopologyInterface::get_topology_iteration_level_set_settings(int topology_id, int iteration_id, TopologyLevelSetSettings& settings)
{
  settings = {};
  if (!ccx_core.topology)
  {
    return false;
  }
  return CoreTopologyQuery::get_iteration_level_set_settings(*ccx_core.topology, topology_id, iteration_id, settings);
}

bool CalculiXTopologyInterface::get_topology_iteration_method(int topology_id, int iteration_id, TopologyMethod& method)
{
  method = TopologyMethod::Unspecified;
  if (!ccx_core.topology)
  {
    return false;
  }
  return CoreTopologyQuery::get_iteration_method(*ccx_core.topology, topology_id, iteration_id, method);
}

std::vector<std::vector<double>> CalculiXTopologyInterface::get_topology_iteration_element_data(int topology_id, int iteration_id)
{
  if (!ccx_core.topology || !ccx_core.mat || !ccx_core.jobs)
  {
    PRINT_ERROR("Invalid topology element-data request.\n");
    return {};
  }

  return CoreTopologyQuery::get_iteration_element_data(*ccx_core.topology, *ccx_core.mat, *ccx_core.jobs, topology_id, iteration_id);
}

std::vector<std::vector<double>> CalculiXTopologyInterface::get_topology_iteration_sensitivity_data(int topology_id, int iteration_id)
{
  return ccx_core.get_topology_iteration_sensitivity_data(topology_id, iteration_id);
}

bool CalculiXTopologyInterface::is_topology_adjoint_export() const
{
  return ccx_core.job_export_context.adjoint;
}

bool CalculiXTopologyInterface::check_topology_export(int topology_id, int iteration_id)
{
  return ccx_core.check_topology_export(topology_id, iteration_id);
}

std::vector<std::vector<std::string>> CalculiXTopologyInterface::get_topology_tree_data()
{
  if (!ccx_core.topology)
  {
    return {};
  }

  return ccx_core.topology->get_tree_data();
}

bool CalculiXTopologyInterface::is_topology_level_set_run_enabled()
{
  return ccx_core.is_topology_level_set_run_enabled();
}

std::vector<std::vector<double>> CalculiXTopologyInterface::get_topology_iteration_level_set_field_data(int topology_id, int iteration_id)
{
  return ccx_core.get_topology_iteration_level_set_field_data(topology_id, iteration_id);
}
