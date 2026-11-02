#ifndef COREDRAWSYMBOLS_HPP
#define COREDRAWSYMBOLS_HPP

#include "CoreDrawGeometry.hpp"

class CoreDrawSymbols
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
  static bool arrow(DrawGeometry& geometry, std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size);
  static bool flatArrow(DrawGeometry& geometry, std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size);
  static bool displacement(DrawGeometry& geometry, std::vector<double> coord, int dof, std::string color, double size);
  static bool equationNode(DrawGeometry& geometry, std::vector<double> coord, int dof, std::string color, double size);
};

#endif
