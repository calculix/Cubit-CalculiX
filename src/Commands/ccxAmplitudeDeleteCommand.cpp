#include "ccxAmplitudeDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxAmplitudeDeleteCommand::ccxAmplitudeDeleteCommand()
{}

ccxAmplitudeDeleteCommand::~ccxAmplitudeDeleteCommand()
{}

std::vector<std::string> ccxAmplitudeDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete amplitude ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='amplitude id',help='<amplitude id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='amplitude id',help='<amplitude id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <amplitude id>...
      syntax.append("all except ");
      syntax.append("<value:label='amplitude id except'>... ");
    }else if (syn_i==4)
    {
      // all except <amplitude id> to <amplitude id 2>
      syntax.append("all except ");
      syntax.append("<value:label='amplitude id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='amplitude id s2'>");
    }else if (syn_i==5)
    {
      // <amplitude id> to <amplitude id 2>
      syntax.append("<value:label='amplitude id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='amplitude id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxAmplitudeDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete amplitude <amplitude id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxAmplitudeDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxAmplitudeDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> amplitude_ids;
  int amplitude_id_s1;
  int amplitude_id_s2;
  std::vector<std::string> amplitudes_string;

  std::string amplitude_string = " ";
  
  data.get_value("amplitude id s1", amplitude_id_s1);
  data.get_value("amplitude id s2", amplitude_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("amplitude id except", amplitude_ids))
    {
      amplitude_string.append("all except");
      for (size_t i = 0; i < amplitude_ids.size(); i++)
      {
        amplitude_string.append(" " + std::to_string(amplitude_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    amplitude_string.append("all except " + std::to_string(amplitude_id_s1) + " to " + std::to_string(amplitude_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    amplitude_string.append(std::to_string(amplitude_id_s1) + " to " + std::to_string(amplitude_id_s2));
  }else if(data.get_strings("amplitude id", amplitudes_string))
  {
    for (size_t i = 0; i < amplitudes_string.size(); i++)
    {
      amplitude_string.append(amplitudes_string[i]);
    }
  }
  
  if(!data.get_values("amplitude id", amplitude_ids))
  {   
    amplitude_ids = ccx_iface.parser("amplitude", amplitude_string);
  }

  if (amplitude_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < amplitude_ids.size(); i++)
  {    
    if (!ccx_iface.delete_amplitude(amplitude_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(amplitude_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }

  return true;
}