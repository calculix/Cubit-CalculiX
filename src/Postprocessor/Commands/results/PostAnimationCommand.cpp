#include "PostAnimationCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <exception>
#include <utility>
PostAnimationCommand::PostAnimationCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}
std::vector<std::string> PostAnimationCommand::get_syntax()
{
  const std::string source = " [source <string:type='quoted',number='1',label='source_name'>]";
  return {"post animation {play|pause}"+source,
    "post animation fps <value:label='animation_fps'>"+source,
    "post animation loop {on|off}"+source,
    "post animation rotation {x|y|z} {on|off}"+source,
    "post animation rotation {x|y|z} angle <value:label='rotation_degrees'>"+source,
    "post animation advance <value:label='elapsed_seconds'>"+source};
}
std::vector<std::string> PostAnimationCommand::get_syntax_help()
{
  return {"post animation {play|pause|fps N|loop on/off} [source \"name\"]",
    "post animation rotation x|y|z {on|off|angle DEGREES} [source \"name\"]",
    "post animation advance SECONDS [source \"name\"]"};
}
std::vector<std::string> PostAnimationCommand::get_help()
{
  return {"Play/pause result frames in the active view. FPS and loop are stored per source representation.",
    "Default is 10 FPS, loop on. Axis rotation defaults off, with 180 degrees per nominal pass.",
    "Rotation uses fixed world axes through the camera focal point. Multiple axes compose X, Y, Z. Negative angles reverse direction.",
    "GUI playback requires the active Postprocessor navigation dock. Frames advance through silent commands."};
}
bool PostAnimationCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name,error;
    if (!active || (data.find_keyword("SOURCE") && (!data.get_string("source_name",name) || name.empty())))
    {
      PRINT_ERROR("Animation requires an available source.\n");
      return false;
    }
    if (data.find_keyword("ADVANCE"))
    {
      double seconds = 0;
      if (!data.get_value("elapsed_seconds",seconds) || !std::isfinite(seconds) || seconds < 0)
      {
        PRINT_ERROR("Animation elapsed time must be finite and nonnegative.\n");
        return false;
      }
      if (!active->advance_animation_rotation(name,seconds,error))
      {
        PRINT_ERROR("Cannot advance animation rotation: %s\n",error.c_str());
        return false;
      }
      return true;
    }
    auto settings = active->animation(name,error);
    if (!settings)
    {
      PRINT_ERROR("Cannot configure animation: %s\n",error.c_str());
      return false;
    }
    if (data.find_keyword("ROTATION"))
    {
      std::size_t axis = 0;
      if (data.find_keyword("Y"))
      {
        axis = 1;
      }
      else if (data.find_keyword("Z"))
      {
        axis = 2;
      }
      auto& rotation = settings->rotation[axis];
      if (data.find_keyword("ANGLE"))
      {
        if (!data.get_value("rotation_degrees",rotation.degrees) || !std::isfinite(rotation.degrees))
        {
          PRINT_ERROR("Animation rotation angles must be finite degrees.\n");
          return false;
        }
      }
      else
      {
        rotation.enabled = data.find_keyword("ON");
      }
    }
    else if (data.find_keyword("PLAY"))
    {
      settings->playing = true;
    }
    else if (data.find_keyword("PAUSE"))
    {
      settings->playing = false;
    }
    else if (data.find_keyword("LOOP"))
    {
      settings->loop = data.find_keyword("ON");
    }
    else if (data.find_keyword("FPS"))
    {
      double fps = 0;
      if (!data.get_value("animation_fps",fps) || !std::isfinite(fps) || fps != std::floor(fps) ||
          fps < Postprocessor::AnimationDefaults::minimum_fps || fps > Postprocessor::AnimationDefaults::maximum_fps)
      {
        PRINT_ERROR("Animation FPS must be an integer from 1 to 120.\n");
        return false;
      }
      settings->frames_per_second = static_cast<int>(fps);
    }
    else
    {
      PRINT_ERROR("Unknown animation operation.\n");
      return false;
    }
    if (!active->set_animation(name,*settings,error))
    {
      PRINT_ERROR("Cannot configure animation: %s\n",error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot configure animation: %s\n",exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Cannot configure animation: unknown failure.\n");
    return false;
  }
}
