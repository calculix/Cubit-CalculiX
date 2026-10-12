#include "CoreTopology.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

#include <cmath>
#include <set>

// HELPERS
namespace
{
void ApplyTopologyOptions(
    TopologyCase& configuration,
    const TopologyOptions& options)
{
  if (options.has_name)
    configuration.name = options.name;

  if (options.has_volume_fraction)
    configuration.target_volume_fraction = options.volume_fraction;

  if (options.has_void_ratio)
    configuration.void_stiffness_ratio = options.void_ratio;

  if (options.has_maximum_iterations)
    configuration.maximum_iterations = options.maximum_iterations;
}

bool ValidateTopologyConfiguration(const TopologyCase& configuration)
{
  return
      configuration.name.find_first_not_of(" \t\r\n")
          != std::string::npos &&
      std::isfinite(configuration.target_volume_fraction) &&
      configuration.target_volume_fraction > 0.0 &&
      configuration.target_volume_fraction <= 1.0 &&
      std::isfinite(configuration.void_stiffness_ratio) &&
      configuration.void_stiffness_ratio > 0.0 &&
      configuration.void_stiffness_ratio < 1.0 &&
      configuration.maximum_iterations > 0;
}
}
// HELPERS END

CoreTopology::CoreTopology()
{
  ccx_iface = new CalculiXCoreInterface();
}

CoreTopology::~CoreTopology()
{
  delete ccx_iface;
}

bool CoreTopology::init()
{
  if (is_initialized)
    return false;

  is_initialized = true;
  return true;
}

bool CoreTopology::update()
{
  // Case configuration currently changes only through explicit methods.
  return is_initialized;
}

bool CoreTopology::reset()
{
  topology_cases.clear();
  return true;
}

bool CoreTopology::check_initialized()
{
  return is_initialized;
}

bool CoreTopology::create_topology(
    int topology_id,
    const TopologyOptions& options)
{
  if (!is_initialized || topology_id <= 0)
  {
    PRINT_ERROR("Invalid topology initialization or ID.\n");
    return false;
  }

  if (topology_cases.count(topology_id) != 0)
  {
    PRINT_ERROR("Topology case %d already exists.\n", topology_id);
    return false;
  }

  // Defaults belong to CoreTopology.
  TopologyCase configuration;
  configuration.name = "topology_" + std::to_string(topology_id);
  configuration.target_volume_fraction = 0.3;
  configuration.void_stiffness_ratio = 1.0e-3;
  configuration.maximum_iterations = 100;

  ApplyTopologyOptions(configuration, options);

  if (!ValidateTopologyConfiguration(configuration))
  {
    PRINT_ERROR("Invalid topology case configuration.\n");
    return false;
  }

  topology_cases.emplace(topology_id, configuration);
  return true;
}

bool CoreTopology::modify_topology(
    int topology_id,
    const TopologyOptions& options)
{
  if (!is_initialized)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  // Preserve selections, element flags, and iteration records.
  TopologyCase configuration = entry->second;
  ApplyTopologyOptions(configuration, options);

  if (!ValidateTopologyConfiguration(configuration))
  {
    PRINT_ERROR("Invalid topology case configuration.\n");
    return false;
  }

  entry->second = configuration;
  return true;
}

bool CoreTopology::delete_topology(int topology_id)
{
  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  // The coordinating core must remove linked jobs first.
  if (!entry->second.iteration_jobs.empty())
  {
    PRINT_ERROR("Topology case %d still has linked iteration jobs.\n",
                topology_id);
    return false;
  }

  topology_cases.erase(entry);
  return true;
}

bool CoreTopology::resolve_preserved_elements(int topology_id, const std::vector<int>& analysis_element_ids, const std::map<int, std::set<int>>& default_sets, const std::set<int>& default_elements)
{
  if (!is_initialized || !ccx_iface)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (analysis_element_ids.empty())
  {
    PRINT_ERROR("Topology %d has no analysis elements.\n", topology_id);
    return false;
  }

  std::map<int, bool> flags;

  for (int element_id : analysis_element_ids)
  {
    if (element_id <= 0 ||
        !flags.emplace(element_id, false).second)
    {
      PRINT_ERROR("Invalid or duplicate topology element %d.\n",
                  element_id);
      return false;
    }
  }

  const auto& configuration = entry->second;
  auto effective_sets = default_sets;

  for (const auto& group : configuration.added_preserved_sets)
    effective_sets[group.first].insert(
        group.second.begin(), group.second.end());

  for (const auto& group : configuration.removed_preserved_sets)
    for (int set_id : group.second)
      effective_sets[group.first].erase(set_id);

  for (const auto& group : effective_sets)
  {
    const int set_type = group.first;

    if (set_type < 1 || set_type > 3)
    {
      PRINT_ERROR("Invalid default preserved set type %d.\n",
                  set_type);
      return false;
    }

    for (int set_id : group.second)
    {
      std::vector<int> element_ids;

      if (!ccx_iface->get_topology_set_element_ids(
              topology_id, set_type, set_id, element_ids))
        return false;

      for (int element_id : element_ids)
      {
        const auto flag = flags.find(element_id);

        if (flag == flags.end())
        {
          PRINT_ERROR("Unexpected topology element %d.\n", element_id);
          return false;
        }

        flag->second = true;
      }
    }
  }

  for (int element_id : default_elements)
  {
    if (element_id <= 0)
    {
      PRINT_ERROR("Invalid default preserved element %d.\n",
                  element_id);
      return false;
    }

    const auto flag = flags.find(element_id);

    // Only elements participating in this topology analysis are marked.
    if (flag != flags.end())
      flag->second = true;
  }

  // Commit only after all queries succeed.
  entry->second.element_preserved.swap(flags);
  return true;
}

std::vector<std::vector<std::string>> CoreTopology::get_tree_data() const
{
  std::vector<std::vector<std::string>> rows;

  for (const auto& entry : topology_cases)
    rows.push_back({std::to_string(entry.first), entry.second.name});

  return rows;
}

bool CoreTopology::change_preserved_sets(int topology_id,int set_type,const std::vector<int>& ids,bool adding)
{
  if (!is_initialized)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology_cases.find(topology_id);

  if (entry == topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (set_type < 1 || set_type > 3)
  {
    PRINT_ERROR("Invalid topology preserved set type.\n");
    return false;
  }

  // Validate the complete list before changing configuration.
  for (int id : ids)
  {
    bool exists = false;

    if (id > 0)
    {
      if (set_type == 1)
        exists = ccx_iface->check_block_exists(id);
      else if (set_type == 2)
        exists = ccx_iface->check_nodeset_exists(id);
      else
        exists = ccx_iface->check_sideset_exists(id);
    }

    if (!exists)
    {
      PRINT_ERROR("Preserved set type %d, ID %d does not exist.\n", set_type, id);
      return false;
    }
  }

  auto& configuration = entry->second;

  for (int id : ids)
  {
    if (adding)
    {
      configuration.removed_preserved_sets[set_type].erase(id);
      configuration.added_preserved_sets[set_type].insert(id);
    }
    else
    {
      configuration.added_preserved_sets[set_type].erase(id);
      configuration.removed_preserved_sets[set_type].insert(id);
    }
  }

  return true;
}

bool CoreTopology::add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  return change_preserved_sets(topology_id, set_type, ids, true);
}

bool CoreTopology::remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  return change_preserved_sets(topology_id, set_type, ids, false);
}