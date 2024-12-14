#include "ccxStepComplexFrequencyModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepComplexFrequencyModifyCommand::ccxStepComplexFrequencyModifyCommand()
{}

ccxStepComplexFrequencyModifyCommand::~ccxStepComplexFrequencyModifyCommand()
{}

std::vector<std::string> ccxStepComplexFrequencyModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx ";
  syntax.append("modify step <value:label='step id',help='<step id>'> ");
  syntax.append("complexfrequency ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[{coriolis_yes|coriolis_no}] ");
  syntax.append("[number_eigenfrequency <value:label='number_eigenfrequency',help='<number_eigenfrequency>'>] ");
  
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxStepComplexFrequencyModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify step <step id> ");
  help[0].append("complexfrequency ");
  help[0].append("[name <name>] " );
  help[0].append("[solver {pastix|pardiso|spooles|custom <customsolver>}] ");
  help[0].append("[{coriolis_yes|coriolis_no}] ");
  help[0].append("[number_eigenfrequency <value>] ");
  
  return help;
}

std::vector<std::string> ccxStepComplexFrequencyModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepComplexFrequencyModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int step_id;
  std::string name;
  std::string coriolis;  
  double number_eigenfrequency_value;
  std::string number_eigenfrequency;
  
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
  
  if (data.find_keyword("CORIOLIS_YES"))
  {
    coriolis = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("CORIOLIS_NO"))
  {
    coriolis = "";
    options_marker.push_back(1);
  }else{
    coriolis = "";
    options_marker.push_back(0);
  }
  options.push_back(coriolis);

  if (!data.get_value("number_eigenfrequency", number_eigenfrequency_value))
  {
    number_eigenfrequency = "";
    options_marker.push_back(0);
  }
  else
  {
    number_eigenfrequency = ccx_iface.to_string_scientific(number_eigenfrequency_value);
    options_marker.push_back(1);
  }
  options.push_back(number_eigenfrequency);

  if (!ccx_iface.modify_step(step_id, 11, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}