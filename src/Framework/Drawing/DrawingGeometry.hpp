#ifndef CUBITSIM_DRAWINGGEOMETRY_HPP
#define CUBITSIM_DRAWINGGEOMETRY_HPP

#include "DrawingExport.hpp"

#include <string>
#include <vector>

enum class DrawingPrimitiveKind { Point, Line, Polygon, Label };
struct DrawingPrimitive
{
  DrawingPrimitiveKind kind;
  std::vector<std::vector<double>> points;
  std::string color;
  std::string text;
};

// Positions are finite 3D world coordinates. Polygons need at least three
// ordered planar points; builders do not prove planarity or non-self-intersection.
struct CUBITSIM_DRAWING_EXPORT DrawingGeometry
{
  std::vector<DrawingPrimitive> primitives;
  bool point(const std::vector<double>& position, const std::string& color);
  bool line(const std::vector<double>& start, const std::vector<double>& end, const std::string& color);
  bool polygon(const std::vector<std::vector<double>>& positions, const std::string& color);
  bool label(const std::vector<double>& position, const std::string& text, const std::string& color);
  void append(const DrawingGeometry& geometry);
};
#endif
