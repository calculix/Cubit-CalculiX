#include "ccxBCsDisplacementsDeleteCommand.hpp"
#include <string>
#include <vector>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxBCsDisplacementsDeleteCommand::ccxBCsDisplacementsDeleteCommand()
{}

ccxBCsDisplacementsDeleteCommand::~ccxBCsDisplacementsDeleteCommand()
{}

std::vector<std::string> ccxBCsDisplacementsDeleteCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  for (size_t syn_i = 1; syn_i < 6; syn_i++)
  {
    std::string syntax = "ccx ";
    syntax.append("delete displacement ");

    if (syn_i==1)
    {
      syntax.append("<value:label='displacement id',help='<displacement id>'>... ");
    }
    else if (syn_i==2)
    {
      syntax.append("<string:type='unquoted',number='1',label='displacement id',help='<displacement id>'>");
      // to catch all or a quoted input string
    }
    else if (syn_i==3)
    {
      // all except <displacement id>...
      syntax.append("all except ");
      syntax.append("<value:label='displacement id except'>... ");
    }
    else if (syn_i==4)
    {
      // all except <displacement id> to <displacement id 2>
      syntax.append("all except ");
      syntax.append("<value:label='displacement id s1'> ");
      syntax.append("to ");
      syntax.append("<value:label='displacement id s2'> ");
    }
    else if (syn_i==5)
    {
      // <displacement id> to <displacement id 2>
      syntax.append("<value:label='displacement id s1'> ");
      syntax.append("to ");
      syntax.append("<value:label='displacement id s2'> ");
    }

    syntax_list.push_back(syntax);
  }

  return syntax_list;
}

std::vector<std::string> ccxBCsDisplacementsDeleteCommand::get_syntax_help()
{
  std::vector<std::string> help(5);
  help[0] = "ccx delete displacement <displacement id>";
  help[1]=" ";
  help[2]=" ";
  help[3]=" ";
  help[4]=" ";


  return help;
}

std::vector<std::string> ccxBCsDisplacementsDeleteCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxBCsDisplacementsDeleteCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXBCsInterface ccx_bcs_iface;



  std::string output;

  std::vector<int> displacement_ids;
  int displacement_id_s1 = -1;
  int displacement_id_s2 = -1;
  std::vector<int> except_ids;
  std::vector<std::string> displacements_string;

  std::string displacement_string = " ";

  data.get_value("displacement id s1", displacement_id_s1);
  data.get_value("displacement id s2", displacement_id_s2);

  // Resolve the selection using the plugin-owned displacement IDs.

  if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && !data.find_keyword("TO")))
  {
    if(data.get_values("displacement id except", except_ids))
    {
      displacement_string.append("all except");
      for (size_t i = 0; i < except_ids.size(); i++)
      {
        displacement_string.append(" " + std::to_string(except_ids[i]) + " ");
      }
    }
  }
  else if ((data.find_keyword("ALL") && data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    displacement_string.append("all except " + std::to_string(displacement_id_s1) + " to " + std::to_string(displacement_id_s2));
  }
  else if ((!data.find_keyword("ALL") && !data.find_keyword("EXCEPT") && data.find_keyword("TO")))
  {
    displacement_string.append(std::to_string(displacement_id_s1) + " to " + std::to_string(displacement_id_s2));
  }
  else if(data.get_strings("displacement id", displacements_string))
  {
    for (size_t i = 0; i < displacements_string.size(); i++)
    {
      displacement_string.append(displacements_string[i] + " ");
    }
  }

  if(!data.get_values("displacement id", displacement_ids))
  {
    displacement_ids = ccx_application_iface.parser("bcsdisplacements", displacement_string);
  }

  if (displacement_ids.size()==0)
  {
    PRINT_ERROR("No entity found.\n");
    return false;
  }

  for (size_t i = 0; i < displacement_ids.size(); i++)
  {
    if (!ccx_bcs_iface.delete_bcsdisplacements(displacement_ids[i]))
    {
      output = "Failed with ID " + std::to_string(displacement_ids[i]) + "!\n";
      PRINT_ERROR(output.c_str());
      return false;
    }
  }
  return true;
}
