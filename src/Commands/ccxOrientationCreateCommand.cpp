#include "ccxOrientationCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxOrientationCreateCommand::ccxOrientationCreateCommand()
{}

ccxOrientationCreateCommand::~ccxOrientationCreateCommand()
{}

std::vector<std::string> ccxOrientationCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create orientation ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("{system_type_r|system_type_c} ");
  syntax.append("a_coord <value:label='a_coord',help='<a_coord>'>... " );
  syntax.append("b_coord <value:label='b_coord',help='<b_coord>'>... " );
  syntax.append("[{local_axis_x|local_axis_y|local_axis_z} angle <value:label='angle',help='<angle>'>] ");
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxOrientationCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create orientation name <name> {system_type_r|system_type_c} a_coord <x,y,z>... b_coord <x,y,z>... [{local_axis_x|local_axis_y|local_axis_z} angle <angle>]"; 

  return help;
}

std::vector<std::string> ccxOrientationCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxOrientationCreateCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;
  /*
  std::string output;
  std::vector<std::string> options;  
  std::vector<std::vector<std::string>> options2;
  double shiftx_value;
  double shifty_value;
  std::string name;
  std::string shiftx;
  std::string shifty;
  std::vector<double> amplitudevalues;

  data.get_values("time_amplitude", amplitudevalues);
  
  if (amplitudevalues.size() % 2 != 0)
  {
    output = "Failed! numbers count is not even!\n";
    PRINT_ERROR(output.c_str());
    return false;
  }else{
    for (size_t i = 0; i < amplitudevalues.size(); i+=2)
    {
      std::vector<std::string> amplitude(2);
      amplitude[0] = ccx_iface.to_string_scientific(amplitudevalues[i]);
      amplitude[1] = ccx_iface.to_string_scientific(amplitudevalues[i+1]);
      options2.push_back(amplitude);
      amplitude.clear();
    }
  }

  data.get_string("name", name);
  options.push_back(name);

  if (!data.get_value("shiftx", shiftx_value))
  {
    shiftx = "";
  }
  else
  {
    shiftx = ccx_iface.to_string_scientific(shiftx_value);
  }
  options.push_back(shiftx);

  if (!data.get_value("shifty", shifty_value))
  {
    shifty = "";
  }
  else
  {
    shifty = ccx_iface.to_string_scientific(shifty_value);
  }
  options.push_back(shifty);

  if (data.find_keyword("TOTALTIME"))
  {
    options.push_back("1");
  } else
  {
    options.push_back("-1");
  }
  
  if (!ccx_iface.create_amplitude(options,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
  */
  return true;
}