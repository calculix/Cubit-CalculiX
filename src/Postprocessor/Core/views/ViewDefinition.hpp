#ifndef POSTPROCESSOR_VIEWDEFINITION_HPP
#define POSTPROCESSOR_VIEWDEFINITION_HPP

#include <array>
#include <optional>
#include <string>
#include <cstdint>
#include "../selection/SelectionState.hpp"

namespace Postprocessor
{
  struct CameraDefinition
  {
    // Positions and lengths use the view's scene coordinates. Source-to-scene
    // unit conversion must be established before overlaying different sources.
    std::array<double, 3> position = {0.0, 0.0, 1.0};
    std::array<double, 3> focal_point = {0.0, 0.0, 0.0};
    std::array<double, 3> view_up = {0.0, 1.0, 0.0};
    bool parallel_projection = false;
    double vertical_view_angle_degrees = 30.0;

    // Half the visible vertical span for parallel projection.
    double parallel_scale = 1.0;
  };

  // Defines a Postprocessor-owned 3D view, never the Cubit viewport.
  // All camera values must be finite; position and focal point must differ,
  // view_up must not be parallel to the viewing direction, scale must be positive,
  // and the perspective angle must lie strictly between 0 and 180 degrees.
  // An absent camera requests initial fitting once displayable data is available;
  // the resulting camera must be captured to reproduce the saved view.
  struct ViewDefinition
  {
    std::string id;
    std::string name;
    std::optional<CameraDefinition> camera;
    // Session-local request bookkeeping, excluded from future persistence.
    // Keep an existing camera as the orientation/projection seed for fitting.
    bool fit_pending = true;
    std::uint64_t fit_request = 0;
    std::uint64_t camera_revision = 0;
    SelectionState selection;
  };
}

#endif // POSTPROCESSOR_VIEWDEFINITION_HPP
