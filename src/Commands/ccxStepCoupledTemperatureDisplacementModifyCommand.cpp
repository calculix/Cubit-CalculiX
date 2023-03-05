#include "ccxStepCoupledTemperatureDisplacementModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepCoupledTemperatureDisplacementModifyCommand::ccxStepCoupledTemperatureDisplacementModifyCommand()
{}

ccxStepCoupledTemperatureDisplacementModifyCommand::~ccxStepCoupledTemperatureDisplacementModifyCommand()
{}

std::vector<std::string> ccxStepCoupledTemperatureDisplacementModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx ";
  syntax.append("modify step <value:label='step id',help='<step id>'> ");
  syntax.append("coupledtemperaturedisplacement ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[solver {pastix|pardiso|spooles|custom <string:type='unquoted', number='1', label='customsolver', help='<customsolver>'>}] ");
  syntax.append("[{direct_yes|direct_no}] ");
  syntax.append("[alpha <value:label='alpha',help='<alpha>'>] ");
  syntax.append("[{steadystate_yes|steadystate_no}] ");
  syntax.append("[deltmx <value:label='deltmx',help='<deltmx>'>] ");
  syntax.append("[{timereset_yes|timereset_no}] ");
  syntax.append("[totaltimeatstart <value:label='totaltimeatstart',help='<totaltimeatstart>'>] ");
  syntax.append("[{compressible_yes|compressible_no}] ");
  syntax.append("[initialtimeincrement <value:label='initialtimeincrement',help='<initialtimeincrement>'>] ");
  syntax.append("[timeperiodofstep <value:label='timeperiodofstep',help='<timeperiodofstep>'>] ");
  syntax.append("[minimumtimeincrement <value:label='minimumtimeincrement',help='<minimumtimeincrement>'>] ");
  syntax.append("[maximumtimeincrement <value:label='maximumtimeincrement',help='<maximumtimeincrement>'>] ");
  syntax.append("[initialtimeincrementcfd <value:label='initialtimeincrementcfd',help='<initialtimeincrementcfd>'>] ");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxStepCoupledTemperatureDisplacementModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify step <step id> ");
  help[0].append("coupledtemperaturedisplacement ");
  help[0].append("[name <name>] " );
  help[0].append("[solver {pastix|pardiso|spooles|custom <string>] ");
  help[0].append("[{direct_yes|direct_no}] ");
  help[0].append("[alpha <value>] ");
  help[0].append("[{steadystate_yes|steadystate_no}] ");
  help[0].append("[deltmx <value>] ");
  help[0].append("[{timereset_yes|timereset_no}] ");
  help[0].append("[totaltimeatstart <value>] ");
  help[0].append("[{compressible_yes|compressible_no}] ");
  help[0].append("[initialtimeincrement <value>] ");
  help[0].append("[timeperiodofstep <value>] ");
  help[0].append("[minimumtimeincrement <value>] ");
  help[0].append("[maximumtimeincrement <value>] ");
  help[0].append("[initialtimeincrementcfd <value>] ");

  return help;
}

std::vector<std::string> ccxStepCoupledTemperatureDisplacementModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepCoupledTemperatureDisplacementModifyCommand::execute(CubitCommandData &data)
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
  std::string steadystate;
  double deltmx_value;
  std::string deltmx;
  std::string timereset;
  double totaltimeatstart_value;
  std::string totaltimeatstart;
  std::string compressible;
  double initialtimeincrement_value;
  std::string initialtimeincrement;
  double timeperiodofstep_value;
  std::string timeperiodofstep;
  double minimumtimeincrement_value;
  std::string minimumtimeincrement;
  double maximumtimeincrement_value;
  std::string maximumtimeincrement;
  double initialtimeincrementcfd_value;
  std::string initialtimeincrementcfd;

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

  if (data.find_keyword("COMPRESSIBLE_YES"))
  {
    compressible = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("COMPRESSIBLE_NO"))
  {
    compressible = "";
    options_marker.push_back(1);
  }else{
    compressible = "";
    options_marker.push_back(0);
  }
  options.push_back(compressible);

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

   if (!data.get_value("initialtimeincrementcfd", initialtimeincrementcfd_value))
  {
    initialtimeincrementcfd = "";
    options_marker.push_back(0);
  }
  else
  {
    initialtimeincrementcfd = std::to_string(initialtimeincrementcfd_value);
    options_marker.push_back(1);
  }
  options.push_back(initialtimeincrementcfd);

  if (!ccx_iface.modify_step(step_id, 6, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}