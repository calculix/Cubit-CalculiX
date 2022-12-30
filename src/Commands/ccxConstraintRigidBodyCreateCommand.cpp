#include "ccxConstraintRigidBodyCreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxConstraintRigidBodyCreateCommand::ccxConstraintRigidBodyCreateCommand()
{}

ccxConstraintRigidBodyCreateCommand::~ccxConstraintRigidBodyCreateCommand()
{}

std::vector<std::string> ccxConstraintRigidBodyCreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("create constraint rigid body block ");
    
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
    
    syntax.append("vertex <value:label='vertex',help='<vertex>'>");
  
    syntax_list.push_back(syntax);
  }


  return syntax_list;
}

std::vector<std::string> ccxConstraintRigidBodyCreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create constraint rigid body block <block id> vertex <vertex id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxConstraintRigidBodyCreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxConstraintRigidBodyCreateCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<std::string> options;
  int vertex_value;
  std::string vertex;
  std::string block;
  std::vector<int> block_ids;
  int block_id_s1;
  int block_id_s2;
  std::vector<std::string> blocks_string;

  std::string block_string = " ";
  
  if (!data.get_value("vertex", vertex_value))
  {
    vertex = "";
  }
  else
  {
    vertex = std::to_string(vertex_value);
  }
  
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
    options.push_back("2"); // entity type   
    block = std::to_string(block_ids[i]);
    options.push_back(block);
    options.push_back(vertex);

    if (!ccx_iface.create_constraint("RIGIDBODY",options))
    {
      output = "Failed with Block ID " + std::to_string(block_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    }
    options.clear();
  }  
  return true;
}