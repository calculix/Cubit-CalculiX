#include "ccxLoadsCentrifugalCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsCentrifugalCreateCommand::ccxLoadsCentrifugalCreateCommand()
{}

ccxLoadsCentrifugalCreateCommand::~ccxLoadsCentrifugalCreateCommand()
{}

std::vector<std::string> ccxLoadsCentrifugalCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create centrifugal <value:label='magnitude_value',help='<magnitude_value>'> ");
  syntax.append("block <value:label='block id',help='<block id>'> ");
  syntax.append("coordinate <value:label='cx_value',help='<x_value>'> ");
  syntax.append("<value:label='cy_value',help='<y_value>'> ");
  syntax.append("<value:label='cz_value',help='<z_value>'> ");
  syntax.append("direction <value:label='x_value',help='<x_value>'> ");
  syntax.append("<value:label='y_value',help='<y_value>'> ");
  syntax.append("<value:label='z_value',help='<z_value>'> ");
  syntax.append("[op {mod | new}] " );
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );

  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxLoadsCentrifugalCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx "; 
  help[0].append("create centrifugal <magnitude_value> ");
  help[0].append("block <block id> ");
  help[0].append("coordinate <x> <y> <z> ");
  help[0].append("direction <x> <y> <z> ");
  help[0].append("[op {mod | new}] " );
  help[0].append("[amplitude <amplitude id>] ");
  help[0].append("[timedelay <timedelay>] ");
  help[0].append("[name <name>] " );

  return help;
}

std::vector<std::string> ccxLoadsCentrifugalCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsCentrifugalCreateCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::string block_id;
  int block_id_value;
  int op_mode = 0;
  std::string amplitude_id;
  int amplitude_value;
  std::string timedelay;
  double timedelay_value;
  double x_value;
  double y_value;
  double z_value;
  std::string x;
  std::string y;
  std::string z;
  double magnitude_value;
  std::string magnitude;
  double cx_value;
  double cy_value;
  double cz_value;
  std::string cx;
  std::string cy;
  std::string cz;
  std::string name = ""; 
  
  data.get_value("magnitude_value", magnitude_value);
  magnitude = ccx_iface.to_string_scientific(magnitude_value);
  data.get_value("block id", block_id_value);
  block_id = std::to_string(block_id_value);
  data.get_value("x_value", x_value);
  x = ccx_iface.to_string_scientific(x_value);
  data.get_value("y_value", y_value);
  y = ccx_iface.to_string_scientific(y_value);
  data.get_value("z_value", z_value);
  z = ccx_iface.to_string_scientific(z_value);
  data.get_value("cx_value", cx_value);
  cx = ccx_iface.to_string_scientific(cx_value);
  data.get_value("cy_value", cy_value);
  cy = ccx_iface.to_string_scientific(cy_value);
  data.get_value("cz_value", cz_value);
  cz = ccx_iface.to_string_scientific(cz_value);

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
    timedelay = std::to_string(timedelay_value);
  }
  data.get_string("name", name);
  
  options.push_back(std::to_string(op_mode));
  options.push_back(amplitude_id);
  options.push_back(timedelay);
  options.push_back(block_id);
  options.push_back(x);
  options.push_back(y);
  options.push_back(z);
  options.push_back(magnitude);
  options.push_back(cx);
  options.push_back(cy);
  options.push_back(cz);
  options.push_back(name);
  
  if (!ccx_iface.create_loadscentrifugal(options))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}