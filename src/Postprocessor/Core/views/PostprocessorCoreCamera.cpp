#include "../application/PostprocessorCore.hpp"
#include "CameraOperations.hpp"
#include <limits>

namespace Postprocessor
{
bool PostprocessorCore::camera_navigation_ready(const std::string& view_id) const noexcept
{
  const auto found = state.views.find(view_id);
  return !source_operation_active && found != state.views.end() && found->second.camera && !found->second.fit_pending;
}

bool PostprocessorCore::set_camera(const std::string& view_id, const CameraDefinition& camera, std::string& error)
{
  error.clear();
  const auto found = state.views.find(view_id);
  if (source_operation_active || found == state.views.end())
  {
    error = "Cannot set the camera of a missing view or during a source operation.";
    return false;
  }
  if (!CameraOperations::valid(camera))
  {
    error = "Camera values are invalid, non-finite or form a degenerate view frame.";
    return false;
  }
  auto& view = found->second;
  if (view.camera && !view.fit_pending && CameraOperations::equal(*view.camera, camera))
  {
    return true;
  }
  if (view.camera_revision == std::numeric_limits<std::uint64_t>::max() || view.fit_request == std::numeric_limits<std::uint64_t>::max())
  {
    error = "Camera revision identifiers are exhausted.";
    return false;
  }
  view.camera = camera;
  view.fit_pending = false;
  ++view.camera_revision;
  ++view.fit_request;
  return true;
}

bool PostprocessorCore::rotate_camera(const std::string& view_id, double yaw, double pitch, std::string& error)
{
  const auto found = state.views.find(view_id);
  if (found == state.views.end() || !found->second.camera)
  {
    error = "Fit or set the view camera before rotating.";
    return false;
  }
  CameraDefinition candidate;
  return CameraOperations::rotate(*found->second.camera, yaw, pitch, candidate, error) && set_camera(view_id, candidate, error);
}

bool PostprocessorCore::pan_camera(const std::string& view_id, double horizontal, double vertical, std::string& error)
{
  const auto found = state.views.find(view_id);
  if (found == state.views.end() || !found->second.camera)
  {
    error = "Fit or set the view camera before panning.";
    return false;
  }
  CameraDefinition candidate;
  return CameraOperations::pan(*found->second.camera, horizontal, vertical, candidate, error) && set_camera(view_id, candidate, error);
}

bool PostprocessorCore::zoom_camera(const std::string& view_id, double factor, std::string& error)
{
  const auto found = state.views.find(view_id);
  if (found == state.views.end() || !found->second.camera)
  {
    error = "Fit or set the view camera before zooming.";
    return false;
  }
  CameraDefinition candidate;
  return CameraOperations::zoom(*found->second.camera, factor, candidate, error) && set_camera(view_id, candidate, error);
}
}
