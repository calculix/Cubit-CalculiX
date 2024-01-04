#include "ccxResultConvertJobCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"
//#include "MeshExportInterface.hpp"
//#include "CubitEntity.hpp"

ccxResultConvertJobCommand::ccxResultConvertJobCommand()
{}

ccxResultConvertJobCommand::~ccxResultConvertJobCommand()
{}

std::vector<std::string> ccxResultConvertJobCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result convert job <value:label='job_id',help='<job_id>'> [partial] ");
  syntax.append("[block <value:label='block_ids',help='<block_ids>'>...] " );
  //syntax.append("[block <entities:type='<block:label='mBlockList'>...'>] ");
  syntax.append("[nodeset <value:label='nodeset_ids',help='<nodeset_ids>'>...] " );
  syntax.append("[sideset <value:label='sideset_ids',help='<sideset_ids>'>...] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxResultConvertJobCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result convert job <job_id> [partial] "; 
  help[0].append("[block <block_ids>...] " );
  //help[0].append("[block <entities:type='<block:label='mBlockList'>'>] ");
  help[0].append("[nodeset <nodeset_ids>...] " );
  help[0].append("[sideset <sideset_ids>...] " );

  return help;
}

std::vector<std::string> ccxResultConvertJobCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResultConvertJobCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int job_id;
  int option;
  std::vector<int> block_ids;
  std::vector<int> nodeset_ids;
  std::vector<int> sideset_ids;

  data.get_value("job_id", job_id);

  if (data.find_keyword("PARTIAL")){
    option = 1;
  }else{
    option = -1;
  }


  if (data.find_keyword("BLOCK")){
    data.get_values("block_ids", block_ids);
  }else{
    if (!((data.find_keyword("NODESET"))||(data.find_keyword("SIDESET"))))
    {
      block_ids = ccx_iface.get_blocks();
    }
  }

/*
  //std::vector<ElementOutputGroup*> block_entities;
  std::vector<CubitEntity*> block_entities;
    if (data.get_entity_list("mBlockList", block_entities)) {
        for (size_t i = 0; i < block_entities.size(); i++)
        {
          block_ids.push_back(block_entities[i]->id());
        }
  }
*/
  if (data.find_keyword("NODESET")){
    data.get_values("nodeset_ids", nodeset_ids);
  }else{
    if (!((data.find_keyword("BLOCK"))||(data.find_keyword("SIDESET"))))
    {
     nodeset_ids = CubitInterface::get_nodeset_id_list();
    }
  }

  if (data.find_keyword("SIDESET")){
    data.get_values("sideset_ids", sideset_ids);
  }else{
    if (!((data.find_keyword("NODESET"))||(data.find_keyword("BLOCK"))))
    {
     sideset_ids = CubitInterface::get_sideset_id_list();
    }
  }

  //check if ids exist! Otherwise failed!
  for (size_t i = 0; i < block_ids.size(); i++)
  {
    if (!ccx_iface.check_block_exists(block_ids[i]))
    {
      output = "Failed! Block ID " + std::to_string(block_ids[i]) + " doesn't exist!\n";
      PRINT_ERROR(output.c_str());
      return false;
    }
  }
  for (size_t i = 0; i < nodeset_ids.size(); i++)
  {
    if (!ccx_iface.check_nodeset_exists(nodeset_ids[i]))
    {
      output = "Failed! Nodeset ID " + std::to_string(nodeset_ids[i]) + " doesn't exist!\n";
      PRINT_ERROR(output.c_str());
      return false;
    }
  }
  for (size_t i = 0; i < sideset_ids.size(); i++)
  {
    if (!ccx_iface.check_sideset_exists(sideset_ids[i]))
    {
      output = "Failed! Sideset ID " + std::to_string(sideset_ids[i]) + " doesn't exist!\n";
      PRINT_ERROR(output.c_str());
      return false;
    }
  }
  
  if (!ccx_iface.convert_result(job_id,option,block_ids,nodeset_ids,sideset_ids))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
    return false;
  }
    
  return true;
}