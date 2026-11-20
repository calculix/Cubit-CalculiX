#include "ccxConnectionHistoryCommand.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxConnectionHistoryCommand::get_syntax()
{
  std::vector<std::string> syntax;
  for (std::string kind : {"rp", "joint", "point mass", "spring"})
  {
    for (std::string target : {"all", "off", "step <value:label='step'>" })
    {
      syntax.push_back("ccx " + kind + " <value:label='id'> history " + target);
    }
  }
  return syntax;
}

std::vector<std::string> ccxConnectionHistoryCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> ccxConnectionHistoryCommand::get_help()
{
  return {"Assign history output for a reference point, joint, point mass or spring."};
}

bool ccxConnectionHistoryCommand::execute(CubitCommandData& data)
{
  int id = 0;
  int step = data.find_keyword("OFF") ? 0 : -1;
  data.get_value("id", id);
  if (data.find_keyword("STEP") && (!data.get_value("step", step) || step <= 0))
  {
    PRINT_ERROR("History assignment requires a positive step ID.\n");
    return false;
  }
  if (data.find_keyword("MASS") || data.find_keyword("SPRING"))
  {
    std::string error;
    if (!CalculiXConnectionsInterface().set_discrete_history_step(id,data.find_keyword("SPRING"),step,error))
    {
      PRINT_ERROR("%s\n",error.c_str());
      return false;
    }
    return true;
  }
  if (!CalculiXConnectionsInterface().set_history_step(id, data.find_keyword("JOINT"), step))
  {
    PRINT_ERROR("Unknown connection or step for history assignment.\n");
    return false;
  }
  return true;
}
