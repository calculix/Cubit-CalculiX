#include "DrawingGeometry.hpp"
#include <cmath>

namespace
{
bool valid(const std::vector<double>& point)
{
  return point.size() == 3 && std::isfinite(point[0]) && std::isfinite(point[1]) && std::isfinite(point[2]);
}
}

bool DrawingGeometry::point(const std::vector<double>& position, const std::string& color)
{
  if (!valid(position))
  {
    return false;
  }
  primitives.push_back({DrawingPrimitiveKind::Point, {position}, color, {}});
  return true;
}

bool DrawingGeometry::line(const std::vector<double>& start, const std::vector<double>& end, const std::string& color)
{
  if (!valid(start) || !valid(end))
  {
    return false;
  }
  primitives.push_back({DrawingPrimitiveKind::Line, {start, end}, color, {}});
  return true;
}

bool DrawingGeometry::polygon(const std::vector<std::vector<double>>& positions, const std::string& color)
{
  if (positions.size() < 3)
  {
    return false;
  }
  for (const auto& position : positions)
  {
    if (!valid(position))
    {
      return false;
    }
  }
  primitives.push_back({DrawingPrimitiveKind::Polygon, positions, color, {}});
  return true;
}

bool DrawingGeometry::label(const std::vector<double>& position, const std::string& text, const std::string& color)
{
  if (!valid(position))
  {
    return false;
  }
  primitives.push_back({DrawingPrimitiveKind::Label, {position}, color, text});
  return true;
}

void DrawingGeometry::append(const DrawingGeometry& geometry)
{
  primitives.insert(primitives.end(), geometry.primitives.begin(), geometry.primitives.end());
}
