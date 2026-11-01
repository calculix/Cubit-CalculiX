#include "CoreTopologyOutputPreparation.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyElementHistoryOutputPreparation.hpp"
#include "CoreTopologyNodeHistoryOutputPreparation.hpp"
#include "CoreTopologyFieldOutputPreparation.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include <vector>

bool CoreTopologyOutputPreparation::prepare(const CoreTopologyOutputPreparationData& data, int topology_id, const std::set<int>& block_ids, const std::function<bool(const std::string&)>& execute_command)
{
  const auto entry = data.topology.topology_cases.find(topology_id);

  if (entry == data.topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const int nodeset_id = entry->second.displacement_nodeset;
  const bool needs_displacement_sensitivities = entry->second.objective == TopologyObjective::Displacement || nodeset_id > 0;
  std::vector<int> step_ids;

  for (const auto& step : data.steps.steps_data)
  {
    if (step.empty())
    {
      PRINT_ERROR("Invalid step data.\n");
      return false;
    }

    step_ids.push_back(step[0]);
  }

  if (step_ids.empty())
  {
    PRINT_ERROR("Topology optimization requires an analysis step.\n");
    return false;
  }

  if (nodeset_id > 0)
  {
    if (!data.mesh)
    {
      PRINT_ERROR("Invalid topology history output request.\n");
      return false;
    }

    if (!CoreMeshQuery(*data.mesh).check_nodeset_exists(nodeset_id) || CubitInterface::get_nodeset_nodes_inclusive(nodeset_id).empty())
    {
      PRINT_ERROR("Topology displacement nodeset %d is missing or empty.\n", nodeset_id);
      return false;
    }
  }

  const auto ensure_element = [&](int step_id, int block_id, const std::string& key) -> bool
  {
    if (!data.blocks || !data.blocks->check_block_exists(block_id))
    {
      PRINT_ERROR("Invalid topology history output request.\n");
      return false;
    }

    return CoreTopologyElementHistoryOutputPreparation::ensure(data.historyoutputs, data.steps, topology_id, step_id, block_id, key, execute_command);
  };

  for (int step_id : step_ids)
  {
    for (int block_id : block_ids)
    {
      if (!ensure_element(step_id, block_id, "ELSE"))
      {
        return false;
      }

      if (needs_displacement_sensitivities)
      {
        if (!ensure_element(step_id, block_id, "S"))
        {
          return false;
        }

        if (!ensure_element(step_id, block_id, "ME"))
        {
          return false;
        }
      }
    }

    if (nodeset_id > 0)
    {
      if (!data.mesh || !CoreMeshQuery(*data.mesh).check_nodeset_exists(nodeset_id))
      {
        PRINT_ERROR("Invalid topology history output request.\n");
        return false;
      }

      if (!CoreTopologyNodeHistoryOutputPreparation::ensure(data.historyoutputs, data.steps, topology_id, step_id, nodeset_id, "U", execute_command))
      {
        return false;
      }
    }
  }

  if (nodeset_id > 0)
  {
    if (!data.fieldoutputs)
    {
      PRINT_ERROR("Topology field output interfaces are not initialized.\n");
      return false;
    }

    if (!CoreTopologyFieldOutputPreparation::ensure_displacement(*data.fieldoutputs, data.steps, topology_id, step_ids.back(), execute_command))
    {
      return false;
    }
  }

  return true;
}
