#pragma once
#include "CaptureDefinition.hpp"
#include "Framework/Media/AnimationExportDefinition.hpp"
namespace Postprocessor
{
  struct AnimationCaptureDefinition
  {
    CaptureDefinition image;
    Framework::Media::AnimationExportDefinition output;
    std::string source;
    std::size_t first = 1, last = 0;
    int result_fps = 0;
    int passes = 1;
    std::optional<AnimationRotation> rotation;
  };
  bool valid_animation_capture(const AnimationCaptureDefinition& value,std::string& error);
}
