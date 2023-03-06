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
  syntax.append("[beam_type <string:type='unquoted', number='1', label='beam_type', help='<[rect|circ|block|pipe]>'>] ");
  syntax.append("[block <value:label='block id',help='<block id>'>] ");
  syntax.append("[material <value:label='material',help='<material>'>] ");
  syntax.append("[thickness1 <value:label='thickness1',help='<thickness1>'>] ");
  syntax.append("[thickness2 <value:label='thickness2',help='<thickness2>'>] ");
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
  help[0] = "ccx modify section beam <section id> [beam_type <beam_type>] [block <block id>] [material <material id>] [thickness1 <thickness1>] [thickness2 <thickness2>] [x <x>] [y <y>] [z <z>] [orientation <orientation_name>] [offset1 <offset1>] [offset2 <offset2>]";

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
  double thickness1_value;
  std::string thickness1;
  double thickness2_value;
  std::string thickness2;
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

  if (!data.get_string("beam_type", beam_type))
  {
    beam_type = "";
    options_marker.push_back(0);
  }else
  {
    options_marker.push_back(1);
  }
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
  if (!data.get_string("orientation", orientation))
  {
    orientation = "";
    options_marker.push_back(0);
  }
  else
  {
    options_marker.push_back(1);
  }
  if (!data.get_value("thickness1", thickness1_value))
  {
    thickness1 = "";
    options_marker.push_back(0);
  }
  else
  {
    thickness1 = std::to_string(thickness1_value);
    options_marker.push_back(1);
  }
  if (!data.get_value("thickness2", thickness2_value))
  {
    thickness2 = "";
    options_marker.push_back(0);
  }
  else
  {
    thickness2 = std::to_string(thickness2_value);
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
  options.push_back(thickness1);
  options.push_back(thickness2);
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