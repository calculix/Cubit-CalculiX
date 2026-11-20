#include "CoreJointGeometry.hpp"
#include "CubitInterface.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
using Point = std::array<double, 3>;

double distance(const Point& a, const Point& b)
{
  return std::hypot(a[0]-b[0], a[1]-b[1], a[2]-b[2]);
}

bool ends(int surface, Point& first, Point& last)
{
  const auto types = CubitInterface::get_geometry_type("surface", {surface});
  if (types.empty() || (types.front() != "cylinder" && types.front() != "cone"))
  {
    return false;
  }
  std::vector<Point> centers;
  double radius = 0;
  for (int curve : CubitInterface::get_relatives("surface", surface, "curve"))
  {
    auto arc = CubitInterface::get_arc_center_radius(curve);
    if (arc[3] <= 0)
    {
      continue;
    }
    if (radius > 0 && std::abs(radius - arc[3]) > radius * 1e-7)
    {
      return false;
    }
    radius = arc[3];
    Point p{arc[0], arc[1], arc[2]};
    if (std::none_of(centers.begin(), centers.end(), [&](const Point& q) { return distance(q, p) < radius * 1e-8; }))
    {
      centers.push_back(p);
    }
  }
  if (centers.size() != 2)
  {
    return false;
  }
  std::sort(centers.begin(), centers.end());
  first = centers.front();
  last = centers.back();
  return true;
}
}

bool CoreJointGeometry::coaxial(int surface_a, int surface_b, Point& center, Point& direction, std::string& error)
{
  Point a, b, c, d;
  if (!ends(surface_a, a, b) || !ends(surface_b, c, d))
  {
    error = "Select two cylindrical faces with circular end edges or specify RP endpoints and an axis.";
    return false;
  }
  const double span = distance(a, b);
  for (int i = 0; i < 3; ++i)
  {
    direction[i] = (b[i] - a[i]) / span;
  }
  auto project = [&](const Point& p)
  {
    return (p[0]-a[0])*direction[0] + (p[1]-a[1])*direction[1] + (p[2]-a[2])*direction[2];
  };
  const double first = project(c);
  const double last = project(d);
  const double lo = std::max(0.0, std::min(first, last));
  const double hi = std::min(span, std::max(first, last));
  for (const Point& p : {c, d})
  {
    Point on_axis = a;
    for (int i = 0; i < 3; ++i)
    {
      on_axis[i] += project(p)*direction[i];
    }
    if (distance(p, on_axis) > 1e-7 * std::max(span, distance(c,d)) || lo >= hi)
    {
      error = "Joint faces must have coaxial, overlapping axial spans.";
      return false;
    }
  }
  for (int i = 0; i < 3; ++i)
  {
    center[i] = a[i] + 0.5 * (lo + hi) * direction[i];
  }
  return true;
}
