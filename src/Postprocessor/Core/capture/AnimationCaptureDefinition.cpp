#include "AnimationCaptureDefinition.hpp"
namespace Postprocessor
{
  bool valid_animation_capture(const AnimationCaptureDefinition& value,std::string& error)
  {
    if (!valid_capture(value.image,error))
    {
      return false;
    }
    if (value.rotation && !valid_animation_rotation(*value.rotation,error))
    {
      return false;
    }
    if (value.image.target != CaptureTarget::View || value.first == 0 || (value.last != 0 && value.last < value.first) ||
        value.result_fps < 0 || value.result_fps > 120 || value.passes < 1 || value.passes > 1000 ||
        (value.image.background == CaptureBackground::Transparent && value.output.format != Framework::Media::AnimationFormat::Png))
    {
      error = "Animation capture requires a view, ordered frame range, 1..1000 passes and an opaque background for GIF/MP4.";
      return false;
    }
    return true;
  }
}
