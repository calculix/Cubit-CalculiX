#include "ccxSurfaceInteractionLinearCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionLinearCreateCommand::ccxSurfaceInteractionLinearCreateCommand()
{}

ccxSurfaceInteractionLinearCreateCommand::~ccxSurfaceInteractionLinearCreateCommand()
{}

std::vector<std::string> ccxSurfaceInteractionLinearCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("create surfaceinteraction ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("linear " );
  syntax.append("slopeK <value:label='slopeK',help='<slopeK>'> " );
  syntax.append("sigmaINF <value:label='sigmaINF',help='<sigmaINF>'> " );
  syntax.append("c0 <value:label='c0',help='<c0>'> " );
  
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionLinearCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create surfaceinteraction name <name> linear slopeK <slopeK> sigmaINF <sigmaINF> c0 <c0>"; 

  return help;
}

std::vector<std::string> ccxSurfaceInteractionLinearCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionLinearCreateCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string name;
  std::vector<std::string> options;
  std::vector<std::vector<std::string>> options2;
  double slopeK_value;
  double sigmaINF_value;
  double c0_value;
  std::string slopeK;
  std::string sigmaINF;
  std::string c0;

  data.get_string("name", name);
  data.get_value("slopeK", slopeK_value);
  slopeK = std::to_string(slopeK_value);
  data.get_value("sigmaINF", sigmaINF_value);
  sigmaINF = std::to_string(sigmaINF_value);
  data.get_value("c0", c0_value);
  c0 = std::to_string(c0_value);

  options.push_back(name);
  options.push_back(slopeK);
  options.push_back(sigmaINF);
  options.push_back(c0);
  
  if (!ccx_iface.create_surfaceinteraction("LINEAR", options, options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}