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
  syntax.append(" [size <value:label='size_value',help='<size_value>'>]");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxDrawCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx draw [size <size_value>]";

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
  double size_value;
  
  if (!data.get_value("size_value", size_value))
  {
    size_value = 1;
  }
  
  if (!ccx_iface.draw_all(size_value))
  {
    output = "Failed ccx draw!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}