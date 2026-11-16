#pragma once
#include <string>
namespace Postprocessor
{
  namespace ScaleDefaults
  {
    inline constexpr bool visible = false;
    inline constexpr double width_pixels = 140.0;
  }
  // Largest 1/2/5 x 10^n distance not exceeding available display length.
  // Nonpositive/nonfinite input yields zero (no meaningful scale).
  double scale_interval(double available);
}
