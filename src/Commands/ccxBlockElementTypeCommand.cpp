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

  //for (size_t syn_i = 1; syn_i < 20; syn_i++)
  //{
    std::string syntax = "ccx " 
    //"block <string:type='unquoted',number='" + std::to_string(syn_i) + "',label='block id',help='<block id>'> "
    "block <string:type='unquoted',number='1',label='block id',help='<block id>'> "
    "element_type "
    "{";
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
  //}
  
  return syntax_list;
}

std::vector<std::string> ccxBlockElementTypeCommand::get_syntax_help()
{
  std::vector<std::string> help;
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

  std::vector<int> block_ids;
  std::vector<std::string> blocks_string;

  std::string block_string;
  //data.get_string("block id", block_string);
  data.get_strings("block id", blocks_string);
  for (size_t i = 0; i < blocks_string.size(); i++)
  {
    block_string.append(" "+std::to_string(i) + " ");
    block_string.append(blocks_string[i]);
  }

  std::string output;  

  output = "\tblock string " + block_string + "\n" ;
  PRINT_INFO("%s", output.c_str()); 

  block_ids = CubitInterface::parse_cubit_list("block", block_string);

    
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
