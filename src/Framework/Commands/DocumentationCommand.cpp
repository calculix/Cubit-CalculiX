#include "DocumentationCommand.hpp"
#include "Framework/Core/FrameworkContext.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> DocumentationCommand::get_syntax()
{
  return {"fea documentation"};
}

std::vector<std::string> DocumentationCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> DocumentationCommand::get_help()
{
  return {"Open the installed offline Framework and workspace manual in the desktop browser."};
}

bool DocumentationCommand::execute(CubitCommandData&)
{
  std::string error;
  if (!framework_context().open_documentation(error))
  {
    PRINT_ERROR("Unable to open FEA documentation: %s\n", error.c_str());
    return false;
  }
  return true;
}
