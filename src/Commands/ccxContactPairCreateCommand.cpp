#include "ccxContactPairCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxContactPairCreateCommand::ccxContactPairCreateCommand()
{}

ccxContactPairCreateCommand::~ccxContactPairCreateCommand()
{}

std::vector<std::string> ccxContactPairCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create contactpair ");
  syntax.append("surfaceinteraction <value:label='surfaceinteraction id',help='<surfaceinteraction id>'> "); 
  syntax.append("{nodetosurface|surfacetosurface|mortar|linmortar|pglinmortar|massless} ");
  syntax.append("master <value:label='master id',help='<sideset id>'> ");
  syntax.append("slave <value:label='slave id',help='<sideset id>'> ");
  syntax.append("[{adjust <value:label='adjust',help='<adjust>'>|adjust_nodeset <value:label='adjust_nodeset',help='<adjust_nodeset>'>} ] ");
  
  syntax_list.push_back(syntax);
  


  return syntax_list;
}

std::vector<std::string> ccxContactPairCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create contactpair surfaceinteraction <surfaceinteraction id> {nodetosurface|surfacetosurface|mortar|linmortar|pglinmortar|massless} master <sideset id> slave <sideset id> [{adjust <value>|adjust_nodeset <nodeset id>}]"; 

  return help;
}

std::vector<std::string> ccxContactPairCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxContactPairCreateCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  int surfaceinteraction_value;
  int master_value;
  int slave_value;
  double adjust_value;
  std::string surfaceinteraction_id;
  std::string master_id;
  std::string slave_id;
  std::string adjust;
  int adjust_nodeset_value;
  std::string adjust_nodeset;
  
  data.get_value("surfaceinteraction id", surfaceinteraction_value);
  surfaceinteraction_id = std::to_string(surfaceinteraction_value);
  options.push_back(surfaceinteraction_id);

  if (data.find_keyword("NODETOSURFACE"))
  {
    options.push_back("NODE TO SURFACE");
  } else if (data.find_keyword("SURFACETOSURFACE"))
  {
    options.push_back("SURFACE TO SURFACE");
  } else if (data.find_keyword("MORTAR"))
  {
    options.push_back("MORTAR");
  } else if (data.find_keyword("LINMORTAR"))
  {
    options.push_back("LINMORTAR");
  } else if (data.find_keyword("PGLINMORTAR"))
  {
    options.push_back("PGLINMORTAR");
  } else if (data.find_keyword("MASSLESS"))
  {
    options.push_back("MASSLESS");
  }

  data.get_value("master id", master_value);
  data.get_value("slave id", slave_value);
  master_id = std::to_string(master_value);
  slave_id = std::to_string(slave_value);
  options.push_back(master_id);
  options.push_back(slave_id);

  if (!data.get_value("adjust", adjust_value))
  {
    adjust = "";
  }
  else
  {
    adjust = std::to_string(adjust_value);
  }
  
  options.push_back(adjust);

  if (!data.get_value("adjust_nodeset", adjust_nodeset_value))
  {
    adjust_nodeset = "";
  }
  else
  {
    adjust_nodeset = std::to_string(adjust_nodeset_value);
  }
  
  options.push_back(adjust_nodeset);

  if (!ccx_iface.create_contactpair(options))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}