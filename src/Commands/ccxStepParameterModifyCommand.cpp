#include "ccxStepParameterModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepParameterModifyCommand::ccxStepParameterModifyCommand()
{}

ccxStepParameterModifyCommand::~ccxStepParameterModifyCommand()
{}

std::vector<std::string> ccxStepParameterModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx ";
  syntax.append("modify step <value:label='step id',help='<step id>'> ");
  syntax.append("parameter ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[{perturbation_yes|perturbation_no}] ");
  syntax.append("[{nlgeom_yes|nlgeom_no}] ");
  syntax.append("[inc <value:label='inc',help='<inc>'>] ");
  syntax.append("[incf <value:label='incf',help='<incf>'>] ");
  syntax.append("[{thermalnetwork_yes|thermalnetwork_no}] ");
  syntax.append("[{amplitude_ramp|amplitude_step}] ");
  syntax.append("[shocksmoothing <value:label='shocksmoothing',help='<shocksmoothing>'>] ");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxStepParameterModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify step <step id> ");
  help[0].append("parameter ");
  help[0].append("[name <name>] " );
  help[0].append("[{perturbation_yes|perturbation_no}] ");
  help[0].append("[{nlgeom_yes|nlgeom_no}] ");
  help[0].append("[inc <value>] ");
  help[0].append("[incf <value>] ");
  help[0].append("[{thermalnetwork_yes|thermalnetwork_no}] ");
  help[0].append("[{amplitude_ramp|amplitude_step}] ");
  help[0].append("[shocksmoothing <value>] ");

  return help;
}

std::vector<std::string> ccxStepParameterModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepParameterModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int step_id;
  std::string name;
  std::string perturbation;
  std::string nlgeom;
  int inc_value;
  std::string inc;
  int incf_value;
  std::string incf;
  std::string thermalnetwork;
  std::string amplitude;
  double shocksmoothing_value;
  std::string shocksmoothing;

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
  
  if (data.find_keyword("PERTURBATION_YES"))
  {
    perturbation = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("PERTURBATION_NO"))
  {
    perturbation = "";
    options_marker.push_back(1);
  }else{
    perturbation = "";
    options_marker.push_back(0);
  }
  options.push_back(perturbation);

  if (data.find_keyword("NLGEOM_YES"))
  {
    nlgeom = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("NLGEOM_NO"))
  {
    nlgeom = "";
    options_marker.push_back(1);
  }else{
    nlgeom = "";
    options_marker.push_back(0);
  }
  options.push_back(nlgeom);

  if (!data.get_value("inc", inc_value))
  {
    inc = "";
    options_marker.push_back(0);
  }
  else
  {
    inc = std::to_string(inc_value);
    options_marker.push_back(1);
  }
  options.push_back(inc);

  if (!data.get_value("incf", incf_value))
  {
    incf = "";
    options_marker.push_back(0);
  }
  else
  {
    incf = std::to_string(incf_value);
    options_marker.push_back(1);
  }
  options.push_back(incf);

  if (data.find_keyword("THERMALNETWORK_YES"))
  {
    thermalnetwork = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("THERMALNETWORK_NO"))
  {
    thermalnetwork = "";
    options_marker.push_back(1);
  }else{
    thermalnetwork = "";
    options_marker.push_back(0);
  }
  options.push_back(thermalnetwork);

  if (data.find_keyword("AMPLITUDE_RAMP"))
  {
    amplitude = "AMPLITUDE=RAMP";
    options_marker.push_back(1);
  }else if (data.find_keyword("AMPLITUDE_STEP"))
  {
    amplitude = "AMPLITUDE=STEP";
    options_marker.push_back(1);
  }else{
    amplitude = "";
    options_marker.push_back(0);
  }
  options.push_back(amplitude);

  if (!data.get_value("shocksmoothing", shocksmoothing_value))
  {
    shocksmoothing = "";
    options_marker.push_back(0);
  }
  else
  {
    shocksmoothing = std::to_string(shocksmoothing_value);
    options_marker.push_back(1);
  }
  options.push_back(shocksmoothing);

  if (!ccx_iface.modify_step(step_id, 1, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}