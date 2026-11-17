#include "AnimationRotation.hpp"
#include <cmath>
namespace Postprocessor
{
  bool valid_animation_rotation(const AnimationRotation& value,std::string& error)
  {
    error.clear();
    for (const auto& axis : value)
    {
      if (!std::isfinite(axis.degrees))
      {
        error = "Animation rotation angles must be finite degrees.";
        return false;
      }
    }
    return true;
  }
  bool same_animation_rotation(const AnimationRotation& first,const AnimationRotation& second)
  {
    for (std::size_t axis = 0; axis < first.size(); ++axis)
    {
      if (first[axis].enabled != second[axis].enabled || first[axis].degrees != second[axis].degrees)
      {
        return false;
      }
    }
    return true;
  }
}
