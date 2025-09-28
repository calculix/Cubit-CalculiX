#include "ccxLoadsBodyHeatfluxModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsBodyHeatfluxModifyCommand::ccxLoadsBodyHeatfluxModifyCommand()
{}

ccxLoadsBodyHeatfluxModifyCommand::~ccxLoadsBodyHeatfluxModifyCommand()
{}

std::vector<std::string> ccxLoadsBodyHeatfluxModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify bodyheatflux <value:label='bodyheatflux id',help='<bodyheatflux id>'>");
  syntax.append("[magnitude <value:label='magnitude_value',help='<magnitude_value>'>] ");
  syntax.append("[{block|element} <value:label='element_ids',help='<element_ids>'>...] ");
  syntax.append("[op {mod | new}] " );
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxLoadsBodyHeatfluxModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx "; 
  help[0].append("modify bodyheatflux <bodyheatflux_id> [magnitude <magnitude_value>] ");
  help[0].append("[{block|element} <ids>...] ");
  help[0].append("[op {mod | new}] " );
  help[0].append("[amplitude <amplitude id>] ");
  help[0].append("[timedelay <timedelay>] ");
  help[0].append("[name <name>] " );

  return help;
}

std::vector<std::string> ccxLoadsBodyHeatfluxModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsBodyHeatfluxModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<int> options_marker;
  std::vector<std::string> options;
  int bodyheatflux_id;
  std::vector<int> element_ids;
  std::string element_type = "0";
  int op_mode = 0;
  std::string amplitude_id;
  int amplitude_value;
  std::string timedelay;
  double timedelay_value;
  double magnitude_value;
  std::string magnitude;
  std::string name; 
  
  data.get_value("bodyheatflux id", bodyheatflux_id);
  
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
  
  if (!data.get_values("element_ids", element_ids))
  {
    options_marker.push_back(0);
  }
  else
  {
    options_marker.push_back(1);
  }

  
  if (data.find_keyword("BLOCK")){
    element_type = "1";
    options_marker.push_back(1);
  }else if (data.find_keyword("ELEMENT"))
  {
    element_type = "2";
    options_marker.push_back(1);
  }else{
    options_marker.push_back(0);
  }
  options.push_back(element_type);
  
  if (!data.get_value("magnitude_value", magnitude_value))
  {
    magnitude = "-1";
    options_marker.push_back(0);
  }
  else
  {
    magnitude = ccx_iface.to_string_scientific(magnitude_value);
    options_marker.push_back(1);
  }
  options.push_back(magnitude);
  
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
  
  if (!ccx_iface.modify_loadsbodyheatflux(bodyheatflux_id ,options , options_marker, element_ids))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}