#include "ccxStepFrequencyModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepFrequencyModifyCommand::ccxStepFrequencyModifyCommand()
{}

ccxStepFrequencyModifyCommand::~ccxStepFrequencyModifyCommand()
{}

std::vector<std::string> ccxStepFrequencyModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  std::string syntax = "ccx ";
  syntax.append("modify step <value:label='step id',help='<step id>'> ");
  syntax.append("frequency ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[solver {pastix|pardiso|spooles|custom <string:type='unquoted', number='1', label='customsolver', help='<customsolver>'>}] ");
  syntax.append("[{storage_yes|storage_no}] ");
  syntax.append("[{global_yes|global_no}] ");
  syntax.append("[{cycmpc_active|cycmpc_inactive}] ");
  syntax.append("[eigenfrequencies <value:label='eigenfrequencies',help='<eigenfrequencies>'>] ");
  syntax.append("[lower <value:label='lower',help='<lower>'>] ");
  syntax.append("[upper <value:label='upper',help='<upper>'>] ");
    
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxStepFrequencyModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify step <step id> ");
  help[0].append("frequency ");
  help[0].append("[name <name>] " );
  help[0].append("[solver {pastix|pardiso|spooles|custom <string>] ");
  help[0].append("[{storage_yes|storage_no}] ");
  help[0].append("[{global_yes|global_no}] ");
  help[0].append("[{cycmpc_active|cycmpc_inactive}] ");
  help[0].append("[eigenfrequencies <value>] ");
  help[0].append("[lower <value>] ");
  help[0].append("[upper <value>] ");
  
  return help;
}

std::vector<std::string> ccxStepFrequencyModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepFrequencyModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int step_id;
  std::string name;
  std::string solver;
  std::string storage;
  std::string global;
  std::string cycmpc;
  int eigenfrequencies_value;
  std::string eigenfrequencies;
  int lower_value;
  std::string lower;
  int upper_value;
  std::string upper;

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

  if (data.find_keyword("STORAGE_YES"))
  {
    storage = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("STORAGE_NO"))
  {
    storage = "";
    options_marker.push_back(1);
  }else{
    storage = "";
    options_marker.push_back(0);
  }
  options.push_back(storage);

  if (data.find_keyword("GLOBAL_YES"))
  {
    global = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("GLOBAL_NO"))
  {
    global = "";
    options_marker.push_back(1);
  }else{
    global = "";
    options_marker.push_back(0);
  }
  options.push_back(global);

  if (data.find_keyword("CYCMPC_ACTIVE"))
  {
    cycmpc = "CYCMPC=ACTIVE";
    options_marker.push_back(1);
  }else if (data.find_keyword("CYCMPC_INACTIVE"))
  {
    cycmpc = "CYCMPC=INACTIVE";
    options_marker.push_back(1);
  }else{
    cycmpc = "";
    options_marker.push_back(0);
  }
  options.push_back(cycmpc);

  if (!data.get_value("eigenfrequencies", eigenfrequencies_value))
  {
    eigenfrequencies = "";
    options_marker.push_back(0);
  }
  else
  {
    eigenfrequencies = std::to_string(eigenfrequencies_value);
    options_marker.push_back(1);
  }
  options.push_back(eigenfrequencies);

  if (!data.get_value("lower", lower_value))
  {
    lower = "";
    options_marker.push_back(0);
  }
  else
  {
    lower = std::to_string(lower_value);
    options_marker.push_back(1);
  }
  options.push_back(lower);

  if (!data.get_value("upper", upper_value))
  {
    upper = "";
    options_marker.push_back(0);
  }
  else
  {
    upper = std::to_string(upper_value);
    options_marker.push_back(1);
  }
  options.push_back(upper);
  
  if (!ccx_iface.modify_step(step_id, 3, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}