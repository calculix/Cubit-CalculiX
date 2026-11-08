#include "PostprocessorCommands.hpp"
#include "sources/PostOpenCommand.hpp"
#include "sources/PostListCommand.hpp"
#include "sources/PostSelectCommand.hpp"
#include "sources/PostCloseCommand.hpp"
#include "options/PostUserOptionsCommand.hpp"
#include "display/PostShowCommand.hpp"
#include "display/PostHideCommand.hpp"
#include "views/PostViewFitCommand.hpp"
#include "views/PostViewCameraCommand.hpp"
#include "views/PostViewRotateCommand.hpp"
#include "views/PostViewPanCommand.hpp"
#include "views/PostViewZoomCommand.hpp"

#include <utility>

PostprocessorCommands::PostprocessorCommands(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostprocessorCommands::command_keys() const
{
  return {"PostOpenCommand", "PostListCommand", "PostSelectCommand", "PostCloseCommand", "PostUserOptionsCommand",
    "PostShowCommand", "PostHideCommand", "PostViewFitCommand", "PostViewCameraCommand",
    "PostViewRotateCommand", "PostViewPanCommand", "PostViewZoomCommand"};
}

CubitCommand* PostprocessorCommands::create_command(const std::string& key)
{
  if (key == "PostViewCameraCommand")
  {
    return new PostViewCameraCommand(application);
  }
  if (key == "PostViewRotateCommand")
  {
    return new PostViewRotateCommand(application);
  }
  if (key == "PostViewPanCommand")
  {
    return new PostViewPanCommand(application);
  }
  if (key == "PostViewZoomCommand")
  {
    return new PostViewZoomCommand(application);
  }
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
