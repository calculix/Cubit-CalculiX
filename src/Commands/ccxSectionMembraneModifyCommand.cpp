#include "ccxSectionMembraneModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSectionMembraneModifyCommand::ccxSectionMembraneModifyCommand()
{}

ccxSectionMembraneModifyCommand::~ccxSectionMembraneModifyCommand()
{}

std::vector<std::string> ccxSectionMembraneModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify section membrane <value:label='section id',help='<section id>'>");
  syntax.append("[block <value:label='block id',help='<block id>'>] ");
  syntax.append("[material <value:label='material',help='<material>'>] ");
  syntax.append("[orientation <value:type='orientation' ,help='<orientation_id>'>] ");
  syntax.append("[thickness <value:label='thickness',help='<thickness>'>]");
  syntax.append("[offset <value:label='offset',help='<offset>'>]");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxSectionMembraneModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx modify section membrane <section id> [block <block id>] [material <material id>] [orientation <orientation_id>] [thickness <thickness>] [offset <offset>]";

  return help;
}

std::vector<std::string> ccxSectionMembraneModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSectionMembraneModifyCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int material_value;
  std::string material;
  int orientation_value;
  std::string orientation;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  double thickness_value;
  std::string thickness;
  double offset_value;
  std::string offset; 
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
  
  if (!data.get_value("orientation", orientation_value))
  {
    orientation = "";
    options_marker.push_back(0);
  }
  else
  {
    orientation = std::to_string(orientation_value);
    options_marker.push_back(1);
  }
  
  if (!data.get_value("thickness", thickness_value))
  {
    thickness = "";
    options_marker.push_back(0);
  }
  else
  {
    thickness = std::to_string(thickness_value);
    options_marker.push_back(1);
  }

  if (!data.get_value("offset", offset_value))
  {
    offset = "";
    options_marker.push_back(0);
  }
  else
  {
    offset = std::to_string(offset_value);
    options_marker.push_back(1);
  }

  options.push_back(block_id);
  options.push_back(material);  
  options.push_back(orientation);
  options.push_back(thickness);
  options.push_back(offset);

  if (!ccx_iface.modify_section("MEMBRANE",section_id,options,options_marker))
  {
    output = "Failed with ID " + std::to_string(section_id) + "!\n";
    PRINT_ERROR(output.c_str());
    return false;
  } 

  return true;
}