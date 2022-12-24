#include "ccxSectionDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxSectionDeleteCommand::ccxSectionDeleteCommand()
{}

ccxSectionDeleteCommand::~ccxSectionDeleteCommand()
{}

std::vector<std::string> ccxSectionDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete section ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='section id',help='<section id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='section id',help='<section id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <section id>...
      syntax.append("all except ");
      syntax.append("<value:label='section id except'>... ");
    }else if (syn_i==4)
    {
      // all except <section id> to <section id 2>
      syntax.append("all except ");
      syntax.append("<value:label='section id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='section id s2'>");
    }else if (syn_i==5)
    {
      // <section id> to <section id 2>
      syntax.append("<value:label='section id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='section id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }


  return syntax_list;
}

std::vector<std::string> ccxSectionDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete section <section id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxSectionDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxSectionDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> section_ids;
  int section_id_s1;
  int section_id_s2;
  std::vector<std::string> sections_string;

  std::string section_string = " ";
  
  data.get_value("section id s1", section_id_s1);
  data.get_value("section id s2", section_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("section id except", section_ids))
    {
      section_string.append("all except");
      for (size_t i = 0; i < section_ids.size(); i++)
      {
        section_string.append(" " + std::to_string(section_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    section_string.append("all except " + std::to_string(section_id_s1) + " to " + std::to_string(section_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    section_string.append(std::to_string(section_id_s1) + " to " + std::to_string(section_id_s2));
  }else if(data.get_strings("section id", sections_string))
  {
    for (size_t i = 0; i < sections_string.size(); i++)
    {
      section_string.append(sections_string[i]);
    }
  }
  
  if(!data.get_values("section id", section_ids))
  {   
    section_ids = ccx_iface.parser("section", section_string);
  }

  if (section_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }
  
  for (size_t i = 0; i < section_ids.size(); i++)
  {    
    if (!ccx_iface.delete_section(section_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(section_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}