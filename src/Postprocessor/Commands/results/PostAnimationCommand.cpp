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
    "post animation loop {on|off}"+source};
}
std::vector<std::string> PostAnimationCommand::get_syntax_help()
{
  return {"post animation {play|pause|fps N|loop on/off} [source \"name\"]"};
}
std::vector<std::string> PostAnimationCommand::get_help()
{
  return {"Play/pause result frames in the active view. FPS and loop are stored per source representation.",
    "Default is 10 FPS, loop off. Play from the last displayable frame restarts at the first.",
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
    auto settings = active->animation(name,error);
    if (!settings)
    {
      PRINT_ERROR("Cannot configure animation: %s\n",error.c_str());
      return false;
    }
    if (data.find_keyword("PLAY"))
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
