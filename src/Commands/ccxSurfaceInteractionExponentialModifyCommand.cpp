#include "ccxSurfaceInteractionExponentialModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionExponentialModifyCommand::ccxSurfaceInteractionExponentialModifyCommand()
{}

ccxSurfaceInteractionExponentialModifyCommand::~ccxSurfaceInteractionExponentialModifyCommand()
{}

std::vector<std::string> ccxSurfaceInteractionExponentialModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("modify surfaceinteraction <value:label='surfaceinteraction id',help='<surfaceinteraction id>'>");
  syntax.append("exponential ");
  syntax.append("[c0 <value:label='c0',help='<c0>'>] " );
  syntax.append("[p0 <value:label='p0',help='<p0>'>] " );

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionExponentialModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify surfaceinteraction <surfaceinteraction id> exponential [c0 <c0>] [p0 <p0>]"; 

  return help;
}

std::vector<std::string> ccxSurfaceInteractionExponentialModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionExponentialModifyCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<std::string> options;
  std::vector<int> options_marker;
  std::vector<std::vector<std::string>> options2;
  double c0_value;
  double p0_value;
  std::string c0;
  std::string p0;
  int surfaceinteraction_id;

  data.get_value("surfaceinteraction id", surfaceinteraction_id);

  if (!data.get_value("c0", c0_value))
  {
    c0 = "";
    options_marker.push_back(0);
  }else
  {
    c0 = std::to_string(c0_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("p0", p0_value))
  {
    p0 = "";
    options_marker.push_back(0);
  }else
  {
    p0 = std::to_string(p0_value);
    options_marker.push_back(1);
  }

  options.push_back(c0);
  options.push_back(p0);
  
  if (!ccx_iface.modify_surfaceinteraction("EXPONENTIAL",surfaceinteraction_id,options,options_marker,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}