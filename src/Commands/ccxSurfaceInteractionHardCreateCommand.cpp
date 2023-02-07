#include "ccxSurfaceInteractionHardCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionHardCreateCommand::ccxSurfaceInteractionHardCreateCommand()
{}

ccxSurfaceInteractionHardCreateCommand::~ccxSurfaceInteractionHardCreateCommand()
{}

std::vector<std::string> ccxSurfaceInteractionHardCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("create surfaceinteraction ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("hard" );
  
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionHardCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create surfaceinteraction name <name> hard"; 

  return help;
}

std::vector<std::string> ccxSurfaceInteractionHardCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionHardCreateCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string name;
  std::vector<std::string> options;
  std::vector<std::vector<std::string>> options2;
  
  data.get_string("name", name);
  
  options.push_back(name);
  
  if (!ccx_iface.create_surfaceinteraction("HARD", options, options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}