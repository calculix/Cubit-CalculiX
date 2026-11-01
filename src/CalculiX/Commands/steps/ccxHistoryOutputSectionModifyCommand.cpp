#include "ccxHistoryOutputSectionModifyCommand.hpp"
#include <string>
#include <vector>
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxHistoryOutputSectionModifyCommand::ccxHistoryOutputSectionModifyCommand()
{}

ccxHistoryOutputSectionModifyCommand::~ccxHistoryOutputSectionModifyCommand()
{}

std::vector<std::string> ccxHistoryOutputSectionModifyCommand::get_syntax()
{
  CalculiXStepsInterface ccx_steps_iface;

  std::vector<std::string> syntax_list;
  std::vector<std::string> keys_list;

  std::string syntax = "ccx ";
  syntax.append("modify historyoutput <value:label='output id',help='<output id>'> ");
  syntax.append("section ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[sideset <value:label='sideset id',help='<sideset id>'>] ");
  syntax.append("[frequencyf <value:label='frequencyf',help='<frequencyf>'>] ");
  //syntax.append("[TIME_POINTS {yes|no}] ");
  syntax.append("[{key_on|key_off}");

  keys_list = ccx_steps_iface.get_historyoutput_section_keys();
  for (size_t i = 0; i < keys_list.size(); i++)
  {
    syntax.append(" [" + keys_list[i] + "]");
  }
  
  syntax.append("]");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxHistoryOutputSectionModifyCommand::get_syntax_help()
{
  CalculiXStepsInterface ccx_steps_iface;

  std::vector<std::string> keys_list;

  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify historyoutput <output id> ");
  help[0].append("section ");
  help[0].append("[name <name>] " );
  help[0].append("[sideset <sideset id>] ");
  help[0].append("[frequencyf <frequencyf>] ");
  //help[0].append("[TIME_POINTS {yes|no}] ");
  help[0].append("[{key_on|key_off}");

  keys_list = ccx_steps_iface.get_historyoutput_section_keys();
  for (size_t i = 0; i < keys_list.size(); i++)
  {
    help[0].append(" [" + keys_list[i] + "]");
  }
  help[0].append("]");

  return help;
}

std::vector<std::string> ccxHistoryOutputSectionModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxHistoryOutputSectionModifyCommand::execute(CubitCommandData &data)
{
  CalculiXStepsInterface ccx_steps_iface;


  std::string output;
  std::vector<std::string> keys_list;
  keys_list = ccx_steps_iface.get_historyoutput_section_keys();
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int output_id;
  std::string name;
  int sideset_value;
  std::string sideset;
  int frequencyf_value;
  std::string frequencyf;

  data.get_value("output id", output_id);

  if (data.get_string("name", name))
  {
    options_marker.push_back(1);
    options.push_back(name);
    if (!ccx_steps_iface.modify_historyoutput(output_id, 0, options, options_marker))
    {
      output = "Changing Name Failed!\n";
      PRINT_ERROR(output.c_str());
    }  
    options.clear();
    options_marker.clear();
  }

  if (!data.get_value("sideset id", sideset_value))
  {
    sideset = "";
    options_marker.push_back(0);
  }
  else
  {
    sideset = std::to_string(sideset_value);
    options_marker.push_back(1);
  }
  options.push_back(sideset);
  
  if (!data.get_value("frequencyf", frequencyf_value))
  {
    frequencyf = "";
    options_marker.push_back(0);
  }
  else
  {
    frequencyf = std::to_string(frequencyf_value);
    options_marker.push_back(1);
  }
  options.push_back(frequencyf);
    
  // timepoints
  options.push_back("");
  options_marker.push_back(0);

  //keys
  if (data.find_keyword("KEY_OFF"))
  {  
    for (size_t i = 0; i < keys_list.size(); i++)
    {
      if (data.find_keyword(keys_list[i]))
      {
        options.push_back("");
        options_marker.push_back(1);
      }else
      {
        options.push_back("");
        options_marker.push_back(0);
      }
    }
  }else if (data.find_keyword("KEY_ON"))
  {
    for (size_t i = 0; i < keys_list.size(); i++)
    {
      if (data.find_keyword(keys_list[i]))
      {
        options.push_back(keys_list[i]);
        options_marker.push_back(1);
      }else
      {
        options.push_back("");
        options_marker.push_back(0);
      }
    }
  }else
  {
    for (size_t i = 6; i < 23; i++)
    {
      options.push_back("");
      options_marker.push_back(0);
    }
  }
  
  if (!ccx_steps_iface.modify_historyoutput(output_id, 4, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}