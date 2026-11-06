#include "PostprocessorCommands.hpp"
#include "sources/PostOpenCommand.hpp"
#include "sources/PostListCommand.hpp"
#include "sources/PostSelectCommand.hpp"
#include "sources/PostCloseCommand.hpp"

#include <utility>

PostprocessorCommands::PostprocessorCommands(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostprocessorCommands::command_keys() const
{
  return {"PostOpenCommand", "PostListCommand", "PostSelectCommand", "PostCloseCommand"};
}

CubitCommand* PostprocessorCommands::create_command(const std::string& key)
{
  if (key == "PostOpenCommand")
  {
    return new PostOpenCommand(application);
  }

  if (key == "PostListCommand")
  {
    return new PostListCommand(application);
  }

  if (key == "PostSelectCommand")
  {
    return new PostSelectCommand(application);
  }

  if (key == "PostCloseCommand")
  {
    return new PostCloseCommand(application);
  }

  return nullptr;
}
