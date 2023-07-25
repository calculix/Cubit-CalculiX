#include "ccxSurfaceInteractionNameModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionNameModifyCommand::ccxSurfaceInteractionNameModifyCommand()
{}

ccxSurfaceInteractionNameModifyCommand::~ccxSurfaceInteractionNameModifyCommand()
{}

std::vector<std::string> ccxSurfaceInteractionNameModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("modify surfaceinteraction <value:label='surfaceinteraction id',help='<surfaceinteraction id>'>");
  syntax.append("name <string:type='unquoted', number='1', label='name', help='<name>'> " );
  
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionNameModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify surfaceinteraction <surfaceinteraction id> name <name>"; 

  return help;
}

std::vector<std::string> ccxSurfaceInteractionNameModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionNameModifyCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string name;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  std::vector<std::vector<std::string>> options2;
  int surfaceinteraction_id;

  data.get_value("surfaceinteraction id", surfaceinteraction_id);

  if (!data.get_string("name", name))
  {
    name = "";
    options_marker.push_back(0);
  }
  else
  {
    options_marker.push_back(1);
  }
  
  options.push_back(name);
  
  if (!ccx_iface.modify_surfaceinteraction("NAME",surfaceinteraction_id,options,options_marker,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}