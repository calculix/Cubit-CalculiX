#include "ccxUserOptionsCommand.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxUserOptionsCommand::get_syntax()
{
  return {"ccx useroptions apply <string:type='unquoted',number='1',label='payload',help='<encoded options>'>"};
}

std::vector<std::string> ccxUserOptionsCommand::get_syntax_help()
{
  return {"ccx useroptions apply <payload>"};
}

std::vector<std::string> ccxUserOptionsCommand::get_help()
{
  return {"Apply and persist a complete CalculiX user-options object. The options panel produces the replayable payload."};
}

bool ccxUserOptionsCommand::execute(CubitCommandData& data)
{
  std::string payload;
  std::string error;
  if (!data.get_string("payload", payload) ||
      !CalculiXApplicationInterface().apply_user_options(payload, error))
  {
    PRINT_ERROR("Cannot apply CalculiX user options: %s\n",
      error.empty() ? "missing payload" : error.c_str());
    return false;
  }
  return true;
}
