#pragma once
#include "AnimationRotation.hpp"
#include "../views/ViewDefinition.hpp"
#include "Framework/Results/VtkResultsData.hpp"
#include <memory>
namespace Postprocessor
{
  // Transient playback anchor. Never serialize phase or retained source buffers.
  struct AnimationRotationRuntime
  {
    CameraDefinition base;
    AnimationRotation settings;
    std::shared_ptr<const SimResults::VtkResultsData> source;
    std::uint64_t camera_revision = 0;
    double phase = 0;
    std::size_t frames = 0;
    bool advance(double seconds,double pass_seconds,CameraDefinition& output,std::string& error);
  };
}
