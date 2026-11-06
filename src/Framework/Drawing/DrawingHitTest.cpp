#include "DrawingHitTest.hpp"
#include "GView.hpp"
#include "GCubitModelSet.hpp"
#include "GGeomPicker.hpp"
#include "GMeshPicker.hpp"
#include "SVSelection.hpp"
#include "SVPickUtil.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

namespace
{
using Point = std::array<double, 3>;
constexpr double PixelTolerance = 6.0;

bool project(GView& view, const std::vector<double>& world, Point& result)
{
  if (world.size() != 3)
  {
    return false;
  }
  double display[4] = {};
  view.world_to_display(world[0], world[1], world[2], display);
  result = {display[0], display[1], display[2]};
  return std::all_of(result.begin(), result.end(), [](double value) { return std::isfinite(value); }) && result[2] >= 0 && result[2] <= 1;
}

double distanceSquared(const Point& point, double x, double y)
{
  return (point[0] - x) * (point[0] - x) + (point[1] - y) * (point[1] - y);
}

Point closestPoint(const Point& a, const Point& b, double x, double y)
{
  const double dx = b[0] - a[0];
  const double dy = b[1] - a[1];
  const double length = dx * dx + dy * dy;
  const double t = length > 0 ? std::clamp(((x - a[0]) * dx + (y - a[1]) * dy) / length, 0.0, 1.0) : 0;
  return {a[0] + t * dx, a[1] + t * dy, a[2] + t * (b[2] - a[2])};
}

bool polygonPoint(const std::vector<Point>& points, double x, double y, Point& result)
{
  bool inside = false;
  for (std::size_t i = 0, j = points.size() - 1; i < points.size(); j = i++)
  {
    const auto& a = points[i];
    const auto& b = points[j];
    if ((a[1] > y) != (b[1] > y) && x < (b[0] - a[0]) * (y - a[1]) / (b[1] - a[1]) + a[0])
    {
      inside = !inside;
    }
  }
  if (!inside)
  {
    return false;
  }
  // Draw polygons are planar. Display depth is affine across their projected plane.
  const auto& a = points.front();
  for (std::size_t i = 1; i + 1 < points.size(); ++i)
  {
    const auto& b = points[i];
    const auto& c = points[i + 1];
    const double determinant = (b[1] - c[1]) * (a[0] - c[0]) + (c[0] - b[0]) * (a[1] - c[1]);
    if (std::abs(determinant) > 1e-12)
    {
      const double u = ((b[1] - c[1]) * (x - c[0]) + (c[0] - b[0]) * (y - c[1])) / determinant;
      const double v = ((c[1] - a[1]) * (x - c[0]) + (a[0] - c[0]) * (y - c[1])) / determinant;
      result = {x, y, u * a[2] + v * b[2] + (1 - u - v) * c[2]};
      return true;
    }
  }
  return false;
}

// Probe native occluders without ending a native pick or changing its selected entities.
// The SDK exposes these temporary buffers explicitly; restore them even on failure.
class NativeBufferGuard
{
public:
  explicit NativeBufferGuard(GPicker& picker) : picker(picker)
  {
    previous.swap(picker.get_selection_buffer());
  }
  ~NativeBufferGuard()
  {
    picker.get_selection_buffer().swap(previous);
  }
private:
  GPicker& picker;
  std::set<EntitySelection> previous;
};

class NativeFilterGuard
{
public:
  NativeFilterGuard()
  {
    int count = SVSelection::get_number_pick_types();
    filters.resize(static_cast<std::size_t>((std::max)(count, 1)));
    SVSelection::get_pick_filters(filters.data(), count);
    filters.resize(static_cast<std::size_t>((std::max)(count, 0)));
    const char* surface[] = {GGeomPicker::gSurfaceType};
    SVSelection::set_filter_types(1, surface);
  }
  ~NativeFilterGuard()
  {
    SVSelection::set_filter_types(filters.size(), filters.data());
  }
private:
  std::vector<const char*> filters;
};

bool bufferOccludes(GView& view, GPicker& picker, double depth)
{
  for (const auto& hit : picker.get_selection_buffer())
  {
    Point display;
    const auto& p = hit.mPickIntersection;
    if (project(view, {p[0], p[1], p[2]}, display) && display[2] < depth - 1e-5)
    {
      return true;
    }
  }
  return false;
}

bool occluded(GView& view, const DrawingPickHit& hit)
{
  auto* models = dynamic_cast<GCubitModelSet*>(view.get_model_set());
  if (models == nullptr)
  {
    return true;
  }
  const float x = static_cast<float>(hit.display[0]);
  const float y = static_cast<float>(hit.display[1]);
  if (models->get_mesh_posted() && models->get_mesh_picker() != nullptr)
  {
    auto& picker = *models->get_mesh_picker();
    NativeBufferGuard guard(picker);
    auto info = SVPickUtil::pick_info(view.get_renderer());
    picker.compute_pick(x, y, false, info);
    if (bufferOccludes(view, picker, hit.display[2]))
    {
      return true;
    }
  }
  if (models->get_geometry_posted() && models->get_geom_picker() != nullptr)
  {
    auto& picker = *models->get_geom_picker();
    NativeFilterGuard filters;
    NativeBufferGuard guard(picker);
    auto info = SVPickUtil::pick_info(view.get_renderer());
    picker.compute_pick(x, y, info);
    if (bufferOccludes(view, picker, hit.display[2]))
    {
      return true;
    }
  }
  return false;
}
}

bool DrawingHitTest::pick(GView& view, float x, float y, const DrawingPickSnapshot& snapshot, DrawingPickHit& result) const
{
  std::vector<DrawingPickHit> candidates;
  for (std::size_t index = 0; index < snapshot.entities.size(); ++index)
  {
    const auto& entity = snapshot.entities[index];
    for (const auto& primitive : entity.geometry.primitives)
    {
      if (primitive.kind == DrawingPrimitiveKind::Label)
      {
        continue;
      }
      std::vector<Point> points;
      for (const auto& world : primitive.points)
      {
        Point display;
        if (!project(view, world, display))
        {
          points.clear();
          break;
        }
        points.push_back(display);
      }
      if (points.empty())
      {
        continue;
      }
      Point closest = points.front();
      if (primitive.kind == DrawingPrimitiveKind::Polygon && points.size() >= 3 && polygonPoint(points, x, y, closest))
      {
        // Interior hit; retain the interpolated depth.
      }
      else
      {
        const std::size_t edges = primitive.kind == DrawingPrimitiveKind::Polygon ? points.size() : points.size() - 1;
        for (std::size_t edge = 0; edge < edges; ++edge)
        {
          const auto point = closestPoint(points[edge], points[(edge + 1) % points.size()], x, y);
          if (distanceSquared(point, x, y) < distanceSquared(closest, x, y))
          {
            closest = point;
          }
        }
      }
      const double distance = distanceSquared(closest, x, y);
      if (distance > PixelTolerance * PixelTolerance)
      {
        continue;
      }
      double world[4] = {};
      view.display_to_world(closest[0], closest[1], closest[2], world);
      if (!std::isfinite(world[3]) || std::abs(world[3]) < 1e-15)
      {
        continue;
      }
      DrawingPickHit hit;
      hit.owner = entity.owner;
      hit.display = closest;
      hit.distanceSquared = distance;
      hit.handle = index + 1;
      hit.world = {world[0] / world[3], world[1] / world[3], world[2] / world[3]};
      candidates.push_back(hit);
    }
  }
  std::stable_sort(candidates.begin(), candidates.end(), [](const DrawingPickHit& a, const DrawingPickHit& b)
  {
    if (a.display[2] != b.display[2])
    {
      return a.display[2] < b.display[2];
    }
    return a.distanceSquared < b.distanceSquared;
  });
  for (const auto& hit : candidates)
  {
    if (!occluded(view, hit))
    {
      result = hit;
      return true;
    }
  }
  return false;
}
