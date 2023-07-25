#include "ccxSurfaceInteractionGapHeatGenerationModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionGapHeatGenerationModifyCommand::ccxSurfaceInteractionGapHeatGenerationModifyCommand()
{}

ccxSurfaceInteractionGapHeatGenerationModifyCommand::~ccxSurfaceInteractionGapHeatGenerationModifyCommand()
{}

std::vector<std::string> ccxSurfaceInteractionGapHeatGenerationModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("modify surfaceinteraction <value:label='surfaceinteraction id',help='<surfaceinteraction id>'>");
  syntax.append("gapheatgeneration " );
  syntax.append("heat_conversion_factor <value:label='heat_conversion_factor',help='<heat_conversion_factor>'> " );
  syntax.append("surface_weighing_factor <value:label='surface_weighing_factor',help='<surface_weighing_factor>'> " );
  syntax.append("differential_tangential_velocity <value:label='differential_tangential_velocity',help='<differential_tangential_velocity>'> " );

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionGapHeatGenerationModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify surfaceinteraction <surfaceinteraction id> gapheatgeneration heat_conversion_factor <heat_conversion_factor> surface_weighing_factor <surface_weighing_factor> differential_tangential_velocity <differential_tangential_velocity>";

  return help;
}

std::vector<std::string> ccxSurfaceInteractionGapHeatGenerationModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionGapHeatGenerationModifyCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<std::string> options;
  std::vector<int> options_marker;
  std::vector<std::vector<std::string>> options2;
  double heat_conversion_factor_value;
  double surface_weighing_factor_value;
  double differential_tangential_velocity_value;
  std::string heat_conversion_factor;
  std::string surface_weighing_factor;
  std::string differential_tangential_velocity;
  int surfaceinteraction_id;


  data.get_value("surfaceinteraction id", surfaceinteraction_id);

  if (!data.get_value("heat_conversion_factor", heat_conversion_factor_value))
  {
    heat_conversion_factor = "";
    options_marker.push_back(0);
  }else
  {
    heat_conversion_factor = ccx_iface.to_string_scientific(heat_conversion_factor_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("surface_weighing_factor", surface_weighing_factor_value))
  {
    surface_weighing_factor = "";
    options_marker.push_back(0);
  }else
  {
    surface_weighing_factor = ccx_iface.to_string_scientific(surface_weighing_factor_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("differential_tangential_velocity", differential_tangential_velocity_value))
  {
    differential_tangential_velocity = "";
    options_marker.push_back(0);
  }else
  {
    differential_tangential_velocity = ccx_iface.to_string_scientific(differential_tangential_velocity_value);
    options_marker.push_back(1);
  }
  
  options.push_back(heat_conversion_factor);
  options.push_back(surface_weighing_factor);
  options.push_back(differential_tangential_velocity);
  
  if (!ccx_iface.modify_surfaceinteraction("GAPHEATGENERATION",surfaceinteraction_id,options,options_marker,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}