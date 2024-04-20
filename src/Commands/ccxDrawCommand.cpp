#include "ccxDrawCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxDrawCommand::ccxDrawCommand()
{}

ccxDrawCommand::~ccxDrawCommand()
{}

std::vector<std::string> ccxDrawCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("draw");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxDrawCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx draw"; 

  return help;
}

std::vector<std::string> ccxDrawCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxDrawCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  if (!ccx_iface.draw_all())
  {
    output = "Failed ccx draw!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}