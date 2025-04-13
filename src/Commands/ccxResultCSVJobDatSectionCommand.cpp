#include "ccxResultCSVJobDatSectionCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxResultCSVJobDatSectionCommand::ccxResultCSVJobDatSectionCommand()
{}

ccxResultCSVJobDatSectionCommand::~ccxResultCSVJobDatSectionCommand()
{}

std::vector<std::string> ccxResultCSVJobDatSectionCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result csv job <value:label='job_id',help='<job_id>'> ");
  syntax.append("dat section ");
  syntax.append("block_set <string:type='unquoted', number='1', label='block_set', help='<block_set>'> " );
  syntax.append("inc_time <string:type='unquoted', number='1', label='time', help='<time>'> " );
  syntax.append("save <string:type='unquoted', number='1', label='save_filepath', help='<save_filepath>'> " );
  syntax.append("[overwrite] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxResultCSVJobDatSectionCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result csv job <job_id> dat section "; 
  help[0].append("block_set <block_set> " );
  help[0].append("inc_time <time> " );
  help[0].append("save <save_filepath> " );
  help[0].append("[overwrite] " );

  return help;
}

std::vector<std::string> ccxResultCSVJobDatSectionCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResultCSVJobDatSectionCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int job_id;

  data.get_value("job_id", job_id);

  std::string block_set="";
  std::string time = "";
  bool overwrite = false;
  std::string save_filepath;

  if (!data.get_string("block_set", block_set))
  {
    block_set = "";
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

  if (!ccx_iface.result_csv_job_dat_section(job_id,block_set,time,overwrite,save_filepath))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}
