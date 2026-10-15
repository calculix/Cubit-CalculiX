#include "ccxWaitTopologyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxWaitTopologyCommand::ccxWaitTopologyCommand()
{}

ccxWaitTopologyCommand::~ccxWaitTopologyCommand()
{}

std::vector<std::string> ccxWaitTopologyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("wait topology <value:label='topology_id',help='<topology_id>'>");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxWaitTopologyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx wait topology <topology_id>"; 

  return help;
}

std::vector<std::string> ccxWaitTopologyCommand::get_help()
{
  std::vector<std::string> help;
  help.push_back("Wait for the current topology run to finish.");
  return help;
}

bool ccxWaitTopologyCommand::execute(CubitCommandData& data)
{
  CalculiXCoreInterface ccx_iface;
  int topology_id = -1;
  if (!data.get_value("topology_id", topology_id))
  {
    PRINT_ERROR("Missing topology ID.\n");
    return false;
  }
  return ccx_iface.wait_topology(topology_id);
}
