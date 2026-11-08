#include "CameraOperations.hpp"
#include <algorithm>
#include <cmath>

namespace
{
using Vector = std::array<double, 3>;
constexpr double radians = 0.017453292519943295769;

bool finite(const Vector& vector)
{
  return std::all_of(vector.begin(), vector.end(), [](double value)
  {
    return std::isfinite(value);
  });
}

Vector cross(const Vector& a, const Vector& b)
{
  return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}

bool normalize(Vector& vector)
{
  const double length = std::hypot(vector[0], vector[1], vector[2]);
  if (!std::isfinite(length) || length <= 0)
  {
    return false;
  }
  for (auto& value : vector)
  {
    value /= length;
  }
  return true;
}

bool basis(const Postprocessor::CameraDefinition& camera, Vector& direction, Vector& right, Vector& up)
{
  for (std::size_t axis = 0; axis < 3; ++axis)
  {
    direction[axis] = camera.focal_point[axis] - camera.position[axis];
  }
  up = camera.view_up;
  if (!normalize(direction) || !normalize(up))
  {
    return false;
  }
  right = cross(direction, up);
  const double length = std::hypot(right[0], right[1], right[2]);
  if (!std::isfinite(length) || length <= 1e-12 || !normalize(right))
  {
    return false;
  }
  up = cross(right, direction);
  return normalize(up);
}

Vector rotated(const Vector& vector, const Vector& axis, double degrees)
{
  const double angle = std::remainder(degrees, 360.0) * radians;
  const double cosine = std::cos(angle);
  const double sine = std::sin(angle);
  const double dot = vector[0] * axis[0] + vector[1] * axis[1] + vector[2] * axis[2];
  const auto perpendicular = cross(axis, vector);
  Vector result;
  for (std::size_t component = 0; component < 3; ++component)
  {
    result[component] = vector[component] * cosine + perpendicular[component] * sine + axis[component] * dot * (1 - cosine);
  }
  return result;
}

bool publish(const Postprocessor::CameraDefinition& candidate, Postprocessor::CameraDefinition& output, std::string& error)
{
  if (!Postprocessor::CameraOperations::valid(candidate))
  {
    error = "The camera operation produced an invalid or non-finite view.";
    return false;
  }
  output = candidate;
  return true;
}
}

namespace Postprocessor::CameraOperations
{
bool valid(const CameraDefinition& camera)
{
  Vector direction, right, up;
  return finite(camera.position) && finite(camera.focal_point) && finite(camera.view_up) &&
    std::isfinite(camera.parallel_scale) && camera.parallel_scale > 0 &&
    std::isfinite(camera.vertical_view_angle_degrees) && camera.vertical_view_angle_degrees > 0 &&
    camera.vertical_view_angle_degrees < 180 && basis(camera, direction, right, up);
}

bool equal(const CameraDefinition& first, const CameraDefinition& second)
{
  return first.position == second.position && first.focal_point == second.focal_point &&
    first.view_up == second.view_up && first.parallel_projection == second.parallel_projection &&
    first.vertical_view_angle_degrees == second.vertical_view_angle_degrees && first.parallel_scale == second.parallel_scale;
}

bool rotate(const CameraDefinition& base, double yaw, double pitch, CameraDefinition& output, std::string& error)
{
  error.clear();
  if (!valid(base) || !std::isfinite(yaw) || !std::isfinite(pitch))
  {
    error = "Camera rotation requires a valid camera and finite angles in degrees.";
    return false;
  }
  if (std::remainder(yaw, 360.0) == 0 && std::remainder(pitch, 360.0) == 0)
  {
    output = base;
    return true;
  }
  Vector direction, right, up;
  basis(base, direction, right, up);
  Vector offset;
  for (std::size_t axis = 0; axis < 3; ++axis)
  {
    offset[axis] = base.position[axis] - base.focal_point[axis];
  }
  offset = rotated(offset, up, yaw);
  right = rotated(right, up, yaw);
  offset = rotated(offset, right, pitch);
  up = rotated(up, right, pitch);
  auto candidate = base;
  for (std::size_t axis = 0; axis < 3; ++axis)
  {
    candidate.position[axis] = base.focal_point[axis] + offset[axis];
  }
  candidate.view_up = up;
  return publish(candidate, output, error);
}

bool standard_view(const CameraDefinition& base, StandardView view, CameraDefinition& output, std::string& error)
{
  error.clear();
  if (!valid(base))
  {
    error = "Fit or set the camera before choosing a standard view.";
    return false;
  }
  const double distance = std::hypot(base.position[0] - base.focal_point[0],
    base.position[1] - base.focal_point[1], base.position[2] - base.focal_point[2]);
  auto candidate = base;
  if (view == StandardView::Orthographic)
  {
    if (!base.parallel_projection)
    {
      candidate.parallel_scale = distance * std::tan(base.vertical_view_angle_degrees * radians / 2);
      candidate.parallel_projection = true;
    }
    return publish(candidate, output, error);
  }
  Vector offset;
  candidate.view_up = {0, 0, 1};
  switch (view)
  {
    case StandardView::Front:
      offset = {0, -1, 0};
      break;
    case StandardView::Back:
      offset = {0, 1, 0};
      break;
    case StandardView::Top:
      offset = {0, 0, 1};
      candidate.view_up = {0, 1, 0};
      break;
    case StandardView::Bottom:
      offset = {0, 0, -1};
      candidate.view_up = {0, -1, 0};
      break;
    case StandardView::Right:
      offset = {1, 0, 0};
      break;
    case StandardView::Left:
      offset = {-1, 0, 0};
      break;
    case StandardView::Isometric:
      offset = {1, 1, 1};
      break;
    default:
      error = "Unknown standard camera view.";
      return false;
  }
  normalize(offset);
  for (std::size_t axis = 0; axis < 3; ++axis)
  {
    candidate.position[axis] = base.focal_point[axis] + offset[axis] * distance;
  }
  return publish(candidate, output, error);
}

bool roll(const CameraDefinition& base, double degrees, CameraDefinition& output, std::string& error)
{
  error.clear();
  if (!valid(base) || !std::isfinite(degrees))
  {
    error = "Camera spin requires a valid camera and finite angle.";
    return false;
  }
  if (std::remainder(degrees, 360.0) == 0)
  {
    output = base;
    return true;
  }
  Vector direction, right, up;
  basis(base, direction, right, up);
  auto candidate = base;
  candidate.view_up = rotated(up, direction, degrees);
  return publish(candidate, output, error);
}

bool pan(const CameraDefinition& base, double horizontal, double vertical, CameraDefinition& output, std::string& error)
{
  error.clear();
  if (!valid(base) || !std::isfinite(horizontal) || !std::isfinite(vertical))
  {
    error = "Camera pan requires a valid camera and finite scene-unit distances.";
    return false;
  }
  Vector direction, right, up;
  basis(base, direction, right, up);
  auto candidate = base;
  for (std::size_t axis = 0; axis < 3; ++axis)
  {
    const double offset = horizontal * right[axis] + vertical * up[axis];
    candidate.position[axis] += offset;
    candidate.focal_point[axis] += offset;
  }
  return publish(candidate, output, error);
}

bool pan_pixels(const CameraDefinition& base, double horizontal, double vertical, double height,
  CameraDefinition& output, std::string& error)
{
  error.clear();
  if (!valid(base) || !std::isfinite(height) || height <= 0)
  {
    error = "Pixel pan requires a valid camera and a positive viewport height.";
    return false;
  }
  double span = 2 * base.parallel_scale;
  if (!base.parallel_projection)
  {
    const double distance = std::hypot(base.position[0] - base.focal_point[0],
      base.position[1] - base.focal_point[1], base.position[2] - base.focal_point[2]);
    span = 2 * distance * std::tan(base.vertical_view_angle_degrees * radians / 2);
  }
  return pan(base, horizontal * (span / height), vertical * (span / height), output, error);
}

bool zoom(const CameraDefinition& base, double factor, CameraDefinition& output, std::string& error)
{
  error.clear();
  if (!valid(base) || !std::isfinite(factor) || factor <= 0)
  {
    error = "Camera zoom requires a valid camera and a finite positive factor.";
    return false;
  }
  if (factor == 1)
  {
    output = base;
    return true;
  }
  auto candidate = base;
  if (base.parallel_projection)
  {
    candidate.parallel_scale /= factor;
  }
  else
  {
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      candidate.position[axis] = base.focal_point[axis] + (base.position[axis] - base.focal_point[axis]) / factor;
    }
  }
  return publish(candidate, output, error);
}
}
