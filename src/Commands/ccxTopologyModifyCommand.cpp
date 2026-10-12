#include "ccxTopologyModifyCommand.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"
#include "CoreTopologyStructs.hpp"

std::vector<std::string> ccxTopologyModifyCommand::get_syntax()
{
  return {
      "ccx modify topology <value:label='topology_id'> "
      "[name <string:label='name'>] "
      "[volume_fraction <value:label='volume_fraction'>] "
      "[void_ratio <value:label='void_ratio'>] "
      "[iterations <value:label='maximum_iterations'>]"
  };
}

std::vector<std::string> ccxTopologyModifyCommand::get_syntax_help()
{
  return {
      "ccx modify topology <id> [name <name>] "
      "[volume_fraction <value>] [void_ratio <value>] [iterations <count>]"
  };
}

std::vector<std::string> ccxTopologyModifyCommand::get_help()
{
  return {
      "Modify topology configuration while preserving stored iterations."
  };
}

bool ccxTopologyModifyCommand::execute(CubitCommandData& data)
{
  CalculiXCoreInterface ccx_iface;

  int topology_id = -1;
  TopologyOptions options;

  data.get_value("topology_id", topology_id);
  
  if (data.find_keyword("name"))
  {
    options.has_name = true;
    data.get_string("name", options.name);
  }

  if (data.find_keyword("volume_fraction"))
  {
    options.has_volume_fraction = true;
    data.get_value("volume_fraction", options.volume_fraction);
  }
  
  if (data.find_keyword("void_ratio"))
  {
    options.has_void_ratio = true;
    data.get_value("void_ratio", options.void_ratio);
  }
  
  if (data.find_keyword("iterations"))
  {
    options.has_maximum_iterations = true;
    data.get_value("maximum_iterations", options.maximum_iterations);
  }

  return ccx_iface.modify_topology(topology_id, options);
}