#include "PostprocessorCommands.hpp"
#include "capture/PostCaptureCommand.hpp"
#include "plots/PostPlotCommand.hpp"
#include "annotations/PostLabelCommand.hpp"
#include "annotations/PostLabelsCommand.hpp"
#include "tables/PostTableCommand.hpp"
#include "tables/PostViewTypeCommand.hpp"
#include "selection/PostPickCommand.hpp"
#include "selection/PostProbeCommand.hpp"
#include "sources/PostOpenCommand.hpp"
#include "sources/PostRefreshCommand.hpp"
#include "results/PostFrameCommand.hpp"
#include "results/PostAnimationCommand.hpp"
#include "sources/PostListCommand.hpp"
#include "sources/PostSelectCommand.hpp"
#include "sources/PostCloseCommand.hpp"
#include "options/PostUserOptionsCommand.hpp"
#include "display/PostShowCommand.hpp"
#include "display/PostColorCommand.hpp"
#include "display/PostDeformCommand.hpp"
#include "display/PostLegendCommand.hpp"
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
  return {"PostCaptureCommand", "PostPlotCommand", "PostLabelCommand", "PostLabelsCommand", "PostTableCommand", "PostViewTypeCommand", "PostPickCommand", "PostProbeCommand", "PostDeformCommand", "PostAnimationCommand", "PostColorCommand", "PostLegendCommand", "PostFrameCommand", "PostOpenCommand", "PostRefreshCommand", "PostListCommand", "PostSelectCommand", "PostCloseCommand", "PostUserOptionsCommand",
    "PostShowCommand", "PostHideCommand", "PostViewFitCommand", "PostViewCameraCommand",
    "PostViewRotateCommand", "PostViewPanCommand", "PostViewZoomCommand"};
}

CubitCommand* PostprocessorCommands::create_command(const std::string& key)
{
  if (key == "PostCaptureCommand")
  {
    return new PostCaptureCommand(application);
  }
  if (key == "PostPlotCommand")
  {
    return new PostPlotCommand(application);
  }
  if (key == "PostLabelCommand")
  {
    return new PostLabelCommand(application);
  }
  if (key == "PostLabelsCommand")
  {
    return new PostLabelsCommand(application);
  }
  if (key == "PostRefreshCommand")
  {
    return new PostRefreshCommand(application);
  }
  if (key == "PostTableCommand")
  {
    return new PostTableCommand(application);
  }
  if (key == "PostViewTypeCommand")
  {
    return new PostViewTypeCommand(application);
  }
  if (key == "PostPickCommand")
  {
    return new PostPickCommand(application);
  }
  if (key == "PostProbeCommand")
  {
    return new PostProbeCommand(application);
  }
  if (key == "PostDeformCommand")
  {
    return new PostDeformCommand(application);
  }
  if (key == "PostAnimationCommand")
  {
    return new PostAnimationCommand(application);
  }
  if (key == "PostColorCommand")
  {
    return new PostColorCommand(application);
  }
  if (key == "PostLegendCommand")
  {
    return new PostLegendCommand(application);
  }
  if (key == "PostFrameCommand")
  {
    return new PostFrameCommand(application);
  }
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
