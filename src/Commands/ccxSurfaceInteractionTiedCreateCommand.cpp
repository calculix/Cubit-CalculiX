#include "ccxSurfaceInteractionTiedCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionTiedCreateCommand::ccxSurfaceInteractionTiedCreateCommand()
{}

ccxSurfaceInteractionTiedCreateCommand::~ccxSurfaceInteractionTiedCreateCommand()
{}

std::vector<std::string> ccxSurfaceInteractionTiedCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("create surfaceinteraction ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("tied " );
  syntax.append("slopeK <value:label='slopeK',help='<slopeK>'>" );
  
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionTiedCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create surfaceinteraction name <name> tied slopeK <slopeK>"; 

  return help;
}

std::vector<std::string> ccxSurfaceInteractionTiedCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionTiedCreateCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string name;
  std::vector<std::string> options;
  std::vector<std::vector<std::string>> options2;
  double slopeK_value;
  std::string slopeK;
  
  data.get_string("name", name);
  data.get_value("slopeK", slopeK_value);
  slopeK = std::to_string(slopeK_value);

  options.push_back(name);
  options.push_back(slopeK);
  
  if (!ccx_iface.create_surfaceinteraction("TIED", options, options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}