#include "PostViewZoomCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>

PostViewZoomCommand::PostViewZoomCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostViewZoomCommand::get_syntax()
{
  return {"post view zoom <value:label='factor'>"};
}

std::vector<std::string> PostViewZoomCommand::get_syntax_help()
{
  return {"post view zoom <factor>"};
}

std::vector<std::string> PostViewZoomCommand::get_help()
{
  return {"Zoom the active camera by a finite positive factor. Greater than one zooms in; projection mode is preserved."};
}

bool PostViewZoomCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    double factor = 0;
    std::string error;
    if (!active || !data.get_value("factor", factor) || !active->zoom_camera(factor, error))
    {
      PRINT_ERROR("Cannot zoom Postprocessor camera: %s\n", error.empty() ? "missing application or values" : error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot zoom Postprocessor camera: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Cannot zoom Postprocessor camera: unknown exception.\n");
    return false;
  }
}
