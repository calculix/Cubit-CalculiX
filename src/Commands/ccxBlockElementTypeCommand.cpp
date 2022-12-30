#include "ccxBlockElementTypeCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxBlockElementTypeCommand::ccxBlockElementTypeCommand()
{}

ccxBlockElementTypeCommand::~ccxBlockElementTypeCommand()
{}

std::vector<std::string> ccxBlockElementTypeCommand::get_syntax()
{
  CalculiXCoreInterface ccx_iface;
  std::vector<std::string> ccx_element_types = ccx_iface.get_ccx_element_types(); 

  std::vector<std::string> syntax_list;

  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    //"block <string:type='unquoted',number='" + std::to_string(syn_i) + "',label='block id',help='<block id>'> "
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
    syntax.append("element_type ");
    syntax.append("{");
    for (size_t i = 0; i < ccx_element_types.size(); i++)
    {
      syntax.append(ccx_element_types[i]);
      if (i+1 < ccx_element_types.size())
      {
        syntax.append(" | ");
      }
      
    }
    syntax.append(":label='ccx_element_type'} ");
    syntax_list.push_back(syntax);
  }
  
  return syntax_list;
}

std::vector<std::string> ccxBlockElementTypeCommand::get_syntax_help()
{
  CalculiXCoreInterface ccx_iface;
  std::vector<std::string> ccx_element_types = ccx_iface.get_ccx_element_types(); 

  std::vector<std::string> help(5);
  help[0] = "ccx block <block_id> element_type {";
  for (size_t i = 0; i < ccx_element_types.size(); i++)
  {
    help[0].append(ccx_element_types[i]);
    if (i+1 < ccx_element_types.size())
    {
      help[0].append(" | ");
    }
  }
  help[0].append("}");

  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";

  return help;
}

std::vector<std::string> ccxBlockElementTypeCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxBlockElementTypeCommand::execute(CubitCommandData &data)
{
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
  
  //output = "\tblock string = " + block_string + "\n" ;
  //PRINT_INFO("%s", output.c_str()); 

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

  return true;
}
