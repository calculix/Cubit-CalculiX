#include "ccxResultCSVJobFrdCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxResultCSVJobFrdCommand::ccxResultCSVJobFrdCommand()
{}

ccxResultCSVJobFrdCommand::~ccxResultCSVJobFrdCommand()
{}

std::vector<std::string> ccxResultCSVJobFrdCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result csv job <value:label='job_id',help='<job_id>'> ");
  syntax.append("frd ");
  syntax.append("{");
  syntax.append("x_node_id <value:label='x_node_id',help='<x_node_id>'>");
  syntax.append("x_block_type <string:type='unquoted', number='1', label='x_block_type', help='<x_block_type>'> " );
  syntax.append("x_block_component <string:type='unquoted', number='1', label='x_block_component', help='<x_block_component>'> " );
  syntax.append("|x_increment");
  syntax.append("|x_time");
  syntax.append("} ");
  syntax.append("{");
  syntax.append("y_node_id <value:label='y_node_id',help='<y_node_id>'>");
  syntax.append("y_block_type <string:type='unquoted', number='1', label='y_block_type', help='<y_block_type>'> " );
  syntax.append("y_block_component <string:type='unquoted', number='1', label='y_block_component', help='<y_block_component>'> " );
  syntax.append("|y_increment");
  syntax.append("|y_time");
  syntax.append("} ");
  syntax.append("[title <string:type='unquoted', number='1', label='title', help='<title>'>] " );
  syntax.append("[x_axis <string:type='unquoted', number='1', label='x_axis', help='<x_axis>'>] " );
  syntax.append("[y_axis <string:type='unquoted', number='1', label='y_axis', help='<y_axis>'>] " );
  syntax.append("[save <string:type='unquoted', number='1', label='save_filepath', help='<save_filepath>'>] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxResultCSVJobFrdCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result csv job <job_id> frd {x_node_id <x_node_id> x_block_type <x_block_type> x_block_component <x_block_type>|x_increment|x_time} {y_node_id <y_node_id> y_block_type <y_block_type> y_block_component <y_block_type>|y_increment|y_time}"; 
  help[0].append("[title <title>] " );
  help[0].append("[x_axis <x_axis>] " );
  help[0].append("[y_axis <y_axis>] " );
  help[0].append("[save <save_filepath>] " );

  return help;
}

std::vector<std::string> ccxResultCSVJobFrdCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResultCSVJobFrdCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int job_id;

  data.get_value("job_id", job_id);

  int x_node_id=-1;
  std::string x_block_type="";
  std::string x_block_component="";
  bool x_increment = false;
  bool x_time = false;
  int y_node_id=-1;
  std::string y_block_type="";
  std::string y_block_component="";
  bool y_increment = false;
  bool y_time = false;
  std::string title;
  std::string x_axis;
  std::string y_axis;
  bool save = true;
  std::string save_filepath;

  if (!data.get_value("x_node_id", x_node_id))
  {
    x_node_id = -1;
  }
  if (!data.get_string("x_block_type", x_block_type))
  {
    x_block_type = "";
  }
  if (!data.get_string("x_block_component", x_block_component))
  {
    x_block_component = "";
  }
  if (data.find_keyword("X_INCREMENT"))
  {
    x_increment = true;
  }
  if (data.find_keyword("X_TIME"))
  {
    x_time = true;
  }

  if (!data.get_value("y_node_id", y_node_id))
  {
    y_node_id = -1;
  }
  if (!data.get_string("y_block_type", y_block_type))
  {
    y_block_type = "";
  }
  if (!data.get_string("y_block_component", y_block_component))
  {
    y_block_component = "";
  }
  if (data.find_keyword("Y_INCREMENT"))
  {
    y_increment = true;
  }
  if (data.find_keyword("Y_TIME"))
  {
    y_time = true;
  }

  if (!data.get_string("title", title))
  {
    title = "";
  }
  if (!data.get_string("x_axis", x_axis))
  {
    x_axis = "";
  }
  if (!data.get_string("y_axis", y_axis))
  {
    y_axis = "";
  }
  if (!data.get_string("save_filepath", save_filepath))
  {
    save_filepath = "";
    save = false;
  }

  if (!ccx_iface.result_plot_job_frd(job_id,x_node_id,x_block_type,x_block_component,x_increment,x_time,y_node_id,y_block_type,y_block_component,y_increment,y_time,QString::fromStdString(title),QString::fromStdString(x_axis),QString::fromStdString(y_axis),save,QString::fromStdString(save_filepath)))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}