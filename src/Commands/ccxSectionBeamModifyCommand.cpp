#include "ccxSectionBeamModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSectionBeamModifyCommand::ccxSectionBeamModifyCommand()
{}

ccxSectionBeamModifyCommand::~ccxSectionBeamModifyCommand()
{}

std::vector<std::string> ccxSectionBeamModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify section beam <value:label='section id',help='<section id>'>");
  syntax.append("[beam_type {rect|circ|box|pipe}] ");
  syntax.append("[block <value:label='block id',help='<block id>'>] ");
  syntax.append("[material <value:label='material',help='<material>'>] ");
  syntax.append("[parameter1 <value:label='parameter1',help='<parameter1>'>] ");
  syntax.append("[parameter2 <value:label='parameter2',help='<parameter2>'>] ");
  syntax.append("[parameter3 <value:label='parameter3',help='<parameter3>'>] ");
  syntax.append("[parameter4 <value:label='parameter4',help='<parameter4>'>] ");
  syntax.append("[parameter5 <value:label='parameter5',help='<parameter5>'>] ");
  syntax.append("[parameter6 <value:label='parameter6',help='<parameter6>'>] ");
  syntax.append("[x <value:label='x',help='<x>'>] ");
  syntax.append("[y <value:label='y',help='<y>'>] ");
  syntax.append("[z <value:label='z',help='<z>'>] ");
  syntax.append("[orientation <string:type='unquoted', number='1', label='orientation', help='<orientation_name>'>] ");
  syntax.append("[offset1 <value:label='offset1',help='<offset1>'>] ");
  syntax.append("[offset2 <value:label='offset2',help='<offset2>'>] ");
    
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSectionBeamModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx modify section beam <section id> [beam_type {rect|circ|box|pipe}] [block <block id>] [material <material id>] [parameter1 <parameter1>] [parameter2 <parameter2>] [parameter3 <parameter3>] [parameter4 <parameter4>] [parameter5 <parameter5>] [parameter6 <parameter6>] [x <x>] [y <y>] [z <z>] [orientation <orientation_name>] [offset1 <offset1>] [offset2 <offset2>]";

  return help;
}

std::vector<std::string> ccxSectionBeamModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSectionBeamModifyCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::string beam_type;
  int material_value;
  std::string material;
  std::string orientation;
  std::vector<std::string> options;
  std::vector<int> options_marker;
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
  int block_id_value;
  std::string block_id;
  int section_id;

  data.get_value("section id", section_id);

  if (!data.get_value("block id", block_id_value))
  {
    block_id = "";
    options_marker.push_back(0);
  }
  else
  {
    block_id = std::to_string(block_id_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("material", material_value))
  {
    material = "";
    options_marker.push_back(0);
  }
  else
  {
    material = std::to_string(material_value);
    options_marker.push_back(1);
  }

  if (!data.find_keyword("BEAM_TYPE"))
  {
    beam_type = "";
    options_marker.push_back(0);
  }else
  {
    options_marker.push_back(1);
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
  }

  if (!data.get_value("parameter1", parameter1_value))
  {
    parameter1 = "";
    options_marker.push_back(0);
  }
  else
  {
    parameter1 = std::to_string(parameter1_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("parameter2", parameter2_value))
  {
    parameter2 = "";
    options_marker.push_back(0);
  }
  else
  {
    parameter2 = std::to_string(parameter2_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("parameter3", parameter3_value))
  {
    parameter3 = "";
    options_marker.push_back(0);
  }
  else
  {
    parameter3 = std::to_string(parameter3_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("parameter4", parameter4_value))
  {
    parameter4 = "";
    options_marker.push_back(0);
  }
  else
  {
    parameter4 = std::to_string(parameter4_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("parameter5", parameter5_value))
  {
    parameter5 = "";
    options_marker.push_back(0);
  }
  else
  {
    parameter5 = std::to_string(parameter5_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("parameter6", parameter6_value))
  {
    parameter6 = "";
    options_marker.push_back(0);
  }
  else
  {
    parameter6 = std::to_string(parameter6_value);
    options_marker.push_back(1);
  }

  if (!data.get_value("x", x_value))
  {
    x = "";
    options_marker.push_back(0);
  }
  else
  {
    x = std::to_string(x_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("y", y_value))
  {
    y = "";
    options_marker.push_back(0);
  }
  else
  {
    y = std::to_string(y_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("z", z_value))
  {
    z = "";
    options_marker.push_back(0);
  }
  else
  {
    z = std::to_string(z_value);
    options_marker.push_back(1);
  }

  if (!data.get_string("orientation", orientation))
  {
    orientation = "";
    options_marker.push_back(0);
  }
  else
  {
    options_marker.push_back(1);
  }

  if (!data.get_value("offset1", offset1_value))
  {
    offset1 = "";
    options_marker.push_back(0);
  }
  else
  {
    offset1 = std::to_string(offset1_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("offset2", offset2_value))
  {
    offset2 = "";
    options_marker.push_back(0);
  }
  else
  {
    offset2 = std::to_string(offset2_value);
    options_marker.push_back(1);
  }


  options.push_back(block_id);
  options.push_back(material);  
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

  if (!ccx_iface.modify_section("BEAM",section_id,options,options_marker))
  {
    output = "Failed with ID " + std::to_string(section_id) + "!\n";
    PRINT_ERROR(output.c_str());
    return false;
  } 

  return true;
}