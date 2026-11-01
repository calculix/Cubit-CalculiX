#include "WorkspaceCommand.hpp"
#include "Framework/Core/FrameworkContext.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> WorkspaceCommand::get_syntax()
{
  return {"fea workspace {cubit|calculix}"};
}

std::vector<std::string> WorkspaceCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> WorkspaceCommand::get_help()
{
  return {"Switch the GUI workspace while retaining the Cubit model and viewport."};
}

bool WorkspaceCommand::execute(CubitCommandData& data)
{
  const std::string target = data.find_keyword("calculix") ? "calculix" : "cubit";
  std::string error;

  if (!framework_context().select_workspace(target, error))
  {
    PRINT_ERROR("Unable to switch workspace: %s\n", error.c_str());
    return false;
  }

  return true;
}
