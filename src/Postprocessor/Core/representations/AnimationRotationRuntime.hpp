#pragma once
#include "AnimationRotation.hpp"
#include "../views/ViewDefinition.hpp"
#include "Framework/Results/VtkResultsData.hpp"
#include <memory>
#include <map>
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

  class AnimationRotationPlayback
  {
  public:
    // Reserve the anchor before the caller changes the view camera.
    AnimationRotationRuntime stage(const std::string& representation);
    void publish(const std::string& representation,AnimationRotationRuntime anchor);
    void erase(const std::string& representation);
    void clear() noexcept;

  private:
    std::map<std::string,AnimationRotationRuntime> anchors;
  };
}
