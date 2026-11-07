#include "../application/PostprocessorCore.hpp"
#include "../representations/ResultMesh.hpp"
#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>

namespace
{
  bool valid_camera(const Postprocessor::CameraDefinition& camera)
  {
    const auto finite = [](const std::array<double, 3>& values)
    {
      return std::all_of(values.begin(), values.end(), [](double value)
      {
        return std::isfinite(value);
      });
    };
    if (!finite(camera.position) || !finite(camera.focal_point) || !finite(camera.view_up) ||
        !std::isfinite(camera.parallel_scale) || camera.parallel_scale <= 0 ||
        !std::isfinite(camera.vertical_view_angle_degrees) || camera.vertical_view_angle_degrees <= 0 ||
        camera.vertical_view_angle_degrees >= 180)
    {
      return false;
    }
    std::array<double, 3> direction;
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      direction[axis] = camera.focal_point[axis] - camera.position[axis];
    }
    const double length = std::hypot(direction[0], direction[1], direction[2]);
    const double upLength = std::hypot(camera.view_up[0], camera.view_up[1], camera.view_up[2]);
    if (!std::isfinite(length) || length <= 0 || !std::isfinite(upLength) || upLength <= 0)
    {
      return false;
    }
    std::array<double, 3> up;
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      direction[axis] /= length;
      up[axis] = camera.view_up[axis] / upLength;
    }
    const double cross = std::hypot(direction[1] * up[2] - direction[2] * up[1],
      direction[2] * up[0] - direction[0] * up[2], direction[0] * up[1] - direction[1] * up[0]);
    // Reject nearly parallel up/direction vectors to avoid unstable view frames.
    return cross > 1e-12;
  }
}

namespace Postprocessor
{
  bool PostprocessorCore::visible_bounds(const std::string& view_id, std::array<double, 6>& bounds, std::string& error) const
  {
    error.clear();
    if (state.views.find(view_id) == state.views.end())
    {
      error = "The target view is not registered.";
      return false;
    }
    try
    {
      bool found = false;
      std::array<double, 6> combined = {};
      for (const auto& entry : state.representations)
      {
        const auto& display = entry.second;
        if (!display.visible || display.view_id != view_id || display.input.owner != OutputOwner::Source || display.input.port != 0)
        {
          continue;
        }
        const auto data = result_data(display.input.object_id);
        if (!data || display.increment_index >= data->increments.size())
        {
          continue;
        }
        for (auto* mesh : result_meshes(data->increments[display.increment_index].dataset.get()))
        {
          std::array<double, 6> current;
          if (!mesh_bounds(mesh, current))
          {
            continue;
          }
          if (!found)
          {
            combined = current;
            found = true;
          }
          else
          {
            for (std::size_t axis = 0; axis < 3; ++axis)
            {
              combined[axis * 2] = std::min(combined[axis * 2], current[axis * 2]);
              combined[axis * 2 + 1] = std::max(combined[axis * 2 + 1], current[axis * 2 + 1]);
            }
          }
        }
      }
      if (!found)
      {
        error = "The view has no visible mesh to fit.";
        return false;
      }
      bounds = combined;
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot compute visible mesh bounds: ") + exception.what();
      return false;
    }
  }

  bool PostprocessorCore::request_camera_fit(const std::string& view_id, std::string& error)
  {
    error.clear();
    const auto found = state.views.find(view_id);
    if (source_operation_active || found == state.views.end() || found->second.fit_request == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Cannot request a fit for this view during the current operation.";
      return false;
    }
    std::array<double, 6> bounds;
    if (!visible_bounds(view_id, bounds, error))
    {
      return false;
    }
    found->second.fit_pending = true;
    ++found->second.fit_request;
    return true;
  }

  bool PostprocessorCore::complete_camera_fit(const std::string& view_id, std::uint64_t request,
    std::uint64_t revision, const CameraDefinition& camera, std::string& error)
  {
    error.clear();
    const auto found = state.views.find(view_id);
    if (source_operation_active || found == state.views.end() || !found->second.fit_pending ||
        found->second.fit_request != request || scene_revision != revision)
    {
      error = "The camera fit request is no longer current.";
      return false;
    }
    if (!valid_camera(camera))
    {
      error = "The fitted camera is invalid.";
      return false;
    }
    found->second.camera = camera;
    found->second.fit_pending = false;
    return true;
  }
}
