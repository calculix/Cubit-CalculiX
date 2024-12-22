#include "ccxStepSteadyStateDynamicsModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepSteadyStateDynamicsModifyCommand::ccxStepSteadyStateDynamicsModifyCommand()
{}

ccxStepSteadyStateDynamicsModifyCommand::~ccxStepSteadyStateDynamicsModifyCommand()
{}

std::vector<std::string> ccxStepSteadyStateDynamicsModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx ";
  syntax.append("modify step <value:label='step id',help='<step id>'> ");
  syntax.append("steadystatedynamics ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[solver {pastix|pardiso|spooles|custom <string:type='unquoted', number='1', label='customsolver', help='<customsolver>'>}] ");
  syntax.append("[{harmonic_yes|harmonic_no}] ");
  syntax.append("[lower_bound_frequency <value:label='lower_bound_frequency',help='<lower_bound_frequency>'>] ");
  syntax.append("[upper_bound_frequency <value:label='upper_bound_frequency',help='<upper_bound_frequency>'>] ");
  syntax.append("[data_points <value:label='data_points',help='<data_points>'>] ");
  syntax.append("[bias <value:label='bias',help='<bias>'>] ");
  syntax.append("[fourier <value:label='fourier',help='<fourier>'>] ");
  syntax.append("[lower_bound_time <value:label='lower_bound_time',help='<lower_bound_time>'>] ");
  syntax.append("[upper_bound_time <value:label='upper_bound_time',help='<upper_bound_time>'>] ");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxStepSteadyStateDynamicsModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify step <step id> ");
  help[0].append("steadystatedynamics ");
  help[0].append("[name <name>] " );
  help[0].append("[solver {pastix|pardiso|spooles|custom <customsolver>}] ");
  help[0].append("[{harmonic_yes|harmonic_no}] ");
  help[0].append("[lower_bound_frequency <value>] ");
  help[0].append("[upper_bound_frequency <value>] ");
  help[0].append("[data_points <value>] ");
  help[0].append("[bias <value>] ");
  help[0].append("[fourier <value>] ");
  help[0].append("[lower_bound_time <value>] ");
  help[0].append("[upper_bound_time <value>] ");

  return help;
}

std::vector<std::string> ccxStepSteadyStateDynamicsModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepSteadyStateDynamicsModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int step_id;
  std::string name;
  std::string solver;
  std::string harmonic;  
  double lower_bound_frequency_value;
  std::string lower_bound_frequency;
  double upper_bound_frequency_value;
  std::string upper_bound_frequency;
  double data_points_value;
  std::string data_points;
  double bias_value;
  std::string bias;
  double fourier_value;
  std::string fourier;
  double lower_bound_time_value;
  std::string lower_bound_time;
  double upper_bound_time_value;
  std::string upper_bound_time;

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

  if (data.find_keyword("HARMONIC_YES"))
  {
    harmonic = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("HARMONIC_NO"))
  {
    harmonic = "";
    options_marker.push_back(1);
  }else{
    harmonic = "";
    options_marker.push_back(0);
  }
  options.push_back(harmonic);

  if (!data.get_value("lower_bound_frequency", lower_bound_frequency_value))
  {
    lower_bound_frequency = "";
    options_marker.push_back(0);
  }
  else
  {
    lower_bound_frequency = ccx_iface.to_string_scientific(lower_bound_frequency_value);
    options_marker.push_back(1);
  }
  options.push_back(lower_bound_frequency);

  if (!data.get_value("upper_bound_frequency", upper_bound_frequency_value))
  {
    upper_bound_frequency = "";
    options_marker.push_back(0);
  }
  else
  {
    upper_bound_frequency = ccx_iface.to_string_scientific(upper_bound_frequency_value);
    options_marker.push_back(1);
  }
  options.push_back(upper_bound_frequency);

  if (!data.get_value("data_points", data_points_value))
  {
    data_points = "";
    options_marker.push_back(0);
  }
  else
  {
    data_points = ccx_iface.to_string_scientific(data_points_value);
    options_marker.push_back(1);
  }
  options.push_back(data_points);

  if (!data.get_value("bias", bias_value))
  {
    bias = "";
    options_marker.push_back(0);
  }
  else
  {
    bias = ccx_iface.to_string_scientific(bias_value);
    options_marker.push_back(1);
  }
  options.push_back(bias);

  if (!data.get_value("fourier", fourier_value))
  {
    fourier = "";
    options_marker.push_back(0);
  }
  else
  {
    fourier = ccx_iface.to_string_scientific(fourier_value);
    options_marker.push_back(1);
  }
  options.push_back(fourier);

  if (!data.get_value("lower_bound_time", lower_bound_time_value))
  {
    lower_bound_time = "";
    options_marker.push_back(0);
  }
  else
  {
    lower_bound_time = ccx_iface.to_string_scientific(lower_bound_time_value);
    options_marker.push_back(1);
  }
  options.push_back(lower_bound_time);

  if (!data.get_value("upper_bound_time", upper_bound_time_value))
  {
    upper_bound_time = "";
    options_marker.push_back(0);
  }
  else
  {
    upper_bound_time = ccx_iface.to_string_scientific(upper_bound_time_value);
    options_marker.push_back(1);
  }
  options.push_back(upper_bound_time);

  if (!ccx_iface.modify_step(step_id, 10, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}