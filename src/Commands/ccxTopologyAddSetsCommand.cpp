#include "ccxTopologyAddSetsCommand.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxTopologyAddSetsCommand::get_syntax()
{
  return {
      "ccx topology <value:label='topology_id'> add preserved "
      "[block <value:label='block_ids'>...] "
      "[nodeset <value:label='nodeset_ids'>...] "
      "[sideset <value:label='sideset_ids'>...]"
  };
}

std::vector<std::string> ccxTopologyAddSetsCommand::get_syntax_help()
{
  return {
      "ccx topology <id> add preserved "
      "[block <ids...>] [nodeset <ids...>] [sideset <ids...>]"
  };
}

std::vector<std::string> ccxTopologyAddSetsCommand::get_help()
{
  return {"Add preserved blocks, nodesets, or sidesets."};
}

bool ccxTopologyAddSetsCommand::execute(CubitCommandData& data)
{
  CalculiXCoreInterface ccx_iface;
  int topology_id = -1;

  std::vector<int> blocks;
  std::vector<int> nodesets;
  std::vector<int> sidesets;

  data.get_value("topology_id", topology_id);
  data.get_values("block_ids", blocks);
  data.get_values("nodeset_ids", nodesets);
  data.get_values("sideset_ids", sidesets);

  if (blocks.empty() && nodesets.empty() && sidesets.empty())
  {
    PRINT_ERROR("Specify a block, nodeset, or sideset.\n");
    return false;
  }

  // Check every set before applying any of the three lists.
  for (int id : blocks)
    if (id <= 0 || !ccx_iface.check_block_exists(id))
    {
      PRINT_ERROR("Block %d does not exist.\n", id);
      return false;
    }

  for (int id : nodesets)
    if (id <= 0 || !ccx_iface.check_nodeset_exists(id))
    {
      PRINT_ERROR("Nodeset %d does not exist.\n", id);
      return false;
    }

  for (int id : sidesets)
    if (id <= 0 || !ccx_iface.check_sideset_exists(id))
    {
      PRINT_ERROR("Sideset %d does not exist.\n", id);
      return false;
    }

  if (!blocks.empty() &&
      !ccx_iface.add_preserved_sets(topology_id, 1, blocks))
    return false;

  if (!nodesets.empty() &&
      !ccx_iface.add_preserved_sets(topology_id, 2, nodesets))
    return false;

  if (!sidesets.empty() &&
      !ccx_iface.add_preserved_sets(topology_id, 3, sidesets))
    return false;

  return true;
}