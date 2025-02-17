#include "ccxStepDynamicModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepDynamicModifyCommand::ccxStepDynamicModifyCommand()
{}

ccxStepDynamicModifyCommand::~ccxStepDynamicModifyCommand()
{}

std::vector<std::string> ccxStepDynamicModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx ";
  syntax.append("modify step <value:label='step id',help='<step id>'> ");
  syntax.append("dynamic ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[solver {pastix|pardiso|spooles|custom <string:type='unquoted', number='1', label='customsolver', help='<customsolver>'>}] ");
  syntax.append("[{direct_yes|direct_no}] ");
  syntax.append("[alpha <value:label='alpha',help='<alpha>'>] ");
  syntax.append("[{explicit_yes|explicit_no}] ");
  syntax.append("[{relative_to_absolute_yes|relative_to_absolute_no}] ");
  syntax.append("[initialtimeincrement <value:label='initialtimeincrement',help='<initialtimeincrement>'>] ");
  syntax.append("[timeperiodofstep <value:label='timeperiodofstep',help='<timeperiodofstep>'>] ");
  syntax.append("[minimumtimeincrement <value:label='minimumtimeincrement',help='<minimumtimeincrement>'>] ");
  syntax.append("[maximumtimeincrement <value:label='maximumtimeincrement',help='<maximumtimeincrement>'>] ");
  syntax.append("[initialtimeincrementcfd <value:label='initialtimeincrementcfd',help='<initialtimeincrementcfd>'>] ");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxStepDynamicModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify step <step id> ");
  help[0].append("dynamic ");
  help[0].append("[name <name>] " );
  help[0].append("[solver {pastix|pardiso|spooles|custom <string>] ");
  help[0].append("[{direct_yes|direct_no}] ");
  help[0].append("[alpha <value>] ");
  help[0].append("[{explicit_yes|explicit_no}] ");
  help[0].append("[{relative_to_absolute_yes|relative_to_absolute_no}] ");
  help[0].append("[initialtimeincrement <value>] ");
  help[0].append("[timeperiodofstep <value>] ");
  help[0].append("[minimumtimeincrement <value>] ");
  help[0].append("[maximumtimeincrement <value>] ");

  return help;
}

std::vector<std::string> ccxStepDynamicModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepDynamicModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int step_id;
  std::string name;
  std::string solver;
  std::string direct;
  double alpha_value;
  std::string alpha;
  std::string explicit_string;
  std::string relative_to_path_string;
  double initialtimeincrement_value;
  std::string initialtimeincrement;
  double timeperiodofstep_value;
  std::string timeperiodofstep;
  double minimumtimeincrement_value;
  std::string minimumtimeincrement;
  double maximumtimeincrement_value;
  std::string maximumtimeincrement;

  data.get_value("step id", step_id);

  if (data.get_string("name", name))
  {
    options_marker.push_back(1);
    options.push_back(name);
    if (!ccx_iface.modify_step(step_id, 0, options, options_marker))
    {
      output = "Changing Name Failed!\n";
      PRINT_ERROR(output.c_str());
    }  
    options.clear();
    options_marker.clear();
  }
  
  if (data.find_keyword("PASTIX"))
  {
    solver = "PASTIX";
    options_marker.push_back(1);
  }else if (data.find_keyword("PARDISO"))
  {
    solver = "PARDISO";
    options_marker.push_back(1);
  }else if (data.find_keyword("SPOOLES"))
  {
    solver = "SPOOLES";
    options_marker.push_back(1);
  }else if (data.find_keyword("CUSTOM"))
  {
    data.get_string("customsolver", solver);
    options_marker.push_back(1);
  }else{
    solver = "";
    options_marker.push_back(0);
  }
  options.push_back(solver);

  if (data.find_keyword("DIRECT_YES"))
  {
    direct = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("DIRECT_NO"))
  {
    direct = "";
    options_marker.push_back(1);
  }else{
    direct = "";
    options_marker.push_back(0);
  }
  options.push_back(direct);

  if (!data.get_value("alpha", alpha_value))
  {
    alpha = "";
    options_marker.push_back(0);
  }
  else
  {
    alpha = std::to_string(alpha_value);
    options_marker.push_back(1);
  }
  options.push_back(alpha);

  if (data.find_keyword("EXPLICIT_YES"))
  {
    explicit_string = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("EXPLICIT_NO"))
  {
    explicit_string = "";
    options_marker.push_back(1);
  }else{
    explicit_string = "";
    options_marker.push_back(0);
  }
  options.push_back(explicit_string);

  if (data.find_keyword("RELATIVE_TO_PATH_YES"))
  {
    relative_to_path_string = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("RELATIVE_TO_PATH_NO"))
  {
    relative_to_path_string = "";
    options_marker.push_back(1);
  }else{
    relative_to_path_string = "";
    options_marker.push_back(0);
  }
  options.push_back(relative_to_path_string);
   
  if (!data.get_value("initialtimeincrement", initialtimeincrement_value))
  {
    initialtimeincrement = "";
    options_marker.push_back(0);
  }
  else
  {
    initialtimeincrement = ccx_iface.to_string_scientific(initialtimeincrement_value);
    options_marker.push_back(1);
  }
  options.push_back(initialtimeincrement);

  if (!data.get_value("timeperiodofstep", timeperiodofstep_value))
  {
    timeperiodofstep = "";
    options_marker.push_back(0);
  }
  else
  {
    timeperiodofstep = ccx_iface.to_string_scientific(timeperiodofstep_value);
    options_marker.push_back(1);
  }
  options.push_back(timeperiodofstep);
  
  if (!data.get_value("minimumtimeincrement", minimumtimeincrement_value))
  {
    minimumtimeincrement = "";
    options_marker.push_back(0);
  }
  else
  {
    minimumtimeincrement = ccx_iface.to_string_scientific(minimumtimeincrement_value);
    options_marker.push_back(1);
  }
  options.push_back(minimumtimeincrement);

  if (!data.get_value("maximumtimeincrement", maximumtimeincrement_value))
  {
    maximumtimeincrement = "";
    options_marker.push_back(0);
  }
  else
  {
    maximumtimeincrement = ccx_iface.to_string_scientific(maximumtimeincrement_value);
    options_marker.push_back(1);
  }
  options.push_back(maximumtimeincrement);

  if (!ccx_iface.modify_step(step_id, 8, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}