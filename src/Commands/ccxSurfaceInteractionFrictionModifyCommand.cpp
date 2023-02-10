#include "ccxSurfaceInteractionFrictionModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSurfaceInteractionFrictionModifyCommand::ccxSurfaceInteractionFrictionModifyCommand()
{}

ccxSurfaceInteractionFrictionModifyCommand::~ccxSurfaceInteractionFrictionModifyCommand()
{}

std::vector<std::string> ccxSurfaceInteractionFrictionModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  std::string syntax = "ccx ";
  syntax.append("modify surfaceinteraction <value:label='surfaceinteraction id',help='<surfaceinteraction id>'>");
  syntax.append("friction " );
  syntax.append("mu <value:label='mu',help='<mu>'> " );
  syntax.append("lambda <value:label='lambda',help='<lambda>'>" );

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSurfaceInteractionFrictionModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx modify surfaceinteraction <surfaceinteraction id> friction mu <mu> lambda <lambda>";

  return help;
}

std::vector<std::string> ccxSurfaceInteractionFrictionModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSurfaceInteractionFrictionModifyCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<std::string> options;
  std::vector<int> options_marker;
  std::vector<std::vector<std::string>> options2;
  double mu_value;
  double lambda_value;
  std::string mu;
  std::string lambda;
  int surfaceinteraction_id;


  data.get_value("surfaceinteraction id", surfaceinteraction_id);

  if (!data.get_value("mu", mu_value))
  {
    mu = "";
    options_marker.push_back(0);
  }else
  {
    mu = std::to_string(mu_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("lambda", lambda_value))
  {
    lambda = "";
    options_marker.push_back(0);
  }else
  {
    lambda = std::to_string(lambda_value);
    options_marker.push_back(1);
  }
    
  options.push_back(mu);
  options.push_back(lambda);
  
  if (!ccx_iface.modify_surfaceinteraction("FRICTION",surfaceinteraction_id,options,options_marker,options2))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();

  return true;
}