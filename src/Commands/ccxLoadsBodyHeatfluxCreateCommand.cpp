#include "ccxLoadsBodyHeatfluxCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsBodyHeatfluxCreateCommand::ccxLoadsBodyHeatfluxCreateCommand()
{}

ccxLoadsBodyHeatfluxCreateCommand::~ccxLoadsBodyHeatfluxCreateCommand()
{}

std::vector<std::string> ccxLoadsBodyHeatfluxCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create bodyheatflux <value:label='magnitude_value',help='<magnitude_value>'> ");
  syntax.append("{block|element} <value:label='element_ids',help='<element_ids>'>... ");
  syntax.append("[op {mod | new}] " );
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxLoadsBodyHeatfluxCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx "; 
  help[0].append("create bodyheatflux <magnitude_value> ");
  help[0].append("{block|element} <ids>... ");
  help[0].append("[op {mod | new}] " );
  help[0].append("[amplitude <amplitude id>] ");
  help[0].append("[timedelay <timedelay>] ");
  help[0].append("[name <name>] " );

  return help;
}

std::vector<std::string> ccxLoadsBodyHeatfluxCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsBodyHeatfluxCreateCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> element_ids;
  std::string element_type = "0"; // 1 block | 2 global element
  int op_mode = 0;
  std::string amplitude_id;
  int amplitude_value;
  std::string timedelay;
  double timedelay_value;
  double magnitude_value;
  std::string magnitude;
  std::string name = ""; 
  
  data.get_value("magnitude_value", magnitude_value);
  magnitude = ccx_iface.to_string_scientific(magnitude_value);
  data.get_values("element_ids", element_ids);
  
  if (data.find_keyword("OP")){
    if (data.find_keyword("MOD")){
      op_mode = 0;
    }else if (data.find_keyword("NEW"))
    {
      op_mode = 1;
    }
  }
  
  if (data.find_keyword("BLOCK")){
    element_type = "1";
  }else if (data.find_keyword("ELEMENT"))
  {
    element_type = "2";
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
    timedelay = std::to_string(timedelay_value);
  }
  data.get_string("name", name);
  
  options.push_back(std::to_string(op_mode));
  options.push_back(amplitude_id);
  options.push_back(timedelay);
  options.push_back(element_type);
  options.push_back(magnitude);
  options.push_back(name);
  
  if (!ccx_iface.create_loadsbodyheatflux(options, element_ids))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}