#ifndef POSTPROCESSOR_DISPLAYDEFAULTS_HPP
#define POSTPROCESSOR_DISPLAYDEFAULTS_HPP

#include <array>

namespace Postprocessor
{
  namespace DisplayDefaults
  {
    inline constexpr std::array<double, 3> surface_color = {0.78, 0.80, 0.83};
    inline constexpr std::array<double, 3> edge_color = {0.16, 0.18, 0.21};
    inline constexpr std::array<double, 3> initial_position = {1.0, 1.0, 1.0};
    inline constexpr std::array<double, 3> initial_view_up = {0.0, 0.0, 1.0};
    inline constexpr double edge_width = 1.0;
  }
}

#endif
