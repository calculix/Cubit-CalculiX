#pragma once
#include "AnimationCaptureDefinition.hpp"
namespace Postprocessor
{
  struct AnimationCapturePlan
  {
    CaptureSnapshot snapshot;
    Framework::Media::AnimationExportDefinition output;
    std::vector<std::size_t> frames;
    std::string representation_id, source_id;
    int result_fps = 10;
    bool prepare(CaptureSnapshot frozen,const std::string& source,const AnimationCaptureDefinition& request,std::string& error);
    bool sample(std::size_t index,CaptureSnapshot& result,std::string& error) const;
  };
}
