#include "ccxAmplitudeModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxAmplitudeModifyCommand::ccxAmplitudeModifyCommand()
{}

ccxAmplitudeModifyCommand::~ccxAmplitudeModifyCommand()
{}

std::vector<std::string> ccxAmplitudeModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify amplitude <value:label='amplitude id',help='<amplitude id>'>");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[amplitudevalues <value:label='amplitudevalues',help='<amplitudevalues>'>...] " );
  syntax.append("[shiftx <value:label='shiftx',help='<shiftx>'>] ");
  syntax.append("[shifty <value:label='shifty',help='<shifty>'>] ");
  syntax.append("[totaltime] ");
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxAmplitudeModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify amplitude <amplitude id> [name <name>] [amplitudevalues <amplitudevalues>...] [shiftx <value>] [shifty <value>] [totaltime]"; 

  return help;
}

std::vector<std::string> ccxAmplitudeModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxAmplitudeModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  std::vector<int> options_marker;
  std::vector<std::vector<std::string>> options2;
  int amplitude_id;
  double shiftx_value;
  double shifty_value;
  std::string name;
  std::string shiftx;
  std::string shifty;
  std::vector<double> amplitudevalues;

  data.get_value("amplitude id", amplitude_id);

  data.get_values("amplitudevalues", amplitudevalues);
  
  if (amplitudevalues.size() % 2 != 0)
  {
    output = "Failed! numbers count is not even!\n";
    PRINT_ERROR(output.c_str());
    return false;
  }else{
    for (size_t i = 0; i < amplitudevalues.size(); i+=2)
    {
      std::vector<std::string> amplitude(2);
      amplitude[0] = std::to_string(amplitudevalues[i]);
      amplitude[1] = std::to_string(amplitudevalues[i+1]);
      options2.push_back(amplitude);
      amplitude.clear();
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
    shiftx = std::to_string(shiftx_value);
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
    shifty = std::to_string(shifty_value);
    options_marker.push_back(1);
  }
  options.push_back(shifty);

  if (data.find_keyword("TOTALTIME"))
  {
    options.push_back("1");
    options_marker.push_back(1);
  } else
  {
    options.push_back("-1");
    options_marker.push_back(0);
  }
  
  if (!ccx_iface.modify_amplitude(amplitude_id, options, options_marker,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}