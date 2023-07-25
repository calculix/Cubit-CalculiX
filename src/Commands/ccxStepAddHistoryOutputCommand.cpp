#include "ccxStepAddHistoryOutputCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepAddHistoryOutputCommand::ccxStepAddHistoryOutputCommand()
{}

ccxStepAddHistoryOutputCommand::~ccxStepAddHistoryOutputCommand()
{}

std::vector<std::string> ccxStepAddHistoryOutputCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("step <value:label='step id',help='<step id>'> ");
  syntax.append("add historyoutput <value:label='historyoutput id',help='<historyoutput id>'>...");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxStepAddHistoryOutputCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx step <step id> add historyoutput <historyoutput id>..."; 

  return help;
}

std::vector<std::string> ccxStepAddHistoryOutputCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepAddHistoryOutputCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  
  int step_id;
  std::vector<int> historyoutput_ids;

  data.get_value("step id", step_id);

  data.get_values("historyoutput id", historyoutput_ids);
   
  if (!ccx_iface.step_add_historyoutputs(step_id, historyoutput_ids))
  {
    output = "Failed adding History Output!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}