#include "ccxKillTopologyCommand.hpp"
#include <string>
#include <vector>
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxKillTopologyCommand::ccxKillTopologyCommand()
{}

ccxKillTopologyCommand::~ccxKillTopologyCommand()
{}

std::vector<std::string> ccxKillTopologyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("kill topology <value:label='topology_id',help='<topology_id>'>");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxKillTopologyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx kill topology <topology_id>"; 

  return help;
}

std::vector<std::string> ccxKillTopologyCommand::get_help()
{
  std::vector<std::string> help;
  help.push_back("Stop the topology run and retain completed iteration results.");
  return help;
}

bool ccxKillTopologyCommand::execute(CubitCommandData& data)
{
  CalculiXTopologyInterface ccx_topology_iface;

  int topology_id = -1;
  if (!data.get_value("topology_id", topology_id))
  {
    PRINT_ERROR("Missing topology ID.\n");
    return false;
  }
  return ccx_topology_iface.kill_topology(topology_id);
}
