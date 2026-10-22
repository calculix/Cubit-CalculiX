#include "ccxLoadsForcesDeleteCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxLoadsForcesDeleteCommand::ccxLoadsForcesDeleteCommand()
{}

ccxLoadsForcesDeleteCommand::~ccxLoadsForcesDeleteCommand()
{}

std::vector<std::string> ccxLoadsForcesDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete force ");

    if (syn_i==1)
    {
      syntax.append("<value:label='force id',help='<force id>'>... ");
    }
    else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='force id',help='<force id>'>");
      // to catch all or a quoted input string
    }
    else if (syn_i==3)
    {
      // all except <force id>...
      syntax.append("all except ");
      syntax.append("<value:label='force id except'>... ");
    }
    else if (syn_i==4)
    {
      // all except <force id> to <force id 2>
      syntax.append("all except ");
      syntax.append("<value:label='force id s1'> ");
      syntax.append("to ");
      syntax.append("<value:label='force id s2'> ");
    }
    else if (syn_i==5)
    {
      // <force id> to <force id 2>
      syntax.append("<value:label='force id s1'> ");
      syntax.append("to ");
      syntax.append("<value:label='force id s2'> ");
    }

    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxLoadsForcesDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete force <force id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";


  return help;
}

std::vector<std::string> ccxLoadsForcesDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsForcesDeleteCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXLoadsInterface ccx_loads_iface;



  std::string output;

  std::vector<int> force_ids;
  int force_id_s1 = -1;
  int force_id_s2 = -1;
  std::vector<int> except_ids;
  std::vector<std::string> forces_string;

  std::string force_string = " ";

  data.get_value("force id s1", force_id_s1);
  data.get_value("force id s2", force_id_s2);

  // Resolve the selection using the plugin-owned force IDs.

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("force id except", except_ids))
    {
      force_string.append("all except");
      for (size_t i = 0; i < except_ids.size(); i++)
      {
        force_string.append(" " + std::to_string(except_ids[i]) + " ");
      }
    }
  }
  else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    force_string.append("all except " + std::to_string(force_id_s1) + " to " + std::to_string(force_id_s2));
  }
  else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    force_string.append(std::to_string(force_id_s1) + " to " + std::to_string(force_id_s2));
  }
  else if(data.get_strings("force id", forces_string))
  {
    for (size_t i = 0; i < forces_string.size(); i++)
    {
      force_string.append(forces_string[i] + " ");
    }
  }

  if(!data.get_values("force id", force_ids))
  {
    force_ids = ccx_application_iface.parser("loadsforces", force_string);
  }

  if (force_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < force_ids.size(); i++)
  {
    if (!ccx_loads_iface.delete_loadsforces(force_ids[i]))
    {
      output = "Failed with ID " + std::to_string(force_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
      return false;
    }
  }
  return true;
}
