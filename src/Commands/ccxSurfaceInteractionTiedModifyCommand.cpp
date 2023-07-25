#include "ccxSurfaceInteractionTiedModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionTiedModifyCommand::ccxSurfaceInteractionTiedModifyCommand()
{}

ccxSurfaceInteractionTiedModifyCommand::~ccxSurfaceInteractionTiedModifyCommand()
{}

std::vector<std::string> ccxSurfaceInteractionTiedModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("modify surfaceinteraction <value:label='surfaceinteraction id',help='<surfaceinteraction id>'>");
  syntax.append("tied " );
  syntax.append("slopeK <value:label='slopeK',help='<slopeK>'>" );

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionTiedModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify surfaceinteraction <surfaceinteraction id> tied slopeK <slopeK>"; 

  return help;
}

std::vector<std::string> ccxSurfaceInteractionTiedModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionTiedModifyCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<std::string> options;
  std::vector<int> options_marker;
  std::vector<std::vector<std::string>> options2;
  double slopeK_value;
  std::string slopeK;
  int surfaceinteraction_id;



  data.get_value("surfaceinteraction id", surfaceinteraction_id);

  if (!data.get_value("slopeK", slopeK_value))
  {
    slopeK = "";
    options_marker.push_back(0);
  }else
  {
    slopeK = ccx_iface.to_string_scientific(slopeK_value);
    options_marker.push_back(1);
  }
  
  options.push_back(slopeK);
  
  if (!ccx_iface.modify_surfaceinteraction("TIED",surfaceinteraction_id,options,options_marker,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}