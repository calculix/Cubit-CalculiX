#include "ccxStepBuckleModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepBuckleModifyCommand::ccxStepBuckleModifyCommand()
{}

ccxStepBuckleModifyCommand::~ccxStepBuckleModifyCommand()
{}

std::vector<std::string> ccxStepBuckleModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx ";
  syntax.append("modify step <value:label='step id',help='<step id>'> ");
  syntax.append("buckle ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[solver {pastix|pardiso|spooles|custom <string:type='unquoted', number='1', label='customsolver', help='<customsolver>'>}] ");
  syntax.append("[bucklingfactors <value:label='bucklingfactors',help='<bucklingfactors>'>] ");
  syntax.append("[accuracy <value:label='accuracy',help='<accuracy>'>] ");
  syntax.append("[lanczosvectors <value:label='lanczosvectors',help='<lanczosvectors>'>] ");
  syntax.append("[maximumiterations <value:label='maximumiterations',help='<maximumiterations>'>] ");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxStepBuckleModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify step <step id> ");
  help[0].append("buckle ");
  help[0].append("[name <name>] " );
  help[0].append("[solver {pastix|pardiso|spooles|custom <string>] ");
  help[0].append("[bucklingfactors <value>] ");
  help[0].append("[accuracy <value>] ");
  help[0].append("[lanczosvectors <value>] ");
  help[0].append("[maximumiterations <value>] ");
  
  return help;
}

std::vector<std::string> ccxStepBuckleModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepBuckleModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int step_id;
  std::string name;
  std::string solver;
  int bucklingfactors_value;
  std::string bucklingfactors;
  double accuracy_value;
  std::string accuracy;
  int lanczosvectors_value;
  std::string lanczosvectors;
  int maximumiterations_value;
  std::string maximumiterations;

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

  if (!data.get_value("bucklingfactors", bucklingfactors_value))
  {
    bucklingfactors = "";
    options_marker.push_back(0);
  }
  else
  {
    bucklingfactors = std::to_string(bucklingfactors_value);
    options_marker.push_back(1);
  }
  options.push_back(bucklingfactors);

  if (!data.get_value("accuracy", accuracy_value))
  {
    accuracy = "";
    options_marker.push_back(0);
  }
  else
  {
    accuracy = ccx_iface.to_string_scientific(accuracy_value);
    options_marker.push_back(1);
  }
  options.push_back(accuracy);

  if (!data.get_value("lanczosvectors", lanczosvectors_value))
  {
    lanczosvectors = "";
    options_marker.push_back(0);
  }
  else
  {
    lanczosvectors = std::to_string(lanczosvectors_value);
    options_marker.push_back(1);
  }
  options.push_back(lanczosvectors);

  if (!data.get_value("maximumiterations", maximumiterations_value))
  {
    maximumiterations = "";
    options_marker.push_back(0);
  }
  else
  {
    maximumiterations = std::to_string(maximumiterations_value);
    options_marker.push_back(1);
  }
  options.push_back(maximumiterations);

  if (!ccx_iface.modify_step(step_id, 4, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}