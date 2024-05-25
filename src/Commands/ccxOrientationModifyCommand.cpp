#include "ccxOrientationModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxOrientationModifyCommand::ccxOrientationModifyCommand()
{}

ccxOrientationModifyCommand::~ccxOrientationModifyCommand()
{}

std::vector<std::string> ccxOrientationModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify orientation <value:label='orientation id',help='<orientation id>'>");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[{system_type_r|system_type_c}] ");
  syntax.append("[a_coord <value:label='a_coord',help='<a_coord>'>...] " );
  syntax.append("[b_coord <value:label='b_coord',help='<b_coord>'>...] " );
  syntax.append("[{local_axis_x|local_axis_y|local_axis_z} angle <value:label='angle',help='<angle>'>] ");
  

  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxOrientationModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify orientation <orientation id> [name <name>] [{system_type_r|system_type_c}] [a_coord <x,y,z>...] [b_coord <x,y,z>...] [{local_axis_x|local_axis_y|local_axis_z} angle <angle>]"; 

  return help;
}

std::vector<std::string> ccxOrientationModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxOrientationModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;
/*
  std::string output;
  std::vector<std::string> options;  
  std::vector<int> options_marker;
  std::vector<std::vector<std::string>> options2;
  int orientation_id;
  double shiftx_value;
  double shifty_value;
  std::string name;
  std::string shiftx;
  std::string shifty;
  std::vector<double> orientationvalues;

  data.get_value("orientation id", orientation_id);

  data.get_values("time_orientation", orientationvalues);
  
  if (orientationvalues.size() % 2 != 0)
  {
    output = "Failed! numbers count is not even!\n";
    PRINT_ERROR(output.c_str());
    return false;
  }else{
    for (size_t i = 0; i < orientationvalues.size(); i+=2)
    {
      std::vector<std::string> orientation(2);
      orientation[0] = ccx_iface.to_string_scientific(orientationvalues[i]);
      orientation[1] = ccx_iface.to_string_scientific(orientationvalues[i+1]);
      options2.push_back(orientation);
      orientation.clear();
    }
  }
  
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


  if (!data.get_value("shiftx", shiftx_value))
  {
    shiftx = "";
    options_marker.push_back(0);
  }
  else
  {
    shiftx = ccx_iface.to_string_scientific(shiftx_value);
    options_marker.push_back(1);
  }
  options.push_back(shiftx);

  if (!data.get_value("shifty", shifty_value))
  {
    shifty = "";
    options_marker.push_back(0);
  }
  else
  {
    shifty = ccx_iface.to_string_scientific(shifty_value);
    options_marker.push_back(1);
  }
  options.push_back(shifty);

  if (data.find_keyword("TOTALTIME_YES"))
  {
    options.push_back("1");
    options_marker.push_back(1);
  }else if (data.find_keyword("TOTALTIME_NO"))
  {
    options.push_back("-1");
    options_marker.push_back(1);
  }else
  {
    options.push_back("-1");
    options_marker.push_back(0);
  }
  
  if (!ccx_iface.modify_orientation(orientation_id, options, options_marker,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    */
  return true;
}