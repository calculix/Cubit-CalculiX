#include "ccxLoadsRadiationCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsRadiationCreateCommand::ccxLoadsRadiationCreateCommand()
{}

ccxLoadsRadiationCreateCommand::~ccxLoadsRadiationCreateCommand()
{}

std::vector<std::string> ccxLoadsRadiationCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create radiation ");
  syntax.append("sideset <value:label='sideset id',help='<sideset id>'> ");
  syntax.append("temperature <value:label='temperature_value',help='<temperature_value>'> ");
  syntax.append("emissivity <value:label='emissivity_value',help='<emissivity_value>'> ");
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

std::vector<std::string> ccxLoadsRadiationCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx "; 
  help[0].append("create radiation ");
  help[0].append("sideset <sideset id> ");
  help[0].append("temperature <temperature_value> ");
  help[0].append("emissivity <emissivity_value> ");
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

std::vector<std::string> ccxLoadsRadiationCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsRadiationCreateCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
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
  std::string name = ""; 
  std::string cavity = "";
  std::string cavity_radiation = "0"; 
  
  
  data.get_value("sideset id", sideset_id_value);
  sideset_id = std::to_string(sideset_id_value);
  data.get_value("temperature_value", temperature_value);
  temperature = ccx_iface.to_string_scientific(temperature_value);
  data.get_value("emissivity_value", emissivity_value);
  emissivity = ccx_iface.to_string_scientific(emissivity_value);
  
  if (data.find_keyword("OP")){
    if (data.find_keyword("MOD")){
      op_mode = 0;
    }else if (data.find_keyword("NEW"))
    {
      op_mode = 1;
    }
  }

  if (!data.get_value("amplitude id", amplitude_value))
  {
    amplitude_id = "-1";
  }
  else
  {
    amplitude_id = std::to_string(amplitude_value);
  }
  
  if (!data.get_value("timedelay", timedelay_value))
  {
    timedelay = "";
  }
  else
  {
    timedelay = ccx_iface.to_string_scientific(timedelay_value);
  }
  
  if (!data.get_value("radiationamplitude id", radiationamplitude_value))
  {
    radiationamplitude_id = "-1";
  }
  else
  {
    radiationamplitude_id = std::to_string(radiationamplitude_value);
  }
  
  if (!data.get_value("radiationtimedelay", radiationtimedelay_value))
  {
    radiationtimedelay = "";
  }
  else
  {
    radiationtimedelay = ccx_iface.to_string_scientific(radiationtimedelay_value);
  }
  data.get_string("name", name);
  data.get_string("cavity", cavity);

  if (data.find_keyword("CAVITY_RADIATION_ON")){
    cavity_radiation = "1";
  }

  options.push_back(std::to_string(op_mode));
  options.push_back(amplitude_id);
  options.push_back(timedelay);
  options.push_back(sideset_id);
  options.push_back(temperature);
  options.push_back(emissivity);
  options.push_back(radiationamplitude_id);
  options.push_back(radiationtimedelay);
  options.push_back(name);
  options.push_back(cavity);
  options.push_back(cavity_radiation);
  
  if (!ccx_iface.create_loadsradiation(options))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}