#include "ccxLoadsSurfaceTractionDeleteCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxLoadsSurfaceTractionDeleteCommand::ccxLoadsSurfaceTractionDeleteCommand()
{}

ccxLoadsSurfaceTractionDeleteCommand::~ccxLoadsSurfaceTractionDeleteCommand()
{}

std::vector<std::string> ccxLoadsSurfaceTractionDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete surfacetraction ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='surfacetraction id',help='<surfacetraction id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='surfacetraction id',help='<surfacetraction id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <surfacetraction id>...
      syntax.append("all except ");
      syntax.append("<value:label='surfacetraction id except'>... ");
    }else if (syn_i==4)
    {
      // all except <surfacetraction id> to <surfacetraction id 2>
      syntax.append("all except ");
      syntax.append("<value:label='surfacetraction id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='surfacetraction id s2'>");
    }else if (syn_i==5)
    {
      // <surfacetraction id> to <surfacetraction id 2>
      syntax.append("<value:label='surfacetraction id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='surfacetraction id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxLoadsSurfaceTractionDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete surfacetraction <surfacetraction id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxLoadsSurfaceTractionDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsSurfaceTractionDeleteCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXLoadsInterface ccx_loads_iface;

  

  std::string output;

  std::vector<int> surfacetraction_ids;
  int surfacetraction_id_s1;
  int surfacetraction_id_s2;
  std::vector<std::string> surfacetractions_string;

  std::string surfacetraction_string = " ";
  
  data.get_value("surfacetraction id s1", surfacetraction_id_s1);
  data.get_value("surfacetraction id s2", surfacetraction_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("surfacetraction id except", surfacetraction_ids))
    {
      surfacetraction_string.append("all except");
      for (size_t i = 0; i < surfacetraction_ids.size(); i++)
      {
        surfacetraction_string.append(" " + std::to_string(surfacetraction_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    surfacetraction_string.append("all except " + std::to_string(surfacetraction_id_s1) + " to " + std::to_string(surfacetraction_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    surfacetraction_string.append(std::to_string(surfacetraction_id_s1) + " to " + std::to_string(surfacetraction_id_s2));
  }else if(data.get_strings("surfacetraction id", surfacetractions_string))
  {
    for (size_t i = 0; i < surfacetractions_string.size(); i++)
    {
      surfacetraction_string.append(surfacetractions_string[i]);
    }
  }
  
  if(!data.get_values("surfacetraction id", surfacetraction_ids))
  {   
    surfacetraction_ids = ccx_application_iface.parser("loadssurfacetraction", surfacetraction_string);
  }

  if (surfacetraction_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < surfacetraction_ids.size(); i++)
  {    
    if (!ccx_loads_iface.delete_loadssurfacetraction(surfacetraction_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(surfacetraction_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}