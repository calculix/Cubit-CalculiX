#include "ccxFieldOutputCreateCommand.hpp"
#include <string>
#include <vector>
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxFieldOutputCreateCommand::ccxFieldOutputCreateCommand()
{}

ccxFieldOutputCreateCommand::~ccxFieldOutputCreateCommand()
{}

std::vector<std::string> ccxFieldOutputCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create fieldoutput ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("{node|element|contact}" );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxFieldOutputCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx create fieldoutput name <name> {node|element|contact}"; 

  return help;
}

std::vector<std::string> ccxFieldOutputCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxFieldOutputCreateCommand::execute(CubitCommandData &data)
{
  CalculiXStepsInterface ccx_steps_iface;


  std::string output;
  std::vector<std::string> options;  
  std::string name;

  data.get_string("name", name);
  options.push_back(name);

  if (data.find_keyword("NODE"))
  {
    options.push_back("1");
  } else if (data.find_keyword("ELEMENT"))
  {
    options.push_back("2");
  } else if (data.find_keyword("CONTACT"))
  {
    options.push_back("3");
  }
  
  if (!ccx_steps_iface.create_fieldoutput(options))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}