#include "ccxLoadsTrajectoryModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsTrajectoryModifyCommand::ccxLoadsTrajectoryModifyCommand()
{}

ccxLoadsTrajectoryModifyCommand::~ccxLoadsTrajectoryModifyCommand()
{}

std::vector<std::string> ccxLoadsTrajectoryModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify trajectory <value:label='trajectory id',help='<trajectory id>'>");
  syntax.append("[curve <value:label='curve_id',help='<curve_id>'>] ");
  syntax.append("[vertex <value:label='vertex_id',help='<vertex_id>'>] ");
  syntax.append("[surface <value:label='surface_id',help='<surface_id>'>...] ");
  syntax.append("[direction <value:label='x_value',help='<x_value>'> ");
  syntax.append("<value:label='y_value',help='<y_value>'> ");
  syntax.append("<value:label='z_value',help='<z_value>'>] ");
  syntax.append("[magnitude <value:label='magnitude_value',help='<magnitude_value>'>] ");
  syntax.append("[time_begin <value:label='time_begin_value',help='<time_begin_value>'>] ");
  syntax.append("[time_end <value:label='time_end_value',help='<time_end_value>'>] ");
  syntax.append("[op {mod | new}] " );
  


  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxLoadsTrajectoryModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx "; 
  help[0].append("modify trajectory <trajectory_id> ");
  help[0].append("[curve <curve_id>] ");
  help[0].append("[vertex <vertex_id>] ");
  help[0].append("[surface <surface_id>...] ");
  help[0].append("[direction <x_value> ");
  help[0].append("<y_value> ");
  help[0].append("<z_value>] ");
  help[0].append("[magnitude <magnitude_value>] ");
  help[0].append("[time_begin <time_begin_value>] ");
  help[0].append("[time_end <time_end_value>] ");
  help[0].append("[op {mod | new}] " );

  return help;
}

std::vector<std::string> ccxLoadsTrajectoryModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsTrajectoryModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<int> options_marker;
  std::vector<std::string> options;
  std::vector<int> options2;
  int trajectory_id;
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
  
  data.get_value("trajectory id", trajectory_id);
  
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

  if (!data.get_value("curve_id", curve_id_value))
  {
    curve_id = "-1";
    options_marker.push_back(0);
  }
  else
  {
    curve_id = std::to_string(curve_id_value);
    options_marker.push_back(1);
  }
  options.push_back(curve_id);

  if (!data.get_value("vertex_id", vertex_id_value))
  {
    vertex_id = "-1";
    options_marker.push_back(0);
  }
  else
  {
    vertex_id = std::to_string(vertex_id_value);
    options_marker.push_back(1);
  }
  options.push_back(vertex_id);

  if (!data.get_values("surface_id", surface_id_values))
  {
    options_marker.push_back(0);
  }
  else
  {
    options2 = surface_id_values;
    options_marker.push_back(1);
  }
  options.push_back("");

  if (!data.get_value("x_value", x_value))
  {
    x = "-1";
    options_marker.push_back(0);
  }
  else
  {
    x = ccx_iface.to_string_scientific(x_value);
    options_marker.push_back(1);
  }
  options.push_back(x);

  if (!data.get_value("y_value", y_value))
  {
    y = "-1";
    options_marker.push_back(0);
  }
  else
  {
    y = ccx_iface.to_string_scientific(y_value);
    options_marker.push_back(1);
  }
  options.push_back(y);

  if (!data.get_value("z_value", z_value))
  {
    z = "-1";
    options_marker.push_back(0);
  }
  else
  {
    z = ccx_iface.to_string_scientific(z_value);
    options_marker.push_back(1);
  }
  options.push_back(z);

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

  if (!data.get_value("time_begin_value", time_begin_value))
  {
    time_begin = "-1";
    options_marker.push_back(0);
  }
  else
  {
    time_begin = ccx_iface.to_string_scientific(time_begin_value);
    options_marker.push_back(1);
  }
  options.push_back(time_begin);

  if (!data.get_value("time_end_value", time_end_value))
  {
    time_end = "-1";
    options_marker.push_back(0);
  }
  else
  {
    time_end = ccx_iface.to_string_scientific(time_end_value);
    options_marker.push_back(1);
  }
  options.push_back(time_end);

  if (!ccx_iface.modify_loadstrajectory(trajectory_id, options, options_marker, options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}