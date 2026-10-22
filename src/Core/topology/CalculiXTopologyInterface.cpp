#include "CalculiXTopologyInterface.hpp"
#include <string>
#include <vector>
#include <set>

#include "Core/loadCalculiXCore.hpp"

bool CalculiXTopologyInterface::run_topology(int topology_id, bool wait)
{
  return ccx_core.run_topology(topology_id, wait);
}

bool CalculiXTopologyInterface::wait_topology(int topology_id)
{
  return ccx_core.wait_topology(topology_id);
}

bool CalculiXTopologyInterface::kill_topology(int topology_id)
{
  return ccx_core.kill_topology(topology_id);
}

bool CalculiXTopologyInterface::get_topology_run_info(int topology_id, TopologyRunInfo& information)
{
  return ccx_core.get_topology_run_info(topology_id, information);
}

int CalculiXTopologyInterface::run_topology_iteration(int topology_id, int iteration_id)
{
  return ccx_core.run_topology_iteration(topology_id, iteration_id);
}

bool CalculiXTopologyInterface::create_topology(int topology_id, const TopologyOptions& options)
{
  return ccx_core.create_topology(topology_id, options);
}

bool CalculiXTopologyInterface::modify_topology(int topology_id, const TopologyOptions& options)
{
  return ccx_core.modify_topology(topology_id, options);
}

bool CalculiXTopologyInterface::delete_topology(int topology_id)
{
  return ccx_core.delete_topology(topology_id);
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
  return ccx_core.reset_preserved_sets(topology_id);
}

bool CalculiXTopologyInterface::add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  return ccx_core.add_preserved_sets(topology_id, set_type, ids);
}

bool CalculiXTopologyInterface::remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  return ccx_core.remove_preserved_sets(topology_id, set_type, ids);
}

bool CalculiXTopologyInterface::get_topology_set_element_ids(int topology_id,int set_type,int set_id,std::vector<int>& element_ids)
{
  return ccx_core.get_topology_set_element_ids(topology_id, set_type, set_id, element_ids);
}

bool CalculiXTopologyInterface::initialize_topology(int topology_id, const std::vector<int>& section_ids)
{
  return ccx_core.initialize_topology(topology_id, section_ids);
}

std::vector<std::vector<int>> CalculiXTopologyInterface::get_topology_element_preservation(int topology_id)
{
  return ccx_core.get_topology_element_preservation(topology_id);
}

bool CalculiXTopologyInterface::set_topology_iteration_factors(int topology_id,int iteration_id,const std::vector<int>& element_ids,const std::vector<double>& stiffness_factors)
{
  return ccx_core.set_topology_iteration_factors(topology_id, iteration_id, element_ids, stiffness_factors);
}

bool CalculiXTopologyInterface::check_topology_material_assignment(int topology_id, int iteration_id, int element_id)
{
  return ccx_core.check_topology_material_assignment(topology_id, iteration_id, element_id);
}

std::string CalculiXTopologyInterface::get_topology_material_name(int topology_id, int element_id)
{
  return ccx_core.get_topology_material_name(topology_id, element_id);
}

bool CalculiXTopologyInterface::get_topology_group_material_name(int topology_id, int iteration_id, int element_id, std::string& material_name)
{
  return ccx_core.get_topology_group_material_name(topology_id, iteration_id, element_id, material_name);
}

std::vector<double> CalculiXTopologyInterface::get_topology_parameters(int topology_id)
{
  return ccx_core.get_topology_parameters(topology_id);
}

bool CalculiXTopologyInterface::get_topology_settings(int topology_id, TopologyOptions& options)
{
  return ccx_core.get_topology_settings(topology_id, options);
}

int CalculiXTopologyInterface::get_topology_iteration_job_id(int topology_id, int iteration_id)
{
  return ccx_core.get_topology_iteration_job_id(topology_id, iteration_id);
}

int CalculiXTopologyInterface::get_topology_iteration_adjoint_job_id(int topology_id, int iteration_id)
{
  return ccx_core.get_topology_iteration_adjoint_job_id(topology_id, iteration_id);
}

std::vector<std::vector<std::string>> CalculiXTopologyInterface::get_topology_iteration_data(int topology_id)
{
  return ccx_core.get_topology_iteration_data(topology_id);
}

bool CalculiXTopologyInterface::select_topology_iteration(int topology_id,int iteration_id)
{
  return ccx_core.select_topology_iteration(topology_id, iteration_id);
}

std::string CalculiXTopologyInterface::get_topology_run_state(int topology_id)
{
  return ccx_core.get_topology_run_state(topology_id);
}

std::vector<std::vector<double>> CalculiXTopologyInterface::get_topology_iteration_element_data(int topology_id,int iteration_id)
{
  return ccx_core.get_topology_iteration_element_data(topology_id, iteration_id);
}

bool CalculiXTopologyInterface::is_topology_adjoint_export() const
{
  return ccx_core.is_topology_adjoint_export();
}

bool CalculiXTopologyInterface::check_topology_export(int topology_id, int iteration_id)
{
  return ccx_core.check_topology_export(topology_id, iteration_id);
}

std::vector<std::vector<std::string>> CalculiXTopologyInterface::get_topology_tree_data()
{
  return ccx_core.get_topology_tree_data();
}
