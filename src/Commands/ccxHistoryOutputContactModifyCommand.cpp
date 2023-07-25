#include "ccxHistoryOutputContactModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxHistoryOutputContactModifyCommand::ccxHistoryOutputContactModifyCommand()
{}

ccxHistoryOutputContactModifyCommand::~ccxHistoryOutputContactModifyCommand()
{}

std::vector<std::string> ccxHistoryOutputContactModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  CalculiXCoreInterface ccx_iface;
  std::vector<std::string> keys_list;

  std::string syntax = "ccx ";
  syntax.append("modify historyoutput <value:label='output id',help='<output id>'> ");
  syntax.append("contact ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[contactpair <value:label='contactpair id',help='<contactpair id>'>] ");
  syntax.append("[frequency <value:label='frequency',help='<frequency>'>] ");
  syntax.append("[{totals_yes|totals_only|totals_no}] ");
  syntax.append("[{global_yes|global_no}] ");
  //syntax.append("[TIME_POINTS {yes|no}] ");
  syntax.append("[{key_on|key_off}");

  keys_list = ccx_iface.get_historyoutput_contact_keys();
  for (size_t i = 0; i < keys_list.size(); i++)
  {
    syntax.append(" [" + keys_list[i] + "]");
  }
  
  syntax.append("]");

  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxHistoryOutputContactModifyCommand::get_syntax_help()
{
  CalculiXCoreInterface ccx_iface;
  std::vector<std::string> keys_list;

  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify historyoutput <output id> ");
  help[0].append("contact ");
  help[0].append("[name <name>] " );
  help[0].append("[contactpair <contactpair id>] ");
  help[0].append("[frequency <frequency>] ");
  help[0].append("[{totals_yes|totals_only|totals_no}] ");
  help[0].append("[{global_yes|global_no}] ");
  //help[0].append("[TIME_POINTS {yes|no}] ");
  help[0].append("[{key_on|key_off}");

  keys_list = ccx_iface.get_historyoutput_contact_keys();
  for (size_t i = 0; i < keys_list.size(); i++)
  {
    help[0].append(" [" + keys_list[i] + "]");
  }
  help[0].append("]");

  return help;
}

std::vector<std::string> ccxHistoryOutputContactModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxHistoryOutputContactModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> keys_list;
  keys_list = ccx_iface.get_historyoutput_contact_keys();
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int output_id;
  std::string name;
  int contactpair_value;
  std::string contactpair;
  int frequency_value;
  std::string frequency;
  std::string totals;
  std::string global;

  data.get_value("output id", output_id);

  if (data.get_string("name", name))
  {
    options_marker.push_back(1);
    options.push_back(name);
    if (!ccx_iface.modify_historyoutput(output_id, 0, options, options_marker))
    {
      output = "Changing Name Failed!\n";
      PRINT_ERROR(output.c_str());
    }  
    options.clear();
    options_marker.clear();
  }

  if (!data.get_value("contactpair id", contactpair_value))
  {
    contactpair = "";
    options_marker.push_back(0);
  }
  else
  {
    contactpair = std::to_string(contactpair_value);
    options_marker.push_back(1);
  }
  options.push_back(contactpair);
  
  if (!data.get_value("frequency", frequency_value))
  {
    frequency = "";
    options_marker.push_back(0);
  }
  else
  {
    frequency = std::to_string(frequency_value);
    options_marker.push_back(1);
  }
  options.push_back(frequency);
    
  if (data.find_keyword("TOTALS_YES"))
  {
    totals = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("TOTALS_ONLY"))
  {
    totals = "ONLY";
    options_marker.push_back(1);
  }else if (data.find_keyword("TOTALS_NO"))
  {
    totals = "NO";
    options_marker.push_back(1);
  }else{
    totals = "";
    options_marker.push_back(0);
  }
  options.push_back(totals);

  if (data.find_keyword("GLOBAL_YES"))
  {
    global = "YES";
    options_marker.push_back(1);
  }else if (data.find_keyword("GLOBAL_NO"))
  {
    global = "NO";
    options_marker.push_back(1);
  }else{
    global = "";
    options_marker.push_back(0);
  }
  options.push_back(global);

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
  
  if (!ccx_iface.modify_historyoutput(output_id, 3, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}