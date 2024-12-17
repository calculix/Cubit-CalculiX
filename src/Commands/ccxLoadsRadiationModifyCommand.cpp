#include "ccxLoadsRadiationModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsRadiationModifyCommand::ccxLoadsRadiationModifyCommand()
{}

ccxLoadsRadiationModifyCommand::~ccxLoadsRadiationModifyCommand()
{}

std::vector<std::string> ccxLoadsRadiationModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify radiation <value:label='radiation id',help='<radiation id>'> ");
  syntax.append("[sideset <value:label='sideset id',help='<sideset id>'>] ");
  syntax.append("[temperature <value:label='temperature_value',help='<temperature_value>'>] ");
  syntax.append("[emissivity <value:label='emissivity_value',help='<emissivity_value>'>] ");
  syntax.append("[op {mod | new}] " );
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[radiationamplitude <value:label='radiationamplitude id',help='<radiationamplitude id>'>] ");
  syntax.append("[radiationtimedelay <value:label='radiationtimedelay',help='<radiationtimedelay>'>] ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[cavity <string:type='unquoted', number='1', label='cavity', help='<cavity>'>] " );
  syntax.append("[{cavity_radiation_on|cavity_radiation_off}] " );
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxLoadsRadiationModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx "; 
  help[0].append("modify radiation <radiation id> ");
  help[0].append("[sideset <sideset id>] ");
  help[0].append("[temperature <temperature_value>] ");
  help[0].append("[emissivity <emissivity_value>] ");
  help[0].append("[op {mod | new}] " );
  help[0].append("[amplitude <amplitude id>] ");
  help[0].append("[timedelay <timedelay>] ");
  help[0].append("[radiationamplitude <radiationamplitude id>] ");
  help[0].append("[radiationtimedelay <radiationtimedelay>] ");
  help[0].append("[name <name>] " );
  help[0].append("[cavity <cavity>] " );
  help[0].append("[{cavity_radiation_on|cavity_radiation_off}] " );

  return help;
}

std::vector<std::string> ccxLoadsRadiationModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsRadiationModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<int> options_marker;
  std::vector<std::string> options;
  int radiation_id;
  std::string sideset_id;
  int sideset_id_value;
  int op_mode = 0;
  double temperature_value;
  std::string temperature;
  double emissivity_value;
  std::string emissivity;
  std::string amplitude_id;
  int amplitude_value;
  std::string timedelay;
  double timedelay_value;
  std::string radiationamplitude_id;
  int radiationamplitude_value;
  std::string radiationtimedelay;
  double radiationtimedelay_value;
  std::string name; 
  std::string cavity; 
  std::string cavity_radiation; 
  
  data.get_value("radiation id", radiation_id);
  
  if (data.find_keyword("OP")){
    if (data.find_keyword("MOD")){
      options_marker.push_back(1);
      options.push_back("0");
    }else if (data.find_keyword("NEW"))
    {
      options_marker.push_back(1);
      options.push_back("1");
    }
  }else{
      options_marker.push_back(0);
      options.push_back("0");
  }

  if (!data.get_value("amplitude id", amplitude_value))
  {
    amplitude_id = "-1";
    options_marker.push_back(0);
  }
  else
  {
    amplitude_id = std::to_string(amplitude_value);
    options_marker.push_back(1);
  }
  options.push_back(amplitude_id);

  if (!data.get_value("timedelay", timedelay_value))
  {
    timedelay = "";
    options_marker.push_back(0);
  }
  else
  {
    timedelay = std::to_string(timedelay_value);
    options_marker.push_back(1);
  }
  options.push_back(timedelay);
  
  if (!data.get_value("sideset id", sideset_id_value))
  {
    amplitude_id = "-1";
    options_marker.push_back(0);
  }
  else
  {
    sideset_id = std::to_string(sideset_id_value);
    options_marker.push_back(1);
  }
  options.push_back(sideset_id);

  if (!data.get_value("temperature_value", temperature_value))
  {
    temperature = "-1";
    options_marker.push_back(0);
  }
  else
  {
    temperature = ccx_iface.to_string_scientific(temperature_value);
    options_marker.push_back(1);
  }
  options.push_back(temperature);
  
  if (!data.get_value("emissivity_value", emissivity_value))
  {
    emissivity = "-1";
    options_marker.push_back(0);
  }
  else
  {
    emissivity = ccx_iface.to_string_scientific(emissivity_value);
    options_marker.push_back(1);
  }
  options.push_back(emissivity);

  if (!data.get_value("radiationamplitude id", radiationamplitude_value))
  {
    radiationamplitude_id = "-1";
    options_marker.push_back(0);
  }
  else
  {
    radiationamplitude_id = std::to_string(radiationamplitude_value);
    options_marker.push_back(1);
  }
  options.push_back(radiationamplitude_id);

  if (!data.get_value("radiationtimedelay", radiationtimedelay_value))
  {
    radiationtimedelay = "";
    options_marker.push_back(0);
  }
  else
  {
    radiationtimedelay = std::to_string(radiationtimedelay_value);
    options_marker.push_back(1);
  }
  options.push_back(radiationtimedelay);
  
  if (!data.get_string("name", name))
  {
    name = "";
    options_marker.push_back(0);
  }
  else
  {
    options_marker.push_back(1);
  }
  options.push_back(name);
  
  if (!data.get_string("cavity", cavity))
  {
    cavity = "";
    options_marker.push_back(0);
  }
  else
  {
    options_marker.push_back(1);
  }
  options.push_back(cavity);

  if (data.find_keyword("CAVITY_RADIATION_ON")){
      options_marker.push_back(1);
      options.push_back("1");
  }else if (data.find_keyword("CAVITY_RADIATION_OFF"))
  {
    options_marker.push_back(1);
    options.push_back("0");
  }else{
      options_marker.push_back(0);
      options.push_back("0");
  }

  if (!ccx_iface.modify_loadsradiation(radiation_id ,options , options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}