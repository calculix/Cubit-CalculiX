#include "ccxTopologyDeleteCommand.hpp"
#include <string>
#include <vector>
#include "Core/topology/CalculiXTopologyInterface.hpp"
std::vector<std::string> ccxTopologyDeleteCommand::get_syntax()
{
  return {
      "ccx delete topology <value:label='topology_id'>"
  };
}

std::vector<std::string> ccxTopologyDeleteCommand::get_syntax_help()
{
  return {"ccx delete topology <id>"};
}

std::vector<std::string> ccxTopologyDeleteCommand::get_help()
{
  return {
      "Delete a topology case and its linked job/result records."
  };
}

bool ccxTopologyDeleteCommand::execute(CubitCommandData& data)
{
  CalculiXTopologyInterface ccx_topology_iface;


  int topology_id = -1;
  data.get_value("topology_id", topology_id);

  return ccx_topology_iface.delete_topology(topology_id);
}