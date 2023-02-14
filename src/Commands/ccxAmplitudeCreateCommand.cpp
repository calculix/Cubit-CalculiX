#include "ccxAmplitudeCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxAmplitudeCreateCommand::ccxAmplitudeCreateCommand()
{}

ccxAmplitudeCreateCommand::~ccxAmplitudeCreateCommand()
{}

std::vector<std::string> ccxAmplitudeCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create amplitude ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("time_amplitude <value:label='time_amplitude',help='<time_amplitude>'>... " );
  syntax.append("[shiftx <value:label='shiftx',help='<shiftx>'>] ");
  syntax.append("[shifty <value:label='shifty',help='<shifty>'>] ");
  syntax.append("[totaltime] ");
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxAmplitudeCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create amplitude name <name> time_amplitude <time_amplitude>... [shiftx <value>] [shifty <value>] [totaltime]"; 

  return help;
}

std::vector<std::string> ccxAmplitudeCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxAmplitudeCreateCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

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
      amplitude[0] = std::to_string(amplitudevalues[i]);
      amplitude[1] = std::to_string(amplitudevalues[i+1]);
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
    shiftx = std::to_string(shiftx_value);
  }
  options.push_back(shiftx);

  if (!data.get_value("shifty", shifty_value))
  {
    shifty = "";
  }
  else
  {
    shifty = std::to_string(shifty_value);
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
    
  return true;
}