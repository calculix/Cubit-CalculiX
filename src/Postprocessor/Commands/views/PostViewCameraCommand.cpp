#include "PostViewCameraCommand.hpp"
#include "CameraCommandTiming.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>

PostViewCameraCommand::PostViewCameraCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostViewCameraCommand::get_syntax()
{
  return {"post view camera position <value:label='px'> <value:label='py'> <value:label='pz'> "
    "focal <value:label='fx'> <value:label='fy'> <value:label='fz'> "
    "up <value:label='ux'> <value:label='uy'> <value:label='uz'> "
    "projection {parallel|perspective} angle <value:label='angle'> scale <value:label='scale'>"};
}

std::vector<std::string> PostViewCameraCommand::get_syntax_help()
{
  return {"post view camera position <x> <y> <z> focal <x> <y> <z> up <x> <y> <z> projection {parallel|perspective} angle <degrees> scale <half-height>"};
}

std::vector<std::string> PostViewCameraCommand::get_help()
{
  return {"Set the active Postprocessor camera. Positions/scale use scene units; angle is vertical field of view in degrees. GUI camera journaling is optional; typed commands follow Cubit's journal settings."};
}

bool PostViewCameraCommand::execute(CubitCommandData& data)
{
  const CameraCommandTiming::Handler handlerTiming;
  try
  {
    const auto active = application.lock();
    Postprocessor::CameraDefinition camera;
    const char* positions[] = {"px", "py", "pz"};
    const char* focal[] = {"fx", "fy", "fz"};
    const char* up[] = {"ux", "uy", "uz"};
    bool parsed = active != nullptr;
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      parsed = data.get_value(positions[axis], camera.position[axis]) && parsed;
      parsed = data.get_value(focal[axis], camera.focal_point[axis]) && parsed;
      parsed = data.get_value(up[axis], camera.view_up[axis]) && parsed;
    }
    parsed = data.get_value("angle", camera.vertical_view_angle_degrees) && parsed;
    parsed = data.get_value("scale", camera.parallel_scale) && parsed;
    const bool parallel = data.find_keyword("PARALLEL");
    parsed = (parallel || data.find_keyword("PERSPECTIVE")) && parsed;
    camera.parallel_projection = parallel;
    std::string error;
    bool updated = false;
    if (parsed)
    {
      const CameraCommandTiming::CoreUpdate coreTiming;
      updated = active->set_camera(camera, error);
    }
    if (!parsed || !updated)
    {
      PRINT_ERROR("Cannot set Postprocessor camera: %s\n", error.empty() ? "missing application or camera values" : error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot set Postprocessor camera: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Cannot set Postprocessor camera: unknown exception.\n");
    return false;
  }
}
