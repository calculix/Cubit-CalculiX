#include "AnimationRotationRuntime.hpp"
#include "../views/CameraOperations.hpp"
#include <cmath>
namespace Postprocessor
{
  bool AnimationRotationRuntime::advance(double seconds,double pass_seconds,CameraDefinition& output,std::string& error)
  {
    error.clear();
    if (!std::isfinite(seconds) || seconds < 0 || !std::isfinite(pass_seconds) || pass_seconds <= 0 ||
        !valid_animation_rotation(settings,error))
    {
      error = "Animation rotation requires a finite elapsed time and positive pass duration.";
      return false;
    }
    const double next = phase+seconds/pass_seconds;
    std::array<double,3> angles = {0,0,0};
    for (std::size_t axis = 0; axis < settings.size(); ++axis)
    {
      if (settings[axis].enabled)
      {
        angles[axis] = settings[axis].degrees*next;
      }
    }
    if (!std::isfinite(next) || !CameraOperations::rotate_world(base,angles,output,error))
    {
      if (error.empty())
      {
        error = "Animation rotation progress overflowed.";
      }
      return false;
    }
    phase = next;
    return true;
  }
}
