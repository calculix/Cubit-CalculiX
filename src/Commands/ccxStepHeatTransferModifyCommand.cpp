#include "ccxStepHeatTransferModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepHeatTransferModifyCommand::ccxStepHeatTransferModifyCommand()
{}

ccxStepHeatTransferModifyCommand::~ccxStepHeatTransferModifyCommand()
{}

std::vector<std::string> ccxStepHeatTransferModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx ";
  syntax.append("modify step <value:label='step id',help='<step id>'> ");
  syntax.append("heattransfer ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[solver {pastix|pardiso|spooles|custom <string:type='unquoted', number='1', label='customsolver', help='<customsolver>'>}] ");
  syntax.append("[{direct_yes|direct_no}] ");
  syntax.append("[{steadystate_yes|steadystate_no}] ");
  syntax.append("[{frequency_yes|frequency_no}] ");
  syntax.append("[{modaldynamic_yes|modaldynamic_no}] ");
  syntax.append("[{storage_yes|storage_no}] ");
  syntax.append("[deltmx <value:label='deltmx',help='<deltmx>'>] ");
  syntax.append("[{timereset_yes|timereset_no}] ");
  syntax.append("[totaltimeatstart <value:label='totaltimeatstart',help='<totaltimeatstart>'>] ");
  syntax.append("[initialtimeincrement <value:label='initialtimeincrement',help='<initialtimeincrement>'>] ");
  syntax.append("[timeperiodofstep <value:label='timeperiodofstep',help='<timeperiodofstep>'>] ");
  syntax.append("[minimumtimeincrement <value:label='minimumtimeincrement',help='<minimumtimeincrement>'>] ");
  syntax.append("[maximumtimeincrement <value:label='maximumtimeincrement',help='<maximumtimeincrement>'>] ");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxStepHeatTransferModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify step <step id> ");
  help[0].append("heattransfer ");
  help[0].append("[name <name>] " );
  help[0].append("[solver {pastix|pardiso|spooles|custom <string>] ");
  help[0].append("[{direct_yes|direct_no}] ");
  help[0].append("[{steadystate_yes|steadystate_no}] ");
  help[0].append("[{frequency_yes|frequency_no}] ");
  help[0].append("[{modaldynamic_yes|modaldynamic_no}] ");
  help[0].append("[{storage_yes|storage_no}] ");
  help[0].append("[deltmx <value>] ");
  help[0].append("[{timereset_yes|timereset_no}] ");
  help[0].append("[totaltimeatstart <value>] ");
  help[0].append("[initialtimeincrement <value>] ");
  help[0].append("[timeperiodofstep <value>] ");
  help[0].append("[minimumtimeincrement <value>] ");
  help[0].append("[maximumtimeincrement <value>] ");

  return help;
}

std::vector<std::string> ccxStepHeatTransferModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepHeatTransferModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int step_id;
  std::string name;
  std::string solver;
  std::string direct;
  std::string steadystate;
  std::string frequency;
  std::string modaldynamic;
  std::string storage;
  double deltmx_value;
  std::string deltmx;
  std::string timereset;
  double totaltimeatstart_value;
  std::string totaltimeatstart;
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

  if (data.find_keyword("STEADYSTATE_YES"))
  {
    steadystate = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("STEADYSTATE_NO"))
  {
    steadystate = "";
    options_marker.push_back(1);
  }else{
    steadystate = "";
    options_marker.push_back(0);
  }
  options.push_back(steadystate);

  if (data.find_keyword("FREQUENCY_YES"))
  {
    frequency = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("FREQUENCY_NO"))
  {
    frequency = "";
    options_marker.push_back(1);
  }else{
    frequency = "";
    options_marker.push_back(0);
  }
  options.push_back(frequency);

  if (data.find_keyword("MODALDYNAMIC_YES"))
  {
    modaldynamic = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("MODALDYNAMIC_NO"))
  {
    modaldynamic = "";
    options_marker.push_back(1);
  }else{
    modaldynamic = "";
    options_marker.push_back(0);
  }
  options.push_back(modaldynamic);

  if (data.find_keyword("STORAGE_YES"))
  {
    storage = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("STORAGE_NO"))
  {
    storage = "NO";
    options_marker.push_back(1);
  }else{
    storage = "";
    options_marker.push_back(0);
  }
  options.push_back(storage);

  if (!data.get_value("deltmx", deltmx_value))
  {
    deltmx = "";
    options_marker.push_back(0);
  }
  else
  {
    deltmx = std::to_string(deltmx_value);
    options_marker.push_back(1);
  }
  options.push_back(deltmx);

  if (data.find_keyword("TIMERESET_YES"))
  {
    timereset = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("TIMERESET_NO"))
  {
    timereset = "";
    options_marker.push_back(1);
  }else{
    timereset = "";
    options_marker.push_back(0);
  }
  options.push_back(timereset);

  if (!data.get_value("totaltimeatstart", totaltimeatstart_value))
  {
    totaltimeatstart = "";
    options_marker.push_back(0);
  }
  else
  {
    totaltimeatstart = std::to_string(totaltimeatstart_value);
    options_marker.push_back(1);
  }
  options.push_back(totaltimeatstart);
   
  if (!data.get_value("initialtimeincrement", initialtimeincrement_value))
  {
    initialtimeincrement = "";
    options_marker.push_back(0);
  }
  else
  {
    initialtimeincrement = std::to_string(initialtimeincrement_value);
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
    timeperiodofstep = std::to_string(timeperiodofstep_value);
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
    minimumtimeincrement = std::to_string(minimumtimeincrement_value);
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
    maximumtimeincrement = std::to_string(maximumtimeincrement_value);
    options_marker.push_back(1);
  }
  options.push_back(maximumtimeincrement);

  if (!ccx_iface.modify_step(step_id, 5, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}