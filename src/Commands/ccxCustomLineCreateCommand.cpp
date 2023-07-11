#include "ccxCustomLineCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxCustomLineCreateCommand::ccxCustomLineCreateCommand()
{}

ccxCustomLineCreateCommand::~ccxCustomLineCreateCommand()
{}

std::vector<std::string> ccxCustomLineCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create customline ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax.append("{before|after} ");
  syntax.append("{elset|nset|sideset|material|section|");
  syntax.append("constraint|surfaceinteraction|contactpair|amplitude|");
  syntax.append("force|pressure|heatflux|displacement|temperature|");
  syntax.append("historyoutput|fieldoutput|initialcondition|step_begin|step_end|export} ");
  syntax.append("item_id <value:label='item_id',help='<item_id>'> ");
  syntax.append("cline <string:type='quoted', number='1', label='cline', help='<custom line>'> ");
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxCustomLineCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("create customline ");
  help[0].append("name <name>");
  help[0].append("{before|after} ");
  help[0].append("{elset|nset|sideset|material|section|");
  help[0].append("constraint|surfaceinteraction|contactpair|amplitude|");
  help[0].append("force|pressure|heatflux|displacement|temperature|");
  help[0].append("historyoutput|fieldoutput|initialcondition|step_begin|step_end|export} ");
  help[0].append("item_id <item_id> ");
  help[0].append("cline <custom line> ");

  return help;
}

std::vector<std::string> ccxCustomLineCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxCustomLineCreateCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  std::string name;
  std::string filepath;

// customlines_data[0][0] customline_id
  // customlines_data[0][1] name
  // customlines_data[0][2] position
  // customlines_data[0][3] insert keyword
  // customlines_data[0][4] insert keyword id
  // customlines_data[0][5] customline


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