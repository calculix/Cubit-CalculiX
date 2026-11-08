#ifndef POSTPROCESSOR_CAMERAOPERATIONS_HPP
#define POSTPROCESSOR_CAMERAOPERATIONS_HPP

#include "ViewDefinition.hpp"

namespace Postprocessor
{
  namespace NavigationDefaults
  {
    inline constexpr double drag_degrees = 180.0;
    inline constexpr double wheel_factor = 1.2;
    inline constexpr double wheel_pixels_per_step = 40.0;
    inline constexpr int recovery_interval_ms = 16;
    inline constexpr double spin_radius_fraction = 0.8;
    inline constexpr double drag_zoom_exponent = 4.0;
    inline constexpr int wheel_commit_delay_ms = 180;
  }

  namespace CameraOperations
  {
    enum class StandardView
    {
      Front, Back, Top, Bottom, Right, Left, Isometric, Orthographic
    };
    // CAD coordinates: Z up; front looks from -Y, right from +X, top from +Z.
    // Preserve focal point and distance. Orthographic preserves apparent vertical span.
    bool standard_view(const CameraDefinition& base, StandardView view, CameraDefinition& output, std::string& error);
    bool valid(const CameraDefinition& camera);
    bool equal(const CameraDefinition& first, const CameraDefinition& second);
    // Angles are degrees, right-hand positive: yaw about orthonormal view-up,
    // then pitch about the resulting right vector. Focal point/distance persist.
    bool rotate(const CameraDefinition& base, double yaw, double pitch, CameraDefinition& output, std::string& error);
    // Distances are scene units along screen-right/up; move position and focal
    // point together. Input/output may alias; failure never changes output.
    // Spin about viewing direction, keeping position and focal point fixed.
    bool roll(const CameraDefinition& base, double degrees, CameraDefinition& output, std::string& error);
    bool pan(const CameraDefinition& base, double horizontal, double vertical, CameraDefinition& output, std::string& error);
    bool pan_pixels(const CameraDefinition& base, double horizontal, double vertical, double height,
      CameraDefinition& output, std::string& error);
    // Positive factor > 1 zooms in; perspective dollies, parallel changes scale.
    bool zoom(const CameraDefinition& base, double factor, CameraDefinition& output, std::string& error);
  }
}

#endif
