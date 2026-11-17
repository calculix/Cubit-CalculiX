#pragma once
#include <array>
#include <string>
namespace Postprocessor
{
  struct AnimationAxisRotation
  {
    bool enabled = false;
    double degrees = 180.0;
  };
  using AnimationRotation = std::array<AnimationAxisRotation,3>;
  bool valid_animation_rotation(const AnimationRotation& value,std::string& error);
  bool same_animation_rotation(const AnimationRotation& first,const AnimationRotation& second);
}
