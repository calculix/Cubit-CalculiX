#include "ccxFieldOutputDeleteCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxFieldOutputDeleteCommand::ccxFieldOutputDeleteCommand()
{}

ccxFieldOutputDeleteCommand::~ccxFieldOutputDeleteCommand()
{}

std::vector<std::string> ccxFieldOutputDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;
  
  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete fieldoutput ");
    
    if (syn_i==1)
    {
      syntax.append("<value:label='output id',help='<output id>'>... ");
    }else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='output id',help='<output id>'>");
      // to catch all or an quoted input string
    }else if (syn_i==3)
    { 
      // all except <output id>...
      syntax.append("all except ");
      syntax.append("<value:label='output id except'>... ");
    }else if (syn_i==4)
    {
      // all except <output id> to <output id 2>
      syntax.append("all except ");
      syntax.append("<value:label='output id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='output id s2'>");
    }else if (syn_i==5)
    {
      // <output id> to <output id 2>
      syntax.append("<value:label='output id s1'>");
      syntax.append("to ");
      syntax.append("<value:label='output id s2'>");
    }
    
    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxFieldOutputDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete fieldoutput <output id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";
  

  return help;
}

std::vector<std::string> ccxFieldOutputDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxFieldOutputDeleteCommand::execute(CubitCommandData &data)
{
  
  CalculiXCoreInterface ccx_iface;

  std::string output;

  std::vector<int> output_ids;
  int output_id_s1;
  int output_id_s2;
  std::vector<std::string> outputs_string;

  std::string output_string = " ";
  
  data.get_value("output id s1", output_id_s1);
  data.get_value("output id s2", output_id_s2);

  //check which syntax was given and put everything into the parser

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("output id except", output_ids))
    {
      output_string.append("all except");
      for (size_t i = 0; i < output_ids.size(); i++)
      {
        output_string.append(" " + std::to_string(output_ids[i]) + " ");
      }
    }
  }else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    output_string.append("all except " + std::to_string(output_id_s1) + " to " + std::to_string(output_id_s2));
  }else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    output_string.append(std::to_string(output_id_s1) + " to " + std::to_string(output_id_s2));
  }else if(data.get_strings("output id", outputs_string))
  {
    for (size_t i = 0; i < outputs_string.size(); i++)
    {
      output_string.append(outputs_string[i]);
    }
  }
  
  if(!data.get_values("output id", output_ids))
  {   
    output_ids = ccx_iface.parser("fieldoutput", output_string);
  }

  if (output_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < output_ids.size(); i++)
  {    
    if (!ccx_iface.delete_fieldoutput(output_ids[i]))
    {
      //PRINT_INFO("%s", output.c_str());  
      output = "Failed with ID " + std::to_string(output_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
    } 
  }  
  return true;
}