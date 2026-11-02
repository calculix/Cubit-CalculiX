#include "CoreDrawGeometry.hpp"
#include <cmath>

namespace
{
bool valid(const std::vector<double>& point)
{
  return point.size() == 3 && std::isfinite(point[0]) && std::isfinite(point[1]) && std::isfinite(point[2]);
}
}

bool DrawGeometry::point(const std::vector<double>& position, const std::string& color)
{
  if (!valid(position))
  {
    return false;
  }
  primitives.push_back({DrawPrimitiveKind::Point, {position}, color, {}});
  return true;
}

bool DrawGeometry::line(const std::vector<double>& start, const std::vector<double>& end, const std::string& color)
{
  if (!valid(start) || !valid(end))
  {
    return false;
  }
  primitives.push_back({DrawPrimitiveKind::Line, {start, end}, color, {}});
  return true;
}

bool DrawGeometry::polygon(const std::vector<std::vector<double>>& positions, const std::string& color)
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
  primitives.push_back({DrawPrimitiveKind::Polygon, positions, color, {}});
  return true;
}

bool DrawGeometry::label(const std::vector<double>& position, const std::string& text, const std::string& color)
{
  if (!valid(position))
  {
    return false;
  }
  primitives.push_back({DrawPrimitiveKind::Label, {position}, color, text});
  return true;
}

void DrawGeometry::append(const DrawGeometry& geometry)
{
  primitives.insert(primitives.end(), geometry.primitives.begin(), geometry.primitives.end());
}
