#include "WorkspaceCommand.hpp"
#include "Framework/Core/FrameworkContext.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> WorkspaceCommand::get_syntax()
{
  return {"sim workspace {cubit|calculix|postprocessor|report}"};
}

std::vector<std::string> WorkspaceCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> WorkspaceCommand::get_help()
{
  return {"Switch application workspaces while retaining the Cubit model and each application's viewport."};
}

bool WorkspaceCommand::execute(CubitCommandData& data)
{
  std::string target = "cubit";

  if (data.find_keyword("calculix"))
  {
    target = "calculix";
  }
  else if (data.find_keyword("postprocessor"))
  {
    target = "postprocessor";
  }
  else if (data.find_keyword("report"))
  {
    target = "report";
  }
  std::string error;

  if (!framework_context().select_workspace(target, error))
  {
    PRINT_ERROR("Unable to switch workspace: %s\n", error.c_str());
    return false;
  }

  return true;
}
