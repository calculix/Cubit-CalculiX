#ifndef POSTPROCESSOR_SELECTIONSTATE_HPP
#define POSTPROCESSOR_SELECTIONSTATE_HPP
#include "EntityReference.hpp"
#include <array>

namespace Postprocessor
{
  namespace SelectionDefaults
  {
    inline constexpr EntityKind mode = EntityKind::Node;
    inline constexpr double pick_radius_pixels = 8.0;
    inline constexpr int hover_interval_ms = 40;
    inline constexpr std::array<double,3> hover_color = {1.0,0.55,0.0};
    inline constexpr double highlight_point_size = 10.0;
    inline constexpr double highlight_line_width = 3.0;
    inline constexpr std::array<double,3> highlight_color = {0.85,0.0,0.65};
  }
  struct SelectionState
  {
    EntityKind mode = SelectionDefaults::mode;
    std::optional<EntityReference> entity;
    // Changes independently of the mesh/display revision. Host-thread only.
    std::uint64_t revision = 1;
  };
}
#endif
