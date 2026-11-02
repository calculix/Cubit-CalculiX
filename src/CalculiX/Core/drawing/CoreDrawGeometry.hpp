#ifndef COREDRAWGEOMETRY_HPP
#define COREDRAWGEOMETRY_HPP

#include <string>
#include <vector>

enum class DrawPrimitiveKind { Point, Line, Polygon, Label };
struct DrawPrimitive
{
  DrawPrimitiveKind kind;
  std::vector<std::vector<double>> points;
  std::string color;
  std::string text;
};

struct DrawGeometry
{
  std::vector<DrawPrimitive> primitives;
  bool point(const std::vector<double>& position, const std::string& color);
  bool line(const std::vector<double>& start, const std::vector<double>& end, const std::string& color);
  bool polygon(const std::vector<std::vector<double>>& positions, const std::string& color);
  bool label(const std::vector<double>& position, const std::string& text, const std::string& color);
  void append(const DrawGeometry& geometry);
};
#endif
