#include "PostViewFitCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>

PostViewFitCommand::PostViewFitCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostViewFitCommand::get_syntax()
{
  return {"post view fit"};
}

std::vector<std::string> PostViewFitCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> PostViewFitCommand::get_help()
{
  return {"Request fitting all visible meshes in the active Postprocessor view. The GUI completes the request when its viewport is ready."};
}

bool PostViewFitCommand::execute(CubitCommandData&)
{
  try
  {
    const auto active = application.lock();
    std::string error;
    if (!active || !active->request_camera_fit(error))
    {
      PRINT_ERROR("Cannot fit Postprocessor view: %s\n", error.empty() ? "application unavailable" : error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot fit Postprocessor view: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Postprocessor display command failed with an unknown exception.\n");
    return false;
  }
}
