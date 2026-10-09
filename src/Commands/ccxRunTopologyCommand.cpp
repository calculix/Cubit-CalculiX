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
  syntax.append("run topology <value:label='topology_id',help='<topology_id>'> job <value:label='job_id',help='<job_id>'>");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxRunTopologyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx run topology <topology_id> job <job_id>"; 

  return help;
}

std::vector<std::string> ccxRunTopologyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxRunTopologyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  //std::string output;

  int topology_id;
  int job_id;
  //int option;

  /*
  if (data.find_keyword("NO_CONVERSION")){
    option = -1;
  }else{
    option = 1;
  }
  */

  data.get_value("topology_id", topology_id);
  data.get_value("job_id", job_id);

  if (!ccx_iface.run_topology(topology_id,job_id))
  {
    PRINT_ERROR("Topology initialization failed.\n");
    return false;
  }
    
  return true;
}