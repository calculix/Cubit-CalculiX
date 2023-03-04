#include "ccxStepAddFieldOutputCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepAddFieldOutputCommand::ccxStepAddFieldOutputCommand()
{}

ccxStepAddFieldOutputCommand::~ccxStepAddFieldOutputCommand()
{}

std::vector<std::string> ccxStepAddFieldOutputCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("step <value:label='step id',help='<step id>'> ");
  syntax.append("add fieldoutput <value:label='fieldoutput id',help='<fieldoutput id>'>... ");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxStepAddFieldOutputCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx step <step id> add fieldoutput <fieldoutput id>..."; 

  return help;
}

std::vector<std::string> ccxStepAddFieldOutputCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepAddFieldOutputCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  
  int step_id;
  std::vector<int> fieldoutput_ids;

  data.get_value("step id", step_id);

  data.get_values("fieldoutput id", fieldoutput_ids);
   
  if (!ccx_iface.step_add_fieldoutputs(step_id, fieldoutput_ids))
  {
    output = "Failed adding Field Output!\n";
    PRINT_ERROR(output.c_str());
  }
     
  return true;
}