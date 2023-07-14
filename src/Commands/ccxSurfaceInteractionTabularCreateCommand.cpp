#include "ccxSurfaceInteractionTabularCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionTabularCreateCommand::ccxSurfaceInteractionTabularCreateCommand()
{}

ccxSurfaceInteractionTabularCreateCommand::~ccxSurfaceInteractionTabularCreateCommand()
{}

std::vector<std::string> ccxSurfaceInteractionTabularCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("create surfaceinteraction ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("tabular " );
  syntax.append("pressure_overclosure <value:label='pressure_overclosure',help='<pressure_overclosure>'>... " );
  
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionTabularCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create surfaceinteraction name <name> tabular pressure_overclosure <values>..."; 

  return help;
}

std::vector<std::string> ccxSurfaceInteractionTabularCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionTabularCreateCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string name;
  std::vector<std::string> options;
  std::vector<std::vector<std::string>> options2;
  std::vector<double> pressure_overclosure_values;

  data.get_string("name", name);
  data.get_values("pressure_overclosure", pressure_overclosure_values);
  
  if (pressure_overclosure_values.size() % 2 != 0)
  {
    output = "Failed! numbers count is not even!\n";
    PRINT_ERROR(output.c_str());
    return false;
  }else{
    for (size_t i = 0; i < pressure_overclosure_values.size(); i+=2)
    {
      std::vector<std::string> pressure_overclosure(2);
      pressure_overclosure[0] = ccx_iface.to_string_scientific(pressure_overclosure_values[i]);
      pressure_overclosure[1] = ccx_iface.to_string_scientific(pressure_overclosure_values[i+1]);
      options2.push_back(pressure_overclosure);
      pressure_overclosure.clear();
    }
  }
  
  options.push_back(name);
   
  if (!ccx_iface.create_surfaceinteraction("TABULAR", options, options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
    return false;
  }
  options.clear();
  options2.clear();

  return true;
}