#include "ccxSectionSolidCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSectionSolidCreateCommand::ccxSectionSolidCreateCommand()
{}

ccxSectionSolidCreateCommand::~ccxSectionSolidCreateCommand()
{}

std::vector<std::string> ccxSectionSolidCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("create section solid block ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='block id',help='<block id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='block id',help='<block id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <block id>...
      syntax.append("all except ");
      syntax.append("<value:label='block id except'>... ");
    }else if (syn_i==4)
    {
      // all except <block id> to <block id 2>
      syntax.append("all except ");
      syntax.append("<value:label='block id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='block id s2'>");
    }else if (syn_i==5)
    {
      // <block id> to <block id 2>
      syntax.append("<value:label='block id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='block id s2'>");
    }
    
    syntax.append("material <value:label='material',help='<material>'> ");
    syntax.append("[orientation <value:label='orientation' ,help='<orientation_id>'>] ");
    syntax.append("[thickness <value:label='thickness',help='<thickness>'>]");
    
    syntax_list.push_back(syntax);
  }


  return syntax_list;
}

std::vector<std::string> ccxSectionSolidCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create section solid block <block id> material <material id> [orientation <orientation_id>] [thickness <thickness>]";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxSectionSolidCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSectionSolidCreateCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int material_id;
  int orientation_value;
  std::string orientation;
  std::vector<std::string> options;
  double thickness_value;
  std::string thickness; 
  std::vector<int> block_ids;
  int block_id_s1;
  int block_id_s2;
  std::vector<std::string> blocks_string;

  std::string block_string = " ";
  
  data.get_value("material", material_id);
  if (!data.get_value("orientation", orientation_value))
  {
    orientation = "-1";
  }
  else
  {
    orientation = std::to_string(orientation_value);
  }
  if (!data.get_value("thickness", thickness_value))
  {
    thickness = "";
  }
  else
  {
    thickness = std::to_string(thickness_value);
  }
  
  options.push_back(orientation);
  options.push_back(thickness);

  data.get_value("block id s1", block_id_s1);
  data.get_value("block id s2", block_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("block id except", block_ids))
    {
      block_string.append("all except");
      for (size_t i = 0; i < block_ids.size(); i++)
      {
        block_string.append(" " + std::to_string(block_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    block_string.append("all except " + std::to_string(block_id_s1) + " to " + std::to_string(block_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    block_string.append(std::to_string(block_id_s1) + " to " + std::to_string(block_id_s2));
  }else if(data.get_strings("block id", blocks_string))
  {
    for (size_t i = 0; i < blocks_string.size(); i++)
    {
      block_string.append(blocks_string[i]);
    }
  }
  
  if(!data.get_values("block id", block_ids))
  {   
    block_ids = CubitInterface::parse_cubit_list("block", block_string);
  }else
  {
    block_string = "";
    for (size_t i = 0; i < block_ids.size(); i++)
    {
      block_string.append(" " + std::to_string(block_ids[i]) + " ");
    }
    block_ids = CubitInterface::parse_cubit_list("block", block_string);
  }

  if (block_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }
  
  for (size_t i = 0; i < block_ids.size(); i++)
  {    
    if (!ccx_iface.create_section("SOLID",block_ids[i],material_id,options))
    {
      PRINT_ERROR("Failed!\n");
    } 
  }  
  return true;
}