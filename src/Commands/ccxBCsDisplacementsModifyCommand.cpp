#include "ccxBCsDisplacementsModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxBCsDisplacementsModifyCommand::ccxBCsDisplacementsModifyCommand()
{}

ccxBCsDisplacementsModifyCommand::~ccxBCsDisplacementsModifyCommand()
{}

std::vector<std::string> ccxBCsDisplacementsModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify displacement <value:label='displacement id',help='<displacement id>'>");
  syntax.append("[op {mod | new}] " );
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[{fixed_1 | unfixed_1}] ");
  syntax.append("[{fixed_2 | unfixed_2}] ");
  syntax.append("[{fixed_3 | unfixed_3}] ");
  syntax.append("[{fixed_4 | unfixed_4}] ");
  syntax.append("[{fixed_5 | unfixed_5}] ");
  syntax.append("[{fixed_6 | unfixed_6}] ");
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxBCsDisplacementsModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify displacement <displacement id> [op {mod | new}] [amplitude <amplitude id>] [timedelay <value>] [{fixed_1|unfixed_1}] [{fixed_2|unfixed_2}] [{fixed_3|unfixed_3}] [{fixed_4|unfixed_4}] [{fixed_5|unfixed_5}] [{fixed_6|unfixed_6}]"; 

  return help;
}

std::vector<std::string> ccxBCsDisplacementsModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxBCsDisplacementsModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;  
  std::vector<int> options_marker;
  int displacement_id;
  std::string amplitude_id;
  int amplitude_value;
  std::string timedelay;
  double timedelay_value;

  data.get_value("displacement id", displacement_id);
  
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
  
  if (data.find_keyword("FIXED_1"))
  {
    options_marker.push_back(1);
    options.push_back("1");
  }else if (data.find_keyword("UNFIXED_1"))
  {
    options_marker.push_back(1);
    options.push_back("0");
  }
  else
  {
    options_marker.push_back(0);
    options.push_back("0");
  }
  if (data.find_keyword("FIXED_2"))
  {
    options_marker.push_back(1);
    options.push_back("1");
  }else if (data.find_keyword("UNFIXED_2"))
  {
    options_marker.push_back(1);
    options.push_back("0");
  }else
  {
    options_marker.push_back(0);
    options.push_back("0");
  }
  if (data.find_keyword("FIXED_3"))
  {
    options_marker.push_back(1);
    options.push_back("1");
  }else if (data.find_keyword("UNFIXED_3"))
  {
    options_marker.push_back(1);
    options.push_back("0");
  }else
  {
    options_marker.push_back(0);
    options.push_back("0");
  }
  if (data.find_keyword("FIXED_4"))
  {
    options_marker.push_back(1);
    options.push_back("1");
  }else if (data.find_keyword("UNFIXED_4"))
  {
    options_marker.push_back(1);
    options.push_back("0");
  }else
  {
    options_marker.push_back(0);
    options.push_back("0");
  }
  if (data.find_keyword("FIXED_5"))
  {
    options_marker.push_back(1);
    options.push_back("1");
  }else if (data.find_keyword("UNFIXED_5"))
  {
    options_marker.push_back(1);
    options.push_back("0");
  }else
  {
    options_marker.push_back(0);
    options.push_back("0");
  }
  if (data.find_keyword("FIXED_6"))
  {
    options_marker.push_back(1);
    options.push_back("1");
  }else if (data.find_keyword("UNFIXED_6"))
  {
    options_marker.push_back(1);
    options.push_back("0");
  }else
  {
    options_marker.push_back(0);
    options.push_back("0");
  }
  
  if (!ccx_iface.modify_bcsdisplacements(displacement_id, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}