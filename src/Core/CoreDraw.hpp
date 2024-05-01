#ifndef COREDRAW_HPP
#define COREDRAW_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

// class for drawing of loads and bcs

class CoreDraw
{

public:
  CoreDraw();
  ~CoreDraw();

  bool is_initialized = false;

  bool init(); // initialize
  std::vector<double> rotate(std::vector<double> coord, std::vector<double> vec_a, std::vector<double> vec_b); // rotates coordinate from vector a to b
  std::vector<double> unit_vector(std::vector<double> input);
  double magnitude(std::vector<double> input); //magnitude for a vector with starting point {0,0,0}
  std::vector<double> cross_product(std::vector<double> vec_a, std::vector<double> vec_b);
  std::vector<std::vector<double>> cross_product_matrix(std::vector<double> vec);
  double inner_product(std::vector<double> vec_a, std::vector<double> vec_b);
  std::vector<std::vector<double>> mult_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b); //multiplicate 3x3 matrix with 3x3
  std::vector<std::vector<double>> mult_matrix_scalar(std::vector<std::vector<double>> mat_a, double scalar); //multiplicate 3x3 matrix with scalar
  std::vector<std::vector<double>> add_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b); //addition of 3x3 matrix
  std::vector<double> mult_matrix_vector(std::vector<std::vector<double>> mat, std::vector<double> vec); //multiplicate 3x3 matrix with 3x1 vector
  
  bool draw_arrow(std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size); //draw an arrow from a center point, vector, color, size
  bool draw_all(); // draw all loads and bcs

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREDRAW_HPP