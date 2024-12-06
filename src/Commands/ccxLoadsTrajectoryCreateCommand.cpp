#include "ccxLoadsTrajectoryCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsTrajectoryCreateCommand::ccxLoadsTrajectoryCreateCommand()
{}

ccxLoadsTrajectoryCreateCommand::~ccxLoadsTrajectoryCreateCommand()
{}

std::vector<std::string> ccxLoadsTrajectoryCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create trajectory ");
  syntax.append("curve <value:label='curve_id',help='<curve_id>'> ");
  syntax.append("vertex <value:label='vertex_id',help='<vertex_id>'> ");
  syntax.append("surface <value:label='surface_id',help='<surface_id>'>... ");
  syntax.append("direction <value:label='x_value',help='<x_value>'> ");
  syntax.append("<value:label='y_value',help='<y_value>'> ");
  syntax.append("<value:label='z_value',help='<z_value>'> ");
  syntax.append("magnitude <value:label='magnitude_value',help='<magnitude_value>'> ");
  syntax.append("time_begin <value:label='time_begin_value',help='<time_begin_value>'> ");
  syntax.append("time_end <value:label='time_end_value',help='<time_end_value>'> ");
  syntax.append("radius <value:label='radius_value',help='<radius_value>'> ");
  syntax.append("[op {mod | new}] " );
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxLoadsTrajectoryCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx "; 
  help[0].append("create trajectory ");
  help[0].append("curve <curve_id> ");
  help[0].append("vertex <vertex_id> ");
  help[0].append("surface <surface_id>... ");
  help[0].append("direction <x_value> ");
  help[0].append("<y_value> ");
  help[0].append("<z_value> ");
  help[0].append("magnitude <magnitude_value> ");
  help[0].append("time_begin <time_begin_value> ");
  help[0].append("time_end <time_end_value> ");
  help[0].append("radius <radius_value> ");
  help[0].append("[op {mod | new}] " );

  return help;
}

std::vector<std::string> ccxLoadsTrajectoryCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsTrajectoryCreateCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> options2;
  std::string curve_id;
  int curve_id_value;
  std::string vertex_id;
  int vertex_id_value;
  std::vector<int> surface_id_values;
  int op_mode = 0;
  std::string time_begin;
  double time_begin_value;
  std::string time_end;
  double time_end_value;
  double x_value;
  double y_value;
  double z_value;
  std::string x;
  std::string y;
  std::string z;
  double magnitude_value;
  std::string magnitude;
  std::string radius;
  double radius_value;
  
  
  data.get_value("curve_id", curve_id_value);
  curve_id = std::to_string(curve_id_value);
  data.get_value("vertex_id", vertex_id_value);
  vertex_id = std::to_string(vertex_id_value);
  data.get_values("surface_id", surface_id_values);
  data.get_value("magnitude_value", magnitude_value);
  magnitude = ccx_iface.to_string_scientific(magnitude_value);
  data.get_value("x_value", x_value);
  x = ccx_iface.to_string_scientific(x_value);
  data.get_value("y_value", y_value);
  y = ccx_iface.to_string_scientific(y_value);
  data.get_value("z_value", z_value);
  z = ccx_iface.to_string_scientific(z_value);
  data.get_value("time_begin_value", time_begin_value);
  time_begin = ccx_iface.to_string_scientific(time_begin_value);
  data.get_value("time_end_value", time_end_value);
  time_end = ccx_iface.to_string_scientific(time_end_value);

  if (data.find_keyword("OP")){
    if (data.find_keyword("MOD")){
      op_mode = 0;
    }else if (data.find_keyword("NEW"))
    {
      op_mode = 1;
    }
  }

  data.get_value("radius_value", radius_value);
  radius = ccx_iface.to_string_scientific(radius_value);

  options.push_back(std::to_string(op_mode));
  options.push_back(curve_id);
  options.push_back(vertex_id);
  options.push_back(x);
  options.push_back(y);
  options.push_back(z);
  options.push_back(magnitude);
  options.push_back(time_begin);
  options.push_back(time_end);
  options.push_back(radius);

  options2 = surface_id_values;
  
  if (!ccx_iface.create_loadstrajectory(options,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}