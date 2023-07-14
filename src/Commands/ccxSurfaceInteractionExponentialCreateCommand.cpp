#include "ccxSurfaceInteractionExponentialCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionExponentialCreateCommand::ccxSurfaceInteractionExponentialCreateCommand()
{}

ccxSurfaceInteractionExponentialCreateCommand::~ccxSurfaceInteractionExponentialCreateCommand()
{}

std::vector<std::string> ccxSurfaceInteractionExponentialCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("create surfaceinteraction ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("exponential " );
  syntax.append("c0 <value:label='c0',help='<c0>'> " );
  syntax.append("p0 <value:label='p0',help='<p0>'> " );
  
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionExponentialCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create surfaceinteraction name <name> exponential c0 <c0> p0 <p0>"; 

  return help;
}

std::vector<std::string> ccxSurfaceInteractionExponentialCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionExponentialCreateCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string name;
  std::vector<std::string> options;
  std::vector<std::vector<std::string>> options2;
  double c0_value;
  double p0_value;
  std::string c0;
  std::string p0;

  data.get_string("name", name);
  data.get_value("c0", c0_value);
  data.get_value("p0", p0_value);
  c0 = ccx_iface.to_string_scientific(c0_value);
  p0 = ccx_iface.to_string_scientific(p0_value);

  options.push_back(name);
  options.push_back(c0);
  options.push_back(p0);
  
  if (!ccx_iface.create_surfaceinteraction("EXPONENTIAL", options, options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}