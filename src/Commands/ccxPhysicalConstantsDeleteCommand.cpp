#include "ccxPhysicalConstantsDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxPhysicalConstantsDeleteCommand::ccxPhysicalConstantsDeleteCommand()
{}

ccxPhysicalConstantsDeleteCommand::~ccxPhysicalConstantsDeleteCommand()
{}

std::vector<std::string> ccxPhysicalConstantsDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("delete physicalconstants [absolute_zero] [stefan_boltzmann] [newton_gravity] ");
  
  syntax_list.push_back(syntax); 

  return syntax_list;
}

std::vector<std::string> ccxPhysicalConstantsDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx delete physicalconstants [absolute_zero] [stefan_boltzmann] [newton_gravity]";
  
  return help;
}

std::vector<std::string> ccxPhysicalConstantsDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxPhysicalConstantsDeleteCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;
  std::string output;
  bool absolute_zero=false;
  bool stefan_boltzmann=false;
  bool newton_gravity=false;

  if (data.find_keyword("ABSOLUTE_ZERO"))
  {
    absolute_zero = true;
  }

  if (data.find_keyword("STEFAN_BOLTZMANN"))
  {
    stefan_boltzmann = true;
  }

  if (data.find_keyword("NEWTON_GRAVITY"))
  {
    newton_gravity = true;
  }
   
  if (!ccx_iface.delete_physicalconstants(absolute_zero,stefan_boltzmann,newton_gravity))
  {
    //PRINT_INFO("%s", output.c_str());  
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  } 
  
  return true;
}