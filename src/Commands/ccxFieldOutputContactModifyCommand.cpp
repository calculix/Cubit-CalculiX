#include "ccxFieldOutputContactModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxFieldOutputContactModifyCommand::ccxFieldOutputContactModifyCommand()
{}

ccxFieldOutputContactModifyCommand::~ccxFieldOutputContactModifyCommand()
{}

std::vector<std::string> ccxFieldOutputContactModifyCommand::get_syntax()
{
  CalculiXCoreInterface ccx_iface;
  std::vector<std::string> keys_list;
  std::vector<std::string> syntax_list;
       
  std::string syntax = "ccx ";
  syntax.append("modify fieldoutput <value:label='output id',help='<output id>'> ");
  syntax.append("contact ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[frequency <value:label='frequency',help='<frequency>'>] ");
  //syntax.append("[TIME_POINTS {yes|no} ");
  syntax.append("[{last_iterations_on|last_iterations_off}] ");
  syntax.append("[{contact_elements_on|contact_elements_off}] ");
  syntax.append("[{key_on|key_off}");
  keys_list = ccx_iface.get_fieldoutput_contact_keys();
  for (size_t i = 0; i < keys_list.size(); i++)
  {
    syntax.append(" [" + keys_list[i] + "]");
  }
  
  syntax.append("]");
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxFieldOutputContactModifyCommand::get_syntax_help()
{
  CalculiXCoreInterface ccx_iface;
  std::vector<std::string> keys_list;
  
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify fieldoutput <output id> ");
  help[0].append("contact ");
  help[0].append("[name <name>] " );
  help[0].append("[frequency <frequency>] ");
  //help[0].append("[TIME_POINTS {yes|no} ");
  help[0].append("[{last_iterations_on|last_iterations_off}] ");
  help[0].append("[{contact_elements_on|contact_elements_off}] ");
  help[0].append("[{key_on|key_off}");

  keys_list = ccx_iface.get_fieldoutput_contact_keys();
  for (size_t i = 0; i < keys_list.size(); i++)
  {
    help[0].append(" [" + keys_list[i] + "]");
  }
  help[0].append("]");


  return help;
}

std::vector<std::string> ccxFieldOutputContactModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxFieldOutputContactModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> keys_list;
  keys_list = ccx_iface.get_fieldoutput_contact_keys();
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int output_id;
  std::string name;
  int frequency_value;
  std::string frequency;
  std::string section_forces;
  std::string last_iterations;
  std::string contact_elements;

  data.get_value("output id", output_id);

  if (data.get_string("name", name))
  {
    options_marker.push_back(1);
    options.push_back(name);
    if (!ccx_iface.modify_fieldoutput(output_id, 0, options, options_marker))
    {
      output = "Changing Name Failed!\n";
      PRINT_ERROR(output.c_str());
    }  
    options.clear();
    options_marker.clear();
  }

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
  
  // timepoints
  options.push_back("");
  options_marker.push_back(0);

  if (data.find_keyword("LAST_ITERATIONS_ON"))
  {
    last_iterations = "LAST ITERATIONS";
    options_marker.push_back(1);
  }else if (data.find_keyword("LAST_ITERATIONS_OFF"))
  {
    last_iterations = "";
    options_marker.push_back(1);
  }else{
    last_iterations = "";
    options_marker.push_back(0);
  }
  options.push_back(last_iterations);

  if (data.find_keyword("CONTACT_ELEMENTS_ON"))
  {
    contact_elements = "CONTACT ELEMENTS";
    options_marker.push_back(1);
  }else if (data.find_keyword("CONTACT_ELEMENTS_OFF"))
  {
    contact_elements = "";
    options_marker.push_back(1);
  }else{
    contact_elements = "";
    options_marker.push_back(0);
  }
  options.push_back(contact_elements);

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
    for (size_t i = 10; i < 34; i++)
    {
      options.push_back("");
      options_marker.push_back(0);
    }
  }
  
  if (!ccx_iface.modify_fieldoutput(output_id, 3, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}