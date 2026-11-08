#include "PostViewRotateCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>

PostViewRotateCommand::PostViewRotateCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostViewRotateCommand::get_syntax()
{
  return {"post view rotate <value:label='yaw'> <value:label='pitch'>"};
}

std::vector<std::string> PostViewRotateCommand::get_syntax_help()
{
  return {"post view rotate <horizontal_degrees> <vertical_degrees>"};
}

std::vector<std::string> PostViewRotateCommand::get_help()
{
  return {"Rotate the active camera: yaw about view-up, then pitch about screen-right. Angles are degrees."};
}

bool PostViewRotateCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    double yaw = 0;
    double pitch = 0;
    std::string error;
    if (!active || !data.get_value("yaw", yaw) || !data.get_value("pitch", pitch) || !active->rotate_camera(yaw, pitch, error))
    {
      PRINT_ERROR("Cannot rotate Postprocessor camera: %s\n", error.empty() ? "missing application or values" : error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot rotate Postprocessor camera: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Cannot rotate Postprocessor camera: unknown exception.\n");
    return false;
  }
}
