#include "ccxPhysicalConstantsModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxPhysicalConstantsModifyCommand::ccxPhysicalConstantsModifyCommand()
{}

ccxPhysicalConstantsModifyCommand::~ccxPhysicalConstantsModifyCommand()
{}

std::vector<std::string> ccxPhysicalConstantsModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify physicalconstants ");
  syntax.append("[absolute_zero <value:label='absolute_zero',help='<absolute_zero>'>] ");
  syntax.append("[stefan_boltzmann <value:label='stefan_boltzmann',help='<stefan_boltzmann>'>] ");
  syntax.append("[newton_gravity <value:label='newton_gravity',help='<newton_gravity>'>] ");
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxPhysicalConstantsModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify physicalconstants ");
  help[0].append("[absolute_zero <absolute_zero>] ");
  help[0].append("[stefan_boltzmann <stefan_boltzmann>] ");
  help[0].append("[newton_gravity <newton_gravity>] ");
  
  return help;
}

std::vector<std::string> ccxPhysicalConstantsModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxPhysicalConstantsModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  std::vector<int> options_marker;
  double absolute_zero_value;
  std::string absolute_zero;
  double stefan_boltzmann_value;
  std::string stefan_boltzmann;
  double newton_gravity_value;
  std::string newton_gravity;
  
  if (!data.get_value("absolute_zero", absolute_zero_value))
  {
    options.push_back("");
    options_marker.push_back(0);
  }else{
    absolute_zero = ccx_iface.to_string_scientific(absolute_zero_value);
    options.push_back(absolute_zero);
    options_marker.push_back(1);
  }

  if (!data.get_value("stefan_boltzmann", stefan_boltzmann_value))
  {
    options.push_back("");
    options_marker.push_back(0);
  }else{
    stefan_boltzmann = ccx_iface.to_string_scientific(stefan_boltzmann_value);
    options.push_back(stefan_boltzmann);
    options_marker.push_back(1);
  }
  
  if (!data.get_value("newton_gravity", newton_gravity_value))
  {
    options.push_back("");
    options_marker.push_back(0);
  }else{
    newton_gravity = ccx_iface.to_string_scientific(newton_gravity_value);
    options.push_back(newton_gravity);
    options_marker.push_back(1);
  }

  if (!ccx_iface.modify_physicalconstants(options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}