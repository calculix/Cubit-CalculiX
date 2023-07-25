#include "ccxSurfaceInteractionLinearModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionLinearModifyCommand::ccxSurfaceInteractionLinearModifyCommand()
{}

ccxSurfaceInteractionLinearModifyCommand::~ccxSurfaceInteractionLinearModifyCommand()
{}

std::vector<std::string> ccxSurfaceInteractionLinearModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("modify surfaceinteraction <value:label='surfaceinteraction id',help='<surfaceinteraction id>'>");
  syntax.append("linear " );
  syntax.append("[slopeK <value:label='slopeK',help='<slopeK>'>] " );
  syntax.append("[sigmaINF <value:label='sigmaINF',help='<sigmaINF>'>] " );
  syntax.append("[c0 <value:label='c0',help='<c0>'>] " );

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionLinearModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify surfaceinteraction <surfaceinteraction id> linear [slopeK <slopeK>] [sigmaINF <sigmaINF>] [c0 <c0>]"; 

  return help;
}

std::vector<std::string> ccxSurfaceInteractionLinearModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionLinearModifyCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<std::string> options;
  std::vector<int> options_marker;
  std::vector<std::vector<std::string>> options2;
  double slopeK_value;
  double sigmaINF_value;
  double c0_value;
  std::string slopeK;
  std::string sigmaINF;
  std::string c0;
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
  if (!data.get_value("sigmaINF", sigmaINF_value))
  {
    sigmaINF = "";
    options_marker.push_back(0);
  }else
  {
    sigmaINF = ccx_iface.to_string_scientific(sigmaINF_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("c0", c0_value))
  {
    c0 = "";
    options_marker.push_back(0);
  }else
  {
    c0 = ccx_iface.to_string_scientific(c0_value);
    options_marker.push_back(1);
  }
  
  options.push_back(slopeK);
  options.push_back(sigmaINF);
  options.push_back(c0);
  
  if (!ccx_iface.modify_surfaceinteraction("LINEAR",surfaceinteraction_id,options,options_marker,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}