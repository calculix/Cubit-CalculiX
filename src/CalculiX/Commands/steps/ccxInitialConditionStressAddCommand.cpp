#include "ccxInitialConditionStressAddCommand.hpp"
#include <string>
#include <vector>
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxInitialConditionStressAddCommand::ccxInitialConditionStressAddCommand()
{}

ccxInitialConditionStressAddCommand::~ccxInitialConditionStressAddCommand()
{}

std::vector<std::string> ccxInitialConditionStressAddCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("initialcondition <value:label='initialcondition id',help='<initialcondition id>'> ");
  syntax.append("stress add ");
  syntax.append("element <value:label='element_id',help='<element_id>'> ");
  syntax.append("integration_point <value:label='integration_point',help='<integration_point>'> ");
  syntax.append("sxx <value:label='sxx',help='<sxx>'> ");
  syntax.append("syy <value:label='syy',help='<syy>'> ");
  syntax.append("szz <value:label='szz',help='<szz>'> ");
  syntax.append("sxy <value:label='sxy',help='<sxy>'> ");
  syntax.append("sxz <value:label='sxz',help='<sxz>'> ");
  syntax.append("syz <value:label='syz',help='<syz>'> ");
  syntax_list.push_back(syntax);

  syntax = "ccx ";
  syntax.append("initialcondition <value:label='initialcondition id',help='<initialcondition id>'> ");
  syntax.append("stress add ");
  syntax.append("block <value:label='block_id',help='<block_id>'> ");
  syntax.append("sxx <value:label='sxx',help='<sxx>'> ");
  syntax.append("syy <value:label='syy',help='<syy>'> ");
  syntax.append("szz <value:label='szz',help='<szz>'> ");
  syntax.append("sxy <value:label='sxy',help='<sxy>'> ");
  syntax.append("sxz <value:label='sxz',help='<sxz>'> ");
  syntax.append("syz <value:label='syz',help='<syz>'> ");
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxInitialConditionStressAddCommand::get_syntax_help()
{
  std::vector<std::string> help(2);
  help[0] = "ccx ";
  help[0].append("initialcondition <initialcondition id> ");
  help[0].append("stress add ");
  help[0].append("element <element_id> ");
  help[0].append("integration_point <integration_point> ");
  help[0].append("sxx <sxx> ");
  help[0].append("syy <syy> ");
  help[0].append("szz <szz> ");
  help[0].append("sxy <sxy> ");
  help[0].append("sxz <sxz> ");
  help[0].append("syz <syz> ");
  help[1] = "ccx ";
  help[1].append("initialcondition <initialcondition id> ");
  help[1].append("stress add ");
  help[1].append("block <block_id> ");
  help[1].append("sxx <sxx> ");
  help[1].append("syy <syy> ");
  help[1].append("szz <szz> ");
  help[1].append("sxy <sxy> ");
  help[1].append("sxz <sxz> ");
  help[1].append("syz <syz> ");

  return help;
}

std::vector<std::string> ccxInitialConditionStressAddCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxInitialConditionStressAddCommand::execute(CubitCommandData &data)
{
  CalculiXStepsInterface ccx_steps_iface;


  std::string output;
  std::vector<double> options;
  int initialcondition_id;
  int modify_type;
  double temp;
  std::string bc_id;

  data.get_value("initialcondition id", initialcondition_id);

  if (data.find_keyword("ELEMENT"))
  {
    modify_type = 1;
    data.get_value("element_id", temp);
    options.push_back(temp);
    data.get_value("integration_point", temp);
    options.push_back(temp);
  } else if (data.find_keyword("BLOCK"))
  {
    modify_type = 2;
    data.get_value("block_id", temp);
    options.push_back(temp);
  }
  data.get_value("sxx", temp);
  options.push_back(temp);
  data.get_value("syy", temp);
  options.push_back(temp);
  data.get_value("szz", temp);
  options.push_back(temp);
  data.get_value("sxy", temp);
  options.push_back(temp);
  data.get_value("sxz", temp);
  options.push_back(temp);
  data.get_value("syz", temp);
  options.push_back(temp);

  if (!ccx_steps_iface.add_initialcondition_stress(initialcondition_id, modify_type, options))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}