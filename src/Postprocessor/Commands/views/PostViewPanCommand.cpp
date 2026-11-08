#include "PostViewPanCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>

PostViewPanCommand::PostViewPanCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostViewPanCommand::get_syntax()
{
  return {"post view pan <value:label='horizontal'> <value:label='vertical'>"};
}

std::vector<std::string> PostViewPanCommand::get_syntax_help()
{
  return {"post view pan <horizontal_distance> <vertical_distance>"};
}

std::vector<std::string> PostViewPanCommand::get_help()
{
  return {"Pan the active camera along screen-right/up in scene units, preserving orientation and distance."};
}

bool PostViewPanCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    double horizontal = 0;
    double vertical = 0;
    std::string error;
    if (!active || !data.get_value("horizontal", horizontal) || !data.get_value("vertical", vertical) || !active->pan_camera(horizontal, vertical, error))
    {
      PRINT_ERROR("Cannot pan Postprocessor camera: %s\n", error.empty() ? "missing application or values" : error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot pan Postprocessor camera: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Cannot pan Postprocessor camera: unknown exception.\n");
    return false;
  }
}
