#include "ccxSectionCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSectionCreateCommand::ccxSectionCreateCommand()
{}

ccxSectionCreateCommand::~ccxSectionCreateCommand()
{}

std::vector<std::string> ccxSectionCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("create section solid block <value:label='block id',help='<block id>'> ");
  syntax.append("material <string:type='unquoted', number='1', label='material', help='<material_name>'> ");
  syntax.append("[orientation <string:type='unquoted', number='1', label='orientation', help='<orientation_name>'>] ");
  syntax.append("[thickness <value:label='thickness',help='<thickness>'>]");
  
  syntax_list.push_back(syntax);
  
  syntax = "ccx ";
  syntax.append("create section beam [rect|circ|block|pipe] block <value:label='block id',help='<block id>'> ");
  syntax.append("material <string:type='unquoted', number='1', label='material', help='<material_name>'> ");
  syntax.append("thickness1 <value:label='thickness1',help='<thickness1>'> "); // first line
  syntax.append("thickness2 <value:label='thickness2',help='<thickness2>'> ");
  syntax.append("x <value:label='x',help='<x>'> "); //second line
  syntax.append("y <value:label='y',help='<y>'> ");
  syntax.append("z <value:label='z',help='<z>'> ");
  syntax.append("[orientation <string:type='unquoted', number='1', label='orientation', help='<orientation_name>'>] ");
  syntax.append("[offset1 <value:label='offset1',help='<offset1>'>] ");
  syntax.append("[offset2 <value:label='offset2',help='<offset2>'>] ");
  syntax_list.push_back(syntax);
  
  
  syntax = "ccx ";
  syntax.append("create section shell block <value:label='block id',help='<block id>'> ");
  syntax.append("material <string:type='unquoted', number='1', label='material', help='<material_name>'> ");
  syntax.append("[orientation <string:type='unquoted', number='1', label='orientation', help='<orientation_name>'>] ");
  syntax.append("[thickness <value:label='thickness',help='<thickness>'>]");
  syntax.append("[offset <value:label='offset',help='<offset>'>]");
  syntax_list.push_back(syntax);
  
  
  syntax = "ccx ";
  syntax.append("create section membrane block <value:label='block id',help='<block id>'> ");
  syntax.append("material <string:type='unquoted', number='1', label='material', help='<material_name>'> ");
  syntax.append("[orientation <string:type='unquoted', number='1', label='orientation', help='<orientation_name>'>] ");
  syntax.append("[thickness <value:label='thickness',help='<thickness>'>]");
  syntax.append("[offset <value:label='offset',help='<offset>'>]");
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxSectionCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  //help[0] = "ccx create section solid block <block id> material <material>";
  /*
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  */

  return help;
}

std::vector<std::string> ccxSectionCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSectionCreateCommand::execute(CubitCommandData &data)
{
  /*
  CalculiXCoreInterface ccx_iface;

  std::vector<std::string> ccx_element_types = ccx_iface.get_ccx_element_types(); 

  std::string element_type;
  std::string output;  

  std::vector<int> block_ids;
  int block_id_s1;
  int block_id_s2;
  std::vector<std::string> blocks_string;

  std::string block_string = " ";

  data.get_value("block id s1", block_id_s1);
  data.get_value("block id s2", block_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("block id", block_ids))
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
  }else if(data.get_values("block id", block_ids))
  {
    for (size_t i = 0; i < block_ids.size(); i++)
    {
      block_string.append(" " + std::to_string(block_ids[i]) + " ");
    }
  }
  
  //output = "\tblock string = " + block_string + "\n" ;
  //PRINT_INFO("%s", output.c_str()); 

  block_ids = CubitInterface::parse_cubit_list("block", block_string);
 
  if (block_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < ccx_element_types.size(); i++)
  {
    if (data.find_keyword(ccx_element_types[i]))
    {
      element_type = ccx_element_types[i];
    } 
  }
  
  for (size_t i = 0; i < block_ids.size(); i++)
  {
    output = "\ttry to set element type " + element_type + " on block " + std::to_string(block_ids[i]) + "\n" ;

    PRINT_INFO("%s", output.c_str());  

    if (ccx_iface.set_ccx_element_type(block_ids[i], element_type))
    {
      output = "\tSuccessful!\n" ;
    }else{
      output = "\tFailed!\n" ;
    }
    PRINT_INFO("%s", output.c_str());
  }
  */

  std::string output;

  if (data.find_keyword("BEAM")){
    output = "\tBEAM\n";
    if (data.find_keyword("ORIENTATION")){
      output.append("\tORIENTATION\n");
    }
    if (data.find_keyword("OFFSET1")){
      output.append("\tOFFSET1\n");
    }
    if (data.find_keyword("OFFSET2")){
      output.append("\tOFFSET2\n");
    }
  }else if (data.find_keyword("MEMBRANE"))
  {
    output = "\tMEMBRANE\n";
    if (data.find_keyword("ORIENTATION")){
      output.append("\tORIENTATION\n");
    }
    if (data.find_keyword("THICKNESS")){
      output.append("\tTHICKNESS\n");
    }
    if (data.find_keyword("OFFSET")){
      output.append("\tOFFSET\n");
    }
  }else if (data.find_keyword("SHELL"))
  {
    output = "\tSHELL\n";
    if (data.find_keyword("ORIENTATION")){
      output.append("\tORIENTATION\n");
    }
    if (data.find_keyword("THICKNESS")){
      output.append("\tTHICKNESS\n");
    }
    if (data.find_keyword("OFFSET")){
      output.append("\tOFFSET\n");
    }
  }else if (data.find_keyword("SOLID"))
  {
    output = "\tSOLID\n";
    if (data.find_keyword("ORIENTATION")){
      output.append("\tORIENTATION\n");
    }
    if (data.find_keyword("THICKNESS")){
      output.append("\tTHICKNESS\n");
    }
  }
  
  PRINT_INFO("%s", output.c_str());

  return true;
}
