#include "ccxConstraintRigidBody2CreateCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxConstraintRigidBody2CreateCommand::ccxConstraintRigidBody2CreateCommand()
{}

ccxConstraintRigidBody2CreateCommand::~ccxConstraintRigidBody2CreateCommand()
{}

std::vector<std::string> ccxConstraintRigidBody2CreateCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("create constraint rigid body nodeset ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='nodeset id',help='<nodeset id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='nodeset id',help='<nodeset id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <nodeset id>...
      syntax.append("all except ");
      syntax.append("<value:label='nodeset id except'>... ");
    }else if (syn_i==4)
    {
      // all except <nodeset id> to <nodeset id 2>
      syntax.append("all except ");
      syntax.append("<value:label='nodeset id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='nodeset id s2'>");
    }else if (syn_i==5)
    {
      // <nodeset id> to <nodeset id 2>
      syntax.append("<value:label='nodeset id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='nodeset id s2'>");
    }
    
    syntax.append("vertex <value:label='vertex',help='<vertex>'>");
  
    syntax_list.push_back(syntax);
  }


  return syntax_list;
}

std::vector<std::string> ccxConstraintRigidBody2CreateCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx create constraint rigid body nodeset <nodeset id> vertex <vertex id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxConstraintRigidBody2CreateCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxConstraintRigidBody2CreateCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<std::string> options;
  int vertex_value;
  std::string vertex;
  std::string nodeset;
  std::vector<int> nodeset_ids;
  int nodeset_id_s1;
  int nodeset_id_s2;
  std::vector<std::string> nodesets_string;

  std::string nodeset_string = " ";
  
  if (!data.get_value("vertex", vertex_value))
  {
    vertex = "";
  }
  else
  {
    vertex = std::to_string(vertex_value);
  }
  
  data.get_value("nodeset id s1", nodeset_id_s1);
  data.get_value("nodeset id s2", nodeset_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("nodeset id except", nodeset_ids))
    {
      nodeset_string.append("all except");
      for (size_t i = 0; i < nodeset_ids.size(); i++)
      {
        nodeset_string.append(" " + std::to_string(nodeset_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    nodeset_string.append("all except " + std::to_string(nodeset_id_s1) + " to " + std::to_string(nodeset_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    nodeset_string.append(std::to_string(nodeset_id_s1) + " to " + std::to_string(nodeset_id_s2));
  }else if(data.get_strings("nodeset id", nodesets_string))
  {
    for (size_t i = 0; i < nodesets_string.size(); i++)
    {
      nodeset_string.append(nodesets_string[i]);
    }
  }
  
  if(!data.get_values("nodeset id", nodeset_ids))
  {   
    nodeset_ids = CubitInterface::parse_cubit_list("nodeset", nodeset_string);
  } 
  else
  {
    nodeset_string = "";
    for (size_t i = 0; i < nodeset_ids.size(); i++)
    {
      nodeset_string.append(" " + std::to_string(nodeset_ids[i]) + " ");
    }
    nodeset_ids = CubitInterface::parse_cubit_list("nodeset", nodeset_string);
  }
  

  if (nodeset_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }
  
  for (size_t i = 0; i < nodeset_ids.size(); i++)
  { 
    options.push_back("1"); // entity type   
    nodeset = std::to_string(nodeset_ids[i]);
    options.push_back(nodeset);
    options.push_back(vertex);

    if (!ccx_iface.create_constraint("RIGIDBODY",options))
    {
      output = "Failed with nodeset ID " + std::to_string(nodeset_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    }
    options.clear();
  }  
  return true;
}