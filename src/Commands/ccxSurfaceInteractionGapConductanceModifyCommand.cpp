#include "ccxSurfaceInteractionGapConductanceModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionGapConductanceModifyCommand::ccxSurfaceInteractionGapConductanceModifyCommand()
{}

ccxSurfaceInteractionGapConductanceModifyCommand::~ccxSurfaceInteractionGapConductanceModifyCommand()
{}

std::vector<std::string> ccxSurfaceInteractionGapConductanceModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("modify surfaceinteraction <value:label='surfaceinteraction id',help='<surfaceinteraction id>'>");
  syntax.append("gapconductance " );
  syntax.append("conductance_contactpressure_temperature <value:label='conductance_contactpressure_temperature',help='<pressure_overclosure>'>... " );
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionGapConductanceModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify surfaceinteraction <surfaceinteraction id> gapconductance conductance_contactpressure_temperature <values>..."; 

  return help;
}

std::vector<std::string> ccxSurfaceInteractionGapConductanceModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionGapConductanceModifyCommand::execute(CubitCommandData &data)
{ 
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<std::string> options;
  std::vector<int> options_marker;
  std::vector<std::vector<std::string>> options2;
  int surfaceinteraction_id;
  std::vector<double> conductance_contactpressure_temperature_values;

  data.get_value("surfaceinteraction id", surfaceinteraction_id);

  data.get_values("conductance_contactpressure_temperature", conductance_contactpressure_temperature_values);
  
  if (conductance_contactpressure_temperature_values.size() % 3 != 0)
  {
    output = "Failed! set of 3 is needed per data line!\n";
    PRINT_ERROR(output.c_str());
    return false;
  }else{
    for (size_t i = 0; i < conductance_contactpressure_temperature_values.size(); i+=3)
    {
      std::vector<std::string> conductance_contactpressure_temperature(3);
      conductance_contactpressure_temperature[0] = std::to_string(conductance_contactpressure_temperature_values[i]);
      conductance_contactpressure_temperature[1] = std::to_string(conductance_contactpressure_temperature_values[i+1]);
      conductance_contactpressure_temperature[2] = std::to_string(conductance_contactpressure_temperature_values[i+2]);
      options2.push_back(conductance_contactpressure_temperature);
      conductance_contactpressure_temperature.clear();
    }
  }
    
  if (!ccx_iface.modify_surfaceinteraction("GAPCONDUCTANCE",surfaceinteraction_id,options,options_marker,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
  options2.clear();

  return true;
}