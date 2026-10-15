#include "ccxRunTopologyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxRunTopologyCommand::ccxRunTopologyCommand()
{}

ccxRunTopologyCommand::~ccxRunTopologyCommand()
{}

std::vector<std::string> ccxRunTopologyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("run topology <value:label='topology_id',help='<topology_id>'> [wait]");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxRunTopologyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx run topology <topology_id> [wait]"; 

  return help;
}

std::vector<std::string> ccxRunTopologyCommand::get_help()
{
  std::vector<std::string> help;
  help.push_back("Start topology optimization; add wait to wait for completion.");
  return help;
}

bool ccxRunTopologyCommand::execute(CubitCommandData& data)
{
  CalculiXCoreInterface ccx_iface;
  int topology_id = -1;
  if (!data.get_value("topology_id", topology_id))
  {
    PRINT_ERROR("Missing topology ID.\n");
    return false;
  }
  return ccx_iface.run_topology(topology_id, data.find_keyword("wait"));
}
