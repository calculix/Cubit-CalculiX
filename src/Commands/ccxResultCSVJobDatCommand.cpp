#include "ccxResultCSVJobDatCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxResultCSVJobDatCommand::ccxResultCSVJobDatCommand()
{}

ccxResultCSVJobDatCommand::~ccxResultCSVJobDatCommand()
{}

std::vector<std::string> ccxResultCSVJobDatCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result csv job <value:label='job_id',help='<job_id>'> ");
  syntax.append("dat ");
  syntax.append("block_set <string:type='unquoted', number='1', label='block_set', help='<block_set>'> " );
  syntax.append("block_type <string:type='unquoted', number='1', label='block_type', help='<block_type>'> " );
  syntax.append("block_component <string:type='unquoted', number='1', label='block_component', help='<block_component>'> " );
  syntax.append("inc_time <string:type='unquoted', number='1', label='time', help='<time>'> " );
  syntax.append("save <string:type='unquoted', number='1', label='save_filepath', help='<save_filepath>'> " );
  syntax.append("[overwrite] " );
  syntax.append("[node_id <value:label='node_id',help='<node_id>'>] ");  
  syntax.append("[element_id <value:label='element_id',help='<element_id>'>] ");  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxResultCSVJobDatCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result csv job <job_id> dat "; 
  help[0].append("block_set <block_set> " );
  help[0].append("block_type <block_type> " );
  help[0].append("block_component <block_component> " );
  help[0].append("inc_time <time> " );
  help[0].append("save <save_filepath> " );
  help[0].append("[overwrite] " );
  help[0].append("[node_id <node_id>] ");
  help[0].append("[element_id <element_id>] ");

  return help;
}

std::vector<std::string> ccxResultCSVJobDatCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResultCSVJobDatCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int job_id;

  data.get_value("job_id", job_id);

  int node_id=-1;
  int element_id=-1;
  std::string block_type="";
  std::string block_set="";
  std::string block_component="";
  std::string time = "";
  bool overwrite = false;
  std::string save_filepath;

  if (!data.get_value("node_id", node_id))
  {
    node_id = -1;
  }
  if (!data.get_value("element_id", element_id))
  {
    element_id = -1;
  }
  if (!data.get_string("block_type", block_type))
  {
    block_type = "";
  }
  if (!data.get_string("block_set", block_set))
  {
    block_set = "";
  }
  if (!data.get_string("block_component", block_component))
  {
    block_component = "";
  }
  if (!data.get_string("time", time))
  {
    time = "";
  }
  if (data.find_keyword("OVERWRITE"))
  {
    overwrite = true;
  }
  if (!data.get_string("save_filepath", save_filepath))
  {
    save_filepath = "";
  }

  //check if block_type got a whitespace at the end
  if (block_type.back()!=' ')
  {
    block_type.append(" ");
  }

  if (!ccx_iface.result_csv_job_dat(job_id,block_type,block_set,block_component,time,node_id,element_id,overwrite,save_filepath))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}
