#include "ccxLoadsForcesModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsForcesModifyCommand::ccxLoadsForcesModifyCommand()
{}

ccxLoadsForcesModifyCommand::~ccxLoadsForcesModifyCommand()
{}

std::vector<std::string> ccxLoadsForcesModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify force <value:label='force id',help='<force id>'> ");
  syntax.append("[op {mod | new}] " );
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxLoadsForcesModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx modify force <force id> [op {mod | new}] [amplitude <amplitude id>] [timedelay <value>]"; 

  return help;
}

std::vector<std::string> ccxLoadsForcesModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsForcesModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  std::vector<int> options_marker;
  int force_id;
  std::string amplitude_id;
  int amplitude_value;
  std::string timedelay;
  double timedelay_value;

  data.get_value("force id", force_id);
  
  if (data.find_keyword("OP")){
    if (data.find_keyword("MOD")){
      options_marker.push_back(1);
      options.push_back("0");
    }else if (data.find_keyword("NEW"))
    {
      options_marker.push_back(1);
      options.push_back("1");
    }
  }else{
      options_marker.push_back(0);
      options.push_back("0");
  }

  if (!data.get_value("amplitude id", amplitude_value))
  {
    amplitude_id = "";
    options_marker.push_back(0);
  }
  else
  {
    amplitude_id = std::to_string(amplitude_value);
    options_marker.push_back(1);
  }
  options.push_back(amplitude_id);

  if (!data.get_value("timedelay", timedelay_value))
  {
    timedelay = "";
    options_marker.push_back(0);
  }
  else
  {
    timedelay = std::to_string(timedelay_value);
    options_marker.push_back(1);
  }
  options.push_back(timedelay);
  
  if (!ccx_iface.modify_loadsforces(force_id, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}