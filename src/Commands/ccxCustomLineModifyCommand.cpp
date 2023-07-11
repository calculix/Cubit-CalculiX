#include "ccxCustomLineModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxCustomLineModifyCommand::ccxCustomLineModifyCommand()
{}

ccxCustomLineModifyCommand::~ccxCustomLineModifyCommand()
{}

std::vector<std::string> ccxCustomLineModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify customline <value:label='customline id',help='<customline id>'>");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] ");
  syntax.append("[{before|after}] ");
  syntax.append("[{elset|nset|sideset|material|section|");
  syntax.append("constraint|surfaceinteraction|contactpair|amplitude|");
  syntax.append("force|pressure|heatflux|displacement|temperature|");
  syntax.append("historyoutput|fieldoutput|initialcondition|step_begin|step_end|export}] ");
  syntax.append("[item_id <value:label='item_id',help='<item_id>'>] ");
  syntax.append("[cline <string:type='quoted', number='1', label='cline', help='<custom line>'>] ");
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxCustomLineModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify customline <customline id> ");
  help[0].append("[name <name>] ");
  help[0].append("[{before|after|begin|end}] ");
  help[0].append("[{elset|nset|sideset|material|section|");
  help[0].append("constraint|surfaceinteraction|contactpair|amplitude|");
  help[0].append("force|pressure|heatflux|displacement|temperature|");
  help[0].append("historyoutput|fieldoutput|initialcondition|step_begin|step_end|export}] ");
  help[0].append("[item_id <item_id>] ");
  help[0].append("[cline <custom line>] ");

  return help;
}

std::vector<std::string> ccxCustomLineModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxCustomLineModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  std::vector<int> options_marker;
  std::string name;
  int item_id_value;
  std::string item_id;
  std::string cline;
  int customline_id;
  
  data.get_value("customline id", customline_id);

  if (!data.get_string("name", name))
  {
    name = "";
    options_marker.push_back(0);
  }
  else
  {
    options_marker.push_back(1);
  }
  options.push_back(name);
  
  if (data.find_keyword("BEFORE"))
  {
    options.push_back("BEFORE");
    options_marker.push_back(1);
  } else if (data.find_keyword("AFTER"))
  {
    options.push_back("AFTER");
    options_marker.push_back(1);
  } else if (data.find_keyword("BEGIN"))
  {
    options.push_back("BEGIN");
    options_marker.push_back(1);
  } else if (data.find_keyword("END"))
  {
    options.push_back("END");
    options_marker.push_back(1);
  } else {
    options.push_back("");
    options_marker.push_back(0);
  }

  if (data.find_keyword("ELSET"))
  {
    options.push_back("ELSET");
    options_marker.push_back(1);
  } else if (data.find_keyword("NSET"))
  {
    options.push_back("NSET");
    options_marker.push_back(1);
  } else if (data.find_keyword("SIDESET"))
  {
    options.push_back("SIDESET");
    options_marker.push_back(1);
  } else if (data.find_keyword("MATERIAL"))
  {
    options.push_back("MATERIAL");
    options_marker.push_back(1);
  } else if (data.find_keyword("SECTION"))
  {
    options.push_back("SECTION");
    options_marker.push_back(1);
  } else if (data.find_keyword("CONSTRAINT"))
  {
    options.push_back("CONSTRAINT");
    options_marker.push_back(1);
  } else if (data.find_keyword("SURFACEINTERACTION"))
  {
    options.push_back("SURFACEINTERACTION");
    options_marker.push_back(1);
  } else if (data.find_keyword("CONTACTPAIR"))
  {
    options.push_back("CONTACTPAIR");
    options_marker.push_back(1);
  } else if (data.find_keyword("AMPLITUDE"))
  {
    options.push_back("AMPLITUDE");
    options_marker.push_back(1);
  }else if (data.find_keyword("FORCE"))
  {
    options.push_back("FORCE");
    options_marker.push_back(1);
  }else if (data.find_keyword("PRESSURE"))
  {
    options.push_back("PRESSURE");
    options_marker.push_back(1);
  }else if (data.find_keyword("HEATFLUX"))
  {
    options.push_back("HEATFLUX");
    options_marker.push_back(1);
  }else if (data.find_keyword("DISPLACEMENT"))
  {
    options.push_back("DISPLACEMENT");
    options_marker.push_back(1);
  }else if (data.find_keyword("TEMPERATURE"))
  {
    options.push_back("TEMPERATURE");
    options_marker.push_back(1);
  }else if (data.find_keyword("HISTORYOUTPUT"))
  {
    options.push_back("HISTORYOUTPUT");
    options_marker.push_back(1);
  }else if (data.find_keyword("FIELDOUTPUT"))
  {
    options.push_back("FIELDOUTPUT");
    options_marker.push_back(1);
  }else if (data.find_keyword("INITIALCONDITION"))
  {
    options.push_back("INITIALCONDITION");
    options_marker.push_back(1);
  }else if (data.find_keyword("STEP_BEGIN"))
  {
    options.push_back("STEP_BEGIN");
    options_marker.push_back(1);
  }else if (data.find_keyword("STEP_END"))
  {
    options.push_back("STEP_END");
    options_marker.push_back(1);
  }else if (data.find_keyword("EXPORT"))
  {
    options.push_back("EXPORT");
    options_marker.push_back(1);
  } else {
    options.push_back("");
    options_marker.push_back(0);
  }

  if (data.find_keyword("EXPORT"))
  {
    options.push_back("-1");
    options_marker.push_back(1);
  } else {
    if (!data.get_value("item_id", item_id_value))
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    else
    {
      item_id = std::to_string(item_id_value);
      options.push_back(item_id);
      options_marker.push_back(1);
    }
  }

  if (!data.get_string("cline", cline))
  {
    cline = "";
    options.push_back(cline);
    options_marker.push_back(0);
  }
  else
  {
    options.push_back(cline);
    options_marker.push_back(1);
  }

  if (!ccx_iface.modify_customline(customline_id, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}