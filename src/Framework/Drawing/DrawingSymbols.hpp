#ifndef CUBITSIM_DRAWINGSYMBOLS_HPP
#define CUBITSIM_DRAWINGSYMBOLS_HPP

#include "DrawingExport.hpp"

#include "DrawingGeometry.hpp"

// Components describe symbols, independently of a solver's DOF numbering.
enum class DrawingSymbolComponent
{
  TranslationX, TranslationY, TranslationZ,
  RotationX, RotationY, RotationZ, Temperature
};

// Vector and matrix helpers require three-component vectors and 3x3 matrices.
// Symbol positions/directions use world coordinates; symbol size must be finite
// and positive. rotate_about_axis takes an angle in degrees. Solver-specific
// component numbering and unit conversion belong to the workspace adapter.
class CUBITSIM_DRAWING_EXPORT DrawingSymbols
{
public:
  static std::vector<double> rotate(std::vector<double> coord, std::vector<double> vec_a, std::vector<double> vec_b);
  static std::vector<double> rotate_about_axis(std::vector<double> coord, std::vector<double> axis, double angle);
  static std::vector<double> unit_vector(std::vector<double> input);
  static double magnitude(std::vector<double> input);
  static std::vector<double> cross_product(std::vector<double> vec_a, std::vector<double> vec_b);
  static std::vector<std::vector<double>> cross_product_matrix(std::vector<double> vec);
  static double inner_product(std::vector<double> vec_a, std::vector<double> vec_b);
  static std::vector<double> mult_vector_scalar(std::vector<double> vec, double scalar);
  static std::vector<std::vector<double>> mult_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b);
  static std::vector<std::vector<double>> mult_matrix_scalar(std::vector<std::vector<double>> mat_a, double scalar);
  static std::vector<std::vector<double>> add_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b);
  static std::vector<double> mult_matrix_vector(std::vector<std::vector<double>> mat, std::vector<double> vec);
  static bool arrow(DrawingGeometry& geometry, std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size);
  static bool flatArrow(DrawingGeometry& geometry, std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size);
  static bool displacement(DrawingGeometry& geometry, std::vector<double> coord, DrawingSymbolComponent component, std::string color, double size);
  static bool equationNode(DrawingGeometry& geometry, std::vector<double> coord, DrawingSymbolComponent component, std::string color, double size);
};

#endif
