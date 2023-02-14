#include "ccxContactPairCreateFromCubitContactPairCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxContactPairCreateFromCubitContactPairCommand::ccxContactPairCreateFromCubitContactPairCommand()
{}

ccxContactPairCreateFromCubitContactPairCommand::~ccxContactPairCreateFromCubitContactPairCommand()
{}

std::vector<std::string> ccxContactPairCreateFromCubitContactPairCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create contactpair ");
  syntax.append("from cubitcontactpair ");
  syntax.append("surfaceinteraction <value:label='surfaceinteraction id',help='<surfaceinteraction id>'> ");
  syntax.append("{nodetosurface|surfacetosurface|mortar|linmortar|pglinmortar|massless} ");
  syntax.append("[adjust <value:label='adjust',help='<adjust>'>] ");
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxContactPairCreateFromCubitContactPairCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create contactpair from cubitcontactpair surfaceinteraction <surfaceinteraction id> {nodetosurface|surfacetosurface|mortar|linmortar|pglinmortar|massless} [adjust <value>]"; 

  return help;
}

std::vector<std::string> ccxContactPairCreateFromCubitContactPairCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxContactPairCreateFromCubitContactPairCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;
  
  std::string output;
  std::vector<std::string> options;  
  int surfaceinteraction_value;
  double adjust_value;
  std::string surfaceinteraction_id;
  std::string adjust;
  
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

  if (!data.get_value("adjust", adjust_value))
  {
    adjust = "";
  }
  else
  {
    adjust = std::to_string(adjust_value);
  }
  
  options.push_back(adjust);

  if (!ccx_iface.create_contactpair_from_cubitcontactpair(surfaceinteraction_value,options[1],options[2]))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear(); 
  return true;
}