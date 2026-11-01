#include "Core/CalculiXCore.hpp"
#include "CoreTopologyOutputPreparation.hpp"
#include "CoreTopologyElementHistoryOutputPreparation.hpp"
#include "CoreTopologyNodeHistoryOutputPreparation.hpp"
#include "CoreTopologyFieldOutputPreparation.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::prepare_topology_history_outputs(int topology_id, const std::set<int>& block_ids)
{
  if (!topology || !steps || !historyoutputs)
  {
    PRINT_ERROR("Topology history output cores are not initialized.\n");
    return false;
  }

  const CoreTopologyOutputPreparationData data = {
    *topology,
    *steps,
    *historyoutputs,
    cb,
    me_iface,
    fieldoutputs
  };

  const auto execute_command = [this](const std::string& text)
  {
    return silent_cmd(text);
  };

  return CoreTopologyOutputPreparation::prepare(data, topology_id, block_ids, execute_command);
}

bool CalculiXCore::ensure_topology_element_history_output(int topology_id, int step_id, int block_id, const std::string& key)
{
  if (!historyoutputs || !steps || !cb || !cb->check_block_exists(block_id))
  {
    PRINT_ERROR("Invalid topology history output request.\n");
    return false;
  }

  const auto execute_command = [this](const std::string& text)
  {
    return silent_cmd(text);
  };

  return CoreTopologyElementHistoryOutputPreparation::ensure(*historyoutputs, *steps, topology_id, step_id, block_id, key, execute_command);
}

bool CalculiXCore::ensure_topology_displacement_field_output(int topology_id, int step_id)
{
  if (!fieldoutputs || !steps)
  {
    PRINT_ERROR("Topology field output interfaces are not initialized.\n");
    return false;
  }

  const auto command = [this](const std::string& text)
  {
    return silent_cmd(text);
  };

  return CoreTopologyFieldOutputPreparation::ensure_displacement(*fieldoutputs, *steps, topology_id, step_id, command);
}

bool CalculiXCore::ensure_topology_node_history_output(int topology_id, int step_id, int nodeset_id, const std::string& key)
{
  if (!historyoutputs || !steps || !me_iface || !CoreMeshQuery(*me_iface).check_nodeset_exists(nodeset_id))
  {
    PRINT_ERROR("Invalid topology history output request.\n");
    return false;
  }

  const auto execute_command = [this](const std::string& text)
  {
    return silent_cmd(text);
  };

  return CoreTopologyNodeHistoryOutputPreparation::ensure(*historyoutputs, *steps, topology_id, step_id, nodeset_id, key, execute_command);
}
