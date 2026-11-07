#include "PostprocessorCommands.hpp"
#include "sources/PostOpenCommand.hpp"
#include "sources/PostListCommand.hpp"
#include "sources/PostSelectCommand.hpp"
#include "sources/PostCloseCommand.hpp"
#include "options/PostUserOptionsCommand.hpp"
#include "display/PostShowCommand.hpp"
#include "display/PostHideCommand.hpp"
#include "views/PostViewFitCommand.hpp"

#include <utility>

PostprocessorCommands::PostprocessorCommands(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostprocessorCommands::command_keys() const
{
  return {"PostOpenCommand", "PostListCommand", "PostSelectCommand", "PostCloseCommand", "PostUserOptionsCommand",
    "PostShowCommand", "PostHideCommand", "PostViewFitCommand"};
}

CubitCommand* PostprocessorCommands::create_command(const std::string& key)
{
  if (key == "PostShowCommand")
  {
    return new PostShowCommand(application);
  }
  if (key == "PostHideCommand")
  {
    return new PostHideCommand(application);
  }
  if (key == "PostViewFitCommand")
  {
    return new PostViewFitCommand(application);
  }
  if (key == "PostUserOptionsCommand")
  {
    return new PostUserOptionsCommand(application);
  }

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
