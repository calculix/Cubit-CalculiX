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
  syntax.append("constraint|surfaceinteraction|contactpair|amplitude|orientation|");
  syntax.append("force|pressure|heatflux|gravity|centrifugal|displacement|temperature|");
  syntax.append("historyoutput|fieldoutput|initialcondition|step_begin|step_end} ");
  syntax.append("item_id <value:label='item_id',help='<item_id>'> ");
  syntax.append("cline <string:type='quoted', number='1', label='cline', help='<custom line>'> ");
  syntax_list.push_back(syntax);

  syntax = "ccx ";
  syntax.append("create customline ");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> ");
  syntax.append("{begin|end} ");
  syntax.append("export ");
  syntax.append("cline <string:type='quoted', number='1', label='cline', help='<custom line>'> ");
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxCustomLineCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(2);
  help[0] = "ccx ";
  help[0].append("create customline ");
  help[0].append("name <name> ");
  help[0].append("{before|after} ");
  help[0].append("{elset|nset|sideset|material|section|");
  help[0].append("constraint|surfaceinteraction|contactpair|amplitude|orientation|");
  help[0].append("force|pressure|heatflux|gravity|centrifugal|displacement|temperature|");
  help[0].append("historyoutput|fieldoutput|initialcondition|step_begin|step_end} ");
  help[0].append("item_id <item_id> ");
  help[0].append("cline <custom line> ");

  help[1] = "ccx ";
  help[1].append("create customline ");
  help[1].append("name <name> ");
  help[1].append("{begin|end} ");
  help[1].append("export ");
  help[1].append("cline <custom line> ");

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
  int item_id_value;
  std::string item_id;
  std::string cline;

  data.get_string("name", name);
  options.push_back(name);
  
  if (data.find_keyword("BEFORE"))
  {
    options.push_back("BEFORE");
  } else if (data.find_keyword("AFTER"))
  {
    options.push_back("AFTER");
  } else if (data.find_keyword("BEGIN"))
  {
    options.push_back("BEGIN");
  } else if (data.find_keyword("END"))
  {
    options.push_back("END");
  }

  if (data.find_keyword("ELSET"))
  {
    options.push_back("ELSET");
  } else if (data.find_keyword("NSET"))
  {
    options.push_back("NSET");
  } else if (data.find_keyword("SIDESET"))
  {
    options.push_back("SIDESET");
  } else if (data.find_keyword("MATERIAL"))
  {
    options.push_back("MATERIAL");
  } else if (data.find_keyword("SECTION"))
  {
    options.push_back("SECTION");
  } else if (data.find_keyword("CONSTRAINT"))
  {
    options.push_back("CONSTRAINT");
  } else if (data.find_keyword("SURFACEINTERACTION"))
  {
    options.push_back("SURFACEINTERACTION");
  } else if (data.find_keyword("CONTACTPAIR"))
  {
    options.push_back("CONTACTPAIR");
  } else if (data.find_keyword("AMPLITUDE"))
  {
    options.push_back("AMPLITUDE");
  } else if (data.find_keyword("ORIENTATION"))
  {
    options.push_back("ORIENTATION");
  }else if (data.find_keyword("FORCE"))
  {
    options.push_back("FORCE");
  }else if (data.find_keyword("PRESSURE"))
  {
    options.push_back("PRESSURE");
  }else if (data.find_keyword("HEATFLUX"))
  {
    options.push_back("HEATFLUX");
  }else if (data.find_keyword("GRAVITY"))
  {
    options.push_back("GRAVITY");
  }else if (data.find_keyword("CENTRIFUGAL"))
  {
    options.push_back("CENTRIFUGAL");
  }else if (data.find_keyword("DISPLACEMENT"))
  {
    options.push_back("DISPLACEMENT");
  }else if (data.find_keyword("TEMPERATURE"))
  {
    options.push_back("TEMPERATURE");
  }else if (data.find_keyword("HISTORYOUTPUT"))
  {
    options.push_back("HISTORYOUTPUT");
  }else if (data.find_keyword("FIELDOUTPUT"))
  {
    options.push_back("FIELDOUTPUT");
  }else if (data.find_keyword("INITIALCONDITION"))
  {
    options.push_back("INITIALCONDITION");
  }else if (data.find_keyword("STEP_BEGIN"))
  {
    options.push_back("STEP_BEGIN");
  }else if (data.find_keyword("STEP_END"))
  {
    options.push_back("STEP_END");
  }else if (data.find_keyword("EXPORT"))
  {
    options.push_back("EXPORT");
  }

  if (data.find_keyword("EXPORT"))
  {
    options.push_back("-1");
  } else {
    data.get_value("item_id", item_id_value);
    item_id = std::to_string(item_id_value);
    options.push_back(item_id);
  }

  data.get_string("cline", cline);
  options.push_back(cline);

  if (!ccx_iface.create_customline(options))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}