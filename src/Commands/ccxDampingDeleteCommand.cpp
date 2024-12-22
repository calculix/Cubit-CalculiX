#include "ccxDampingDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxDampingDeleteCommand::ccxDampingDeleteCommand()
{}

ccxDampingDeleteCommand::~ccxDampingDeleteCommand()
{}

std::vector<std::string> ccxDampingDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("delete damping [alpha] [beta] ");
  
  syntax_list.push_back(syntax); 

  return syntax_list;
}

std::vector<std::string> ccxDampingDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx delete damping [alpha] [beta]";
  
  return help;
}

std::vector<std::string> ccxDampingDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxDampingDeleteCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;
  std::string output;
  bool alpha=false;
  bool beta=false;

  if (data.find_keyword("ALPHA"))
  {
    alpha = true;
  }

  if (data.find_keyword("BETA"))
  {
    beta = true;
  }
   
  if (!ccx_iface.delete_damping(alpha, beta))
  {
    //PRINT_INFO("%s", output.c_str());  
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  } 
  
  return true;
}