#include "ccxSectionBeamCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSectionBeamCreateCommand::ccxSectionBeamCreateCommand()
{}

ccxSectionBeamCreateCommand::~ccxSectionBeamCreateCommand()
{}

std::vector<std::string> ccxSectionBeamCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("create section beam ");
    syntax.append("beam_type {rect|circ|box|pipe} ");

    syntax.append("block ");
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
    syntax.append("parameter1 <value:label='parameter1',help='<parameter1>'> "); // first line
    syntax.append("parameter2 <value:label='parameter2',help='<parameter2>'> ");
    syntax.append("parameter3 <value:label='parameter3',help='<parameter3>'> ");
    syntax.append("parameter4 <value:label='parameter4',help='<parameter4>'> ");
    syntax.append("parameter5 <value:label='parameter5',help='<parameter5>'> ");
    syntax.append("parameter6 <value:label='parameter6',help='<parameter6>'>] ");
    syntax.append("x <value:label='x',help='<x>'> "); //second line
    syntax.append("y <value:label='y',help='<y>'> ");
    syntax.append("z <value:label='z',help='<z>'> ");
    syntax.append("[orientation <value:type='orientation' ,help='<orientation_id>'>] ");
    syntax.append("[offset1 <value:label='offset1',help='<offset1>'>] ");
    syntax.append("[offset2 <value:label='offset2',help='<offset2>'>] ");
    
    syntax_list.push_back(syntax);
  }


  return syntax_list;
}

std::vector<std::string> ccxSectionBeamCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create section beam beam_type {rect|circ|box|pipe} block <block id> material <material id> parameter1 <parameter1> parameter2 <parameter2> [parameter3 <parameter3> parameter4 <parameter4> parameter5 <parameter5> parameter6 <parameter6>] x <x> y <y> z <z> [orientation <orientation_id>] [offset1 <offset1>] [offset2 <offset2>]";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxSectionBeamCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSectionBeamCreateCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string beam_type;
  int material_id;
  int orientation_value;
  std::string orientation;
  std::vector<std::string> options;
  double parameter1_value;
  std::string parameter1;
  double parameter2_value;
  std::string parameter2;
  double parameter3_value;
  std::string parameter3;
  double parameter4_value;
  std::string parameter4;
  double parameter5_value;
  std::string parameter5;
  double parameter6_value;
  std::string parameter6;
  double x_value;
  std::string x;
  double y_value;
  std::string y;
  double z_value;
  std::string z;
  double offset1_value;
  double offset2_value;
  std::string offset1;
  std::string offset2; 
  std::vector<int> block_ids;
  int block_id_s1;
  int block_id_s2;
  std::vector<std::string> blocks_string;

  std::string block_string = " ";
  
  if (data.find_keyword("RECT"))
  {
    beam_type = "RECT";
  }else if (data.find_keyword("CIRC"))
  {
    beam_type = "CIRC";
  }else if (data.find_keyword("BOX"))
  {
    beam_type = "BOX";
  }else if (data.find_keyword("PIPE"))
  {
    beam_type = "PIPE";
  }
  
  data.get_value("material", material_id);
  if (!data.get_value("orientation", orientation_value))
  {
    orientation = "";
  }
  else
  {
    orientation = std::to_string(orientation_value);
  }
  if (!data.get_value("parameter1", parameter1_value))
  {
    parameter1 = "";
  }
  else
  {
    parameter1 = std::to_string(parameter1_value);
  }
  if (!data.get_value("parameter2", parameter2_value))
  {
    parameter2 = "";
  }
  else
  {
    parameter2 = std::to_string(parameter2_value);
  }
  if (!data.get_value("parameter3", parameter3_value))
  {
    parameter3 = "";
  }
  else
  {
    parameter3 = std::to_string(parameter3_value);
  }
  if (!data.get_value("parameter4", parameter4_value))
  {
    parameter4 = "";
  }
  else
  {
    parameter4 = std::to_string(parameter4_value);
  }
  if (!data.get_value("parameter5", parameter5_value))
  {
    parameter5 = "";
  }
  else
  {
    parameter5 = std::to_string(parameter5_value);
  }
  if (!data.get_value("parameter6", parameter6_value))
  {
    parameter6 = "";
  }
  else
  {
    parameter6 = std::to_string(parameter6_value);
  }
  if (!data.get_value("x", x_value))
  {
    x = "";
  }
  else
  {
    x = std::to_string(x_value);
  }
  if (!data.get_value("y", y_value))
  {
    y = "";
  }
  else
  {
    y = std::to_string(y_value);
  }
  if (!data.get_value("z", z_value))
  {
    z = "";
  }
  else
  {
    z = std::to_string(z_value);
  }
  if (!data.get_value("offset1", offset1_value))
  {
    offset1 = "";
  }
  else
  {
    offset1 = std::to_string(offset1_value);
  }
  if (!data.get_value("offset2", offset2_value))
  {
    offset2 = "";
  }
  else
  {
    offset2 = std::to_string(offset2_value);
  }

  options.push_back(beam_type);
  options.push_back(parameter1);
  options.push_back(parameter2);
  options.push_back(parameter3);
  options.push_back(parameter4);
  options.push_back(parameter5);
  options.push_back(parameter6);
  options.push_back(x);
  options.push_back(y);
  options.push_back(z);
  options.push_back(orientation);
  options.push_back(offset1);
  options.push_back(offset2);

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
    if (!ccx_iface.create_section("BEAM",block_ids[i],material_id,options))
    {
      PRINT_ERROR("Failed!\n");
    } 
  }  
  return true;
}