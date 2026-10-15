#include "ccxTopologyResetPreservedCommand.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxTopologyResetPreservedCommand::get_syntax()
{
  return {
    "ccx topology <value:label='topology_id'> reset preserved"
  };
}

std::vector<std::string> ccxTopologyResetPreservedCommand::get_syntax_help()
{
  return {"ccx topology <id> reset preserved"};
}

std::vector<std::string> ccxTopologyResetPreservedCommand::get_help()
{
  return {
    "Clear preservation overrides for subsequent topology runs. "
    "Stored iterations and results are retained."
  };
}

bool ccxTopologyResetPreservedCommand::execute(CubitCommandData& data)
{
  CalculiXCoreInterface ccx_iface;

  int topology_id = -1;
  data.get_value("topology_id", topology_id);

  if (!ccx_iface.reset_preserved_sets(topology_id))
  {
    return false;
  }

  PRINT_INFO("Topology %d: restored automatic preservation defaults "
    "for subsequent runs.\n", topology_id);

  return true;
}
