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

  std::string output;
  std::vector<std::string> options;  
  std::vector<std::vector<std::string>> options2;
  std::vector<int> options_marker;
  int orientation_id;
  std::string name;
  int system_type;
  std::vector<std::string> dummy;
  std::vector<double> a_values;
  std::vector<double> b_values;
  double angle_value;

  data.get_value("orientation id", orientation_id);

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
  options2.push_back(dummy);

  if (data.find_keyword("SYSTEM_TYPE_R"))
  {
    options.push_back("1");
    options_marker.push_back(1);
  } else if (data.find_keyword("SYSTEM_TYPE_C"))
  {
    options.push_back("2");
    options_marker.push_back(1);
  }else  
  {
    options.push_back("-1");
    options_marker.push_back(0);
  }
  options2.push_back(dummy);

  //distribution dummy
  options.push_back("-1");
  options2.push_back(dummy);
  options_marker.push_back(0);

  data.get_values("a_coord", a_values);
  data.get_values("b_coord", b_values);
  
  if (a_values.size()!=0)
  {
    if (a_values.size() != 3)
    {
      output = "Failed! a_coord needs 3 values!\n";
      PRINT_ERROR(output.c_str());
      return false;
    }else{
      std::vector<std::string> a_coord(3);
      a_coord[0] = ccx_iface.to_string_scientific(a_values[0]);
      a_coord[1] = ccx_iface.to_string_scientific(a_values[1]);
      a_coord[2] = ccx_iface.to_string_scientific(a_values[2]);
      options2.push_back(a_coord);
      options.push_back("");
      options_marker.push_back(1);
    }
  }else{
    options.push_back("-1");
    options2.push_back(dummy);
    options_marker.push_back(0);
  }
  
  if (b_values.size()!=0)
  {
    if (b_values.size() != 3)
    {
      output = "Failed! b_coord needs 3 values!\n";
      PRINT_ERROR(output.c_str());
      return false;
    }else{
      std::vector<std::string> b_coord(3);
      b_coord[0] = ccx_iface.to_string_scientific(b_values[0]);
      b_coord[1] = ccx_iface.to_string_scientific(b_values[1]);
      b_coord[2] = ccx_iface.to_string_scientific(b_values[2]);
      options2.push_back(b_coord);
      options.push_back("");
      options_marker.push_back(1);
    }
  }else{
    options.push_back("-1");
    options2.push_back(dummy);
    options_marker.push_back(0);
  }

  if (data.find_keyword("LOCAL_AXIS_X"))
  {
    options.push_back("1");
    options_marker.push_back(1);
  } else if (data.find_keyword("LOCAL_AXIS_Y"))
  {
    options.push_back("2");
    options_marker.push_back(1);
  } else if (data.find_keyword("LOCAL_AXIS_Z"))
  {
    options.push_back("3");
    options_marker.push_back(1);
  }else  
  {
    options.push_back("-1");
    options_marker.push_back(0);
  }
  std::vector<std::string> angle(1);
  if (!data.get_value("angle", angle_value))
  {
    angle[0] = "0";
  }
  else
  {
    angle[0] = ccx_iface.to_string_scientific(angle_value);    
  }
  options2.push_back(angle);
  
  if (!ccx_iface.modify_orientation(orientation_id, options, options_marker,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
  options2.clear();
  
  return true;
}