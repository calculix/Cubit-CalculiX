#include "ccxJobModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxJobModifyCommand::ccxJobModifyCommand()
{}

ccxJobModifyCommand::~ccxJobModifyCommand()
{}

std::vector<std::string> ccxJobModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify job <value:label='job_id',help='<job_id>'>");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[filepath <string:type='unquoted', number='1', label='filepath', help='<filepath>'>] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxJobModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx modify job <job_id> [name <name>] [filepath <filepath>]"; 

  return help;
}

std::vector<std::string> ccxJobModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxJobModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  std::string name;
  std::string filepath;
  std::vector<int> options_marker;
  int job_id;

  data.get_value("job_id", job_id);

  if (data.find_keyword("NAME"))
  {
    data.get_string("name", name);
    options.push_back(name);
    options_marker.push_back(1);
  }else{
    options.push_back("");
    options_marker.push_back(0);
  }

  //filepath
  if (data.find_keyword("FILEPATH"))
  {
    data.get_string("filepath", filepath);
    options.push_back(filepath);
    options_marker.push_back(1);
  }else{
    options.push_back("");
    options_marker.push_back(0);
  }

  if (!ccx_iface.modify_job(job_id, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}