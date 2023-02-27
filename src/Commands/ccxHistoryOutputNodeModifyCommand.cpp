#include "ccxHistoryOutputNodeModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxHistoryOutputNodeModifyCommand::ccxHistoryOutputNodeModifyCommand()
{}

ccxHistoryOutputNodeModifyCommand::~ccxHistoryOutputNodeModifyCommand()
{}

std::vector<std::string> ccxHistoryOutputNodeModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify historyoutput <value:label='output id',help='<output id>'> ");
  syntax.append("node ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  syntax.append("[nodeset <value:label='nodeset id',help='<nodeset id>'>] ");
  syntax.append("[frequency <value:label='frequency',help='<frequency>'>] ");
  syntax.append("[frequencyf <value:label='frequencyf',help='<frequencyf>'>] ");
  syntax.append("[{totals_yes|totals_only|totals_no} ");
  syntax.append("[{global_yes|global_no} ");
  //syntax.append("[TIME_POINTS {yes|no} ");
  syntax.append("[{key_on|key_off} [U] [NT] [TSF] [TTF] [PN] [PSF] [PTF] [MACH] [CP] [VF] [DEPF] [TURB] [MF] [RF] [RFL]]");
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxHistoryOutputNodeModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx ";
  help[0].append("modify historyoutput <output id> ");
  help[0].append("node ");
  help[0].append("[name <name>] " );
  help[0].append("[nodeset <nodeset id>] ");
  help[0].append("[frequency <frequency>] ");
  help[0].append("[frequencyf <frequencyf>] ");
  help[0].append("[{totals_yes|totals_only|totals_no} ");
  help[0].append("[{global_yes|global_no} ");
  //help[0].append("[TIME_POINTS {yes|no} ");
  help[0].append("[{key_on|key_off} [U] [NT] [TSF] [TTF] [PN] [PSF] [PTF] [MACH] [CP] [VF] [DEPF] [TURB] [MF] [RF] [RFL]]");

  return help;
}

std::vector<std::string> ccxHistoryOutputNodeModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxHistoryOutputNodeModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<std::string> options;
  std::vector<int> options_marker;
  int output_id;
  std::string name;
  int nodeset_value;
  std::string nodeset;
  int frequency_value;
  std::string frequency;
  int frequencyf_value;
  std::string frequencyf;
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

  if (!data.get_value("nodeset id", nodeset_value))
  {
    nodeset = "";
    options_marker.push_back(0);
  }
  else
  {
    nodeset = std::to_string(nodeset_value);
    options_marker.push_back(1);
  }
  options.push_back(nodeset);
  
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
    totals = "";
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
    global = "";
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
    if (data.find_keyword("U"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("NT"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("TSF"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("TTF"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("PN"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("PSF"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("PTF"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("MACH"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("CP"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("VF"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("DEPF"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("TURB"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("MF"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("RF"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("RFL"))
    {
      options.push_back("");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
  }else if (data.find_keyword("KEY_ON"))
  {
    if (data.find_keyword("U"))
    {
      options.push_back("U");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("NT"))
    {
      options.push_back("NT");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("TSF"))
    {
      options.push_back("TSF");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("TTF"))
    {
      options.push_back("TTF");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("PN"))
    {
      options.push_back("PN");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("PSF"))
    {
      options.push_back("PSF");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("PTF"))
    {
      options.push_back("PTF");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("MACH"))
    {
      options.push_back("MACH");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("CP"))
    {
      options.push_back("CP");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("VF"))
    {
      options.push_back("VF");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("DEPF"))
    {
      options.push_back("DEPF");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("TURB"))
    {
      options.push_back("TURB");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("MF"))
    {
      options.push_back("MF");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("RF"))
    {
      options.push_back("RF");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
    if (data.find_keyword("RFL"))
    {
      options.push_back("RFL");
      options_marker.push_back(1);
    }else
    {
      options.push_back("");
      options_marker.push_back(0);
    }
  }else
  {
    for (size_t i = 6; i < 21; i++)
    {
      options.push_back("");
      options_marker.push_back(0);
    }
  }
  
  if (!ccx_iface.modify_historyoutput(output_id, 1, options, options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}