#include "ccxDampingModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxDampingModifyCommand::ccxDampingModifyCommand()
{}

ccxDampingModifyCommand::~ccxDampingModifyCommand()
{}

std::vector<std::string> ccxDampingModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify damping ");
  syntax.append("[alpha <value:label='alpha',help='<alpha>'>] ");
  syntax.append("[beta <value:label='beta',help='<beta>'>] ");
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxDampingModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify damping ");
  help[0].append("[alpha <alpha>] ");
  help[0].append("[beta <beta>] ");

  return help;
}

std::vector<std::string> ccxDampingModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxDampingModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  std::vector<int> options_marker;
  double alpha_value;
  std::string alpha;
  double beta_value;
  std::string beta;
  
  if (!data.get_value("alpha", alpha_value))
  {
    options.push_back("");
    options_marker.push_back(0);
  }else{
    alpha = ccx_iface.to_string_scientific(alpha_value);
    options.push_back(alpha);
    options_marker.push_back(1);
  }

  if (!data.get_value("beta", beta_value))
  {
    options.push_back("");
    options_marker.push_back(0);
  }else{
    beta = ccx_iface.to_string_scientific(beta_value);
    options.push_back(beta);
    options_marker.push_back(1);
  }

  if (!ccx_iface.modify_damping(options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}