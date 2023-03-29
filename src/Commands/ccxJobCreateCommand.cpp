#include "ccxJobCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxJobCreateCommand::ccxJobCreateCommand()
{}

ccxJobCreateCommand::~ccxJobCreateCommand()
{}

std::vector<std::string> ccxJobCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create job ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  syntax.append("[filepath <string:type='unquoted', number='1', label='filepath', help='<filepath>'>] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxJobCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx create job name <name> [filepath <filepath>]"; 

  return help;
}

std::vector<std::string> ccxJobCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxJobCreateCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  std::string name;
  std::string filepath;

  data.get_string("name", name);
  options.push_back(name);

  //filepath
  if (data.find_keyword("FILEPATH"))
  {
    data.get_string("filepath", filepath);
    options.push_back(filepath);
  }else{
    options.push_back("");
  }
  
  if (!ccx_iface.create_job(options))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}