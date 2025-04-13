#include "ccxResultPlotJobDatSectionCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxResultPlotJobDatSectionCommand::ccxResultPlotJobDatSectionCommand()
{}

ccxResultPlotJobDatSectionCommand::~ccxResultPlotJobDatSectionCommand()
{}

std::vector<std::string> ccxResultPlotJobDatSectionCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("result plot job <value:label='job_id',help='<job_id>'> ");
  syntax.append("dat section ");
  syntax.append("{");
  syntax.append("x_section_set <string:type='unquoted', number='1', label='x_section_set', help='<x_section_set>'> " );
  syntax.append("x_section_component <string:type='unquoted', number='1', label='x_section_component', help='<x_section_component>'> " );
  syntax.append("|x_time");
  syntax.append("} ");
  syntax.append("{");
  syntax.append("y_section_set <string:type='unquoted', number='1', label='y_section_set', help='<y_section_set>'> " );
  syntax.append("y_section_component <string:type='unquoted', number='1', label='y_section_component', help='<y_section_component>'> " );
  syntax.append("|y_time");
  syntax.append("} ");
  syntax.append("[title <string:type='unquoted', number='1', label='title', help='<title>'>] " );
  syntax.append("[x_axis <string:type='unquoted', number='1', label='x_axis', help='<x_axis>'>] " );
  syntax.append("[y_axis <string:type='unquoted', number='1', label='y_axis', help='<y_axis>'>] " );
  syntax.append("[save <string:type='unquoted', number='1', label='save_filepath', help='<save_filepath>'>] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxResultPlotJobDatSectionCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx result plot job <job_id> dat section ";
  help[0].append("{x_section_set <x_section_set> x_section_component <x_section_type>|x_time} ");
  help[0].append("{y_section_set <y_section_set> y_section_component <y_section_type>|y_time}"); 
  help[0].append("[title <title>] " );
  help[0].append("[x_axis <x_axis>] " );
  help[0].append("[y_axis <y_axis>] " );
  help[0].append("[save <save_filepath>] " );

  return help;
}

std::vector<std::string> ccxResultPlotJobDatSectionCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxResultPlotJobDatSectionCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;

  int job_id;

  data.get_value("job_id", job_id);

  std::string x_section_set="";
  std::string x_section_component="";
  bool x_time = false;
  std::string y_section_set="";
  std::string y_section_component="";
  bool y_time = false;
  std::string title;
  std::string x_axis;
  std::string y_axis;
  bool save = true;
  std::string save_filepath;

  if (!data.get_string("x_section_set", x_section_set))
  {
    x_section_set = "";
  }
  if (!data.get_string("x_section_component", x_section_component))
  {
    x_section_component = "";
  }
  if (data.find_keyword("X_TIME"))
  {
    x_time = true;
  }

  if (!data.get_string("y_section_set", y_section_set))
  {
    y_section_set = "";
  }
  if (!data.get_string("y_section_component", y_section_component))
  {
    y_section_component = "";
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

  if (!ccx_iface.result_plot_job_dat_section(job_id,x_section_set,x_section_component,x_time,y_section_set,y_section_component,y_time,QString::fromStdString(title),QString::fromStdString(x_axis),QString::fromStdString(y_axis),save,QString::fromStdString(save_filepath)))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
    
  return true;
}